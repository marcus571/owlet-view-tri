// boards/shields/nice_view_gem/widgets/screen.c
/*
 * screen.c — 单 canvas 160×68 三栏布局调度
 *
 * 2026-11-08 新架构：
 *   - 单 canvas 160×68 替代旧双 canvas
 *   - 左栏：电池（battery.c，0..51）
 *   - 中栏：连接状态（output.c，56..103）
 *   - 右栏：待实现
 *   - 分隔线 X=53、X=106，Y 4..64
 */

#include <zephyr/kernel.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/event_manager.h>
#include <zmk/events/battery_state_changed.h>
#include <zmk/events/ble_active_profile_changed.h>
#include <zmk/events/endpoint_changed.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/events/usb_conn_state_changed.h>
#include <zmk/battery.h>
#include <zmk/ble.h>
#include <zmk/display.h>
#include <zmk/endpoints.h>
#include <zmk/keymap.h>
#include <zmk/usb.h>

#include "battery.h"
#include "layer.h"
#include "output.h"
#include "profile.h"
#include "screen.h"

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

/* 分隔线 */
#define SEPARATOR_Y        4
#define SEPARATOR_H        61
#define SEPARATOR_LEFT_X   53
#define SEPARATOR_RIGHT_X  106

static void draw_separator(lv_obj_t *canvas, int x) {
    lv_draw_rect_dsc_t dsc;
    init_rect_dsc(&dsc, LVGL_FOREGROUND);
    lv_canvas_draw_rect(canvas, x, SEPARATOR_Y, 1, SEPARATOR_H, &dsc);
}

static void draw_all(lv_obj_t *widget, const struct status_state *state) {
    lv_obj_t *canvas = lv_obj_get_child(widget, 0);
    fill_background(canvas);

    draw_battery_status(canvas, state);   /* 左栏 */
    draw_output_status(canvas, state);    /* 中栏 */
    /* TODO: 右栏 draw_layer_status 重做后启用 */

    draw_separator(canvas, SEPARATOR_LEFT_X);
    draw_separator(canvas, SEPARATOR_RIGHT_X);
}

static void set_battery_status(struct zmk_widget_screen *widget,
                               struct battery_status_state state) {
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
    widget->state.charging = state.usb_present;
#endif
    widget->state.battery = state.level;
    draw_all(widget->obj, &widget->state);
}

static void battery_status_update_cb(struct battery_status_state state) {
    struct zmk_widget_screen *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_battery_status(widget, state); }
}

static struct battery_status_state battery_status_get_state(const zmk_event_t *eh) {
    const struct zmk_battery_state_changed *ev = as_zmk_battery_state_changed(eh);
    (void)ev; /* TODO(max17048): 线路修复后恢复真实读取 */

    return (struct battery_status_state){
        .level = 100, /* TODO(max17048): mock 100% */
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
        .usb_present = zmk_usb_is_powered(),
#endif
    };
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_battery_status, struct battery_status_state,
                            battery_status_update_cb, battery_status_get_state);
ZMK_SUBSCRIPTION(widget_battery_status, zmk_battery_state_changed);
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
ZMK_SUBSCRIPTION(widget_battery_status, zmk_usb_conn_state_changed);
#endif

static void set_layer_status(struct zmk_widget_screen *widget, struct layer_status_state state) {
    widget->state.layer_index = state.index;
    widget->state.layer_label = state.label;
    draw_all(widget->obj, &widget->state);
}

static void layer_status_update_cb(struct layer_status_state state) {
    struct zmk_widget_screen *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_layer_status(widget, state); }
}

static struct layer_status_state layer_status_get_state(const zmk_event_t *eh) {
    uint8_t index = zmk_keymap_highest_layer_active();
    return (struct layer_status_state){.index = index, .label = zmk_keymap_layer_name(index)};
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_layer_status, struct layer_status_state, layer_status_update_cb,
                            layer_status_get_state)
ZMK_SUBSCRIPTION(widget_layer_status, zmk_layer_state_changed);

static void set_output_status(struct zmk_widget_screen *widget,
                              const struct output_status_state *state) {
    widget->state.selected_endpoint = state->selected_endpoint;
    widget->state.active_profile_index = state->active_profile_index;
    widget->state.active_profile_connected = state->active_profile_connected;
    widget->state.active_profile_bonded = state->active_profile_bonded;
    draw_all(widget->obj, &widget->state);
}

static void output_status_update_cb(struct output_status_state state) {
    struct zmk_widget_screen *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_output_status(widget, &state); }
}

static struct output_status_state output_status_get_state(const zmk_event_t *_eh) {
    return (struct output_status_state){
        .selected_endpoint = zmk_endpoints_selected(),
        .active_profile_index = zmk_ble_active_profile_index(),
        .active_profile_connected = zmk_ble_active_profile_is_connected(),
        .active_profile_bonded = !zmk_ble_active_profile_is_open(),
    };
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_output_status, struct output_status_state,
                            output_status_update_cb, output_status_get_state)
ZMK_SUBSCRIPTION(widget_output_status, zmk_endpoint_changed);
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
ZMK_SUBSCRIPTION(widget_output_status, zmk_usb_conn_state_changed);
#endif
#if defined(CONFIG_ZMK_BLE)
ZMK_SUBSCRIPTION(widget_output_status, zmk_ble_active_profile_changed);
#endif

int zmk_widget_screen_init(struct zmk_widget_screen *widget, lv_obj_t *parent) {
    widget->obj = lv_obj_create(parent);
    lv_obj_set_size(widget->obj, SCREEN_HEIGHT, SCREEN_WIDTH);

    lv_obj_t *canvas = lv_canvas_create(widget->obj);
    lv_obj_align(canvas, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_canvas_set_buffer(canvas, widget->cbuf, SCREEN_CANVAS_W, SCREEN_CANVAS_H,
                         LV_IMG_CF_TRUE_COLOR);

    sys_slist_append(&widgets, &widget->node);
    widget_battery_status_init();
    widget_layer_status_init();
    widget_output_status_init();

    struct status_state initial_state = {0};
    initial_state.battery = 100; /* TODO(max17048): mock */
    draw_all(widget->obj, &initial_state);

    return 0;
}

lv_obj_t *zmk_widget_screen_obj(struct zmk_widget_screen *widget) { return widget->obj; }
