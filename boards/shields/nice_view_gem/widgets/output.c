// boards/shields/nice_view_gem/widgets/output.c
/*
 * output.c — 中栏：大号蓝牙状态图标 + 5 个设备点 + USB 图标
 *
 * 屏幕布局（160×68 横屏，中栏 X=56..103，中心 X=80）：
 *   无 USB 模式（两行）：
 *     大蓝牙 24×24   Y 30..53
 *     5 点 4×4       Y 59..62（底 62）
 *   有 USB 模式（三行，等间距 4px）：
 *     大蓝牙 24×24   Y 16..39
 *     5 点 4×4       Y 44..47
 *     USB 20×11      Y 52..62（底 62）
 *
 * 5 个设备点：4px 直径，间距 4px，总宽 36，X 62..97，居中 80
 *
 * 2026-11-08：
 *   - 有 USB 时也使用大号蓝牙，三种小图标（bt/bt_no_signal/bt_unbonded）不再引用
 *   - 所有中栏内容相对上一版上移 2px
 *   - 有 USB 模式三行等间距 4px
 */

#include <zephyr/kernel.h>
#include "output.h"
#include "../assets/custom_fonts.h"

LV_IMG_DECLARE(bt_large_connected);
LV_IMG_DECLARE(bt_large_disconnected);
LV_IMG_DECLARE(bt_large_searching);
LV_IMG_DECLARE(usb);

#define CENTER_X       80

/* 5 个设备点 */
#define DOT_COUNT      5
#define DOT_SIZE       4
#define DOT_GAP        4
#define DOTS_TOTAL_W   (DOT_COUNT * DOT_SIZE + (DOT_COUNT - 1) * DOT_GAP)
#define DOTS_X         (CENTER_X - DOTS_TOTAL_W / 2)

/* 无 USB 模式 */
#define BT_LARGE_Y_NO_USB   30
#define DOTS_Y_NO_USB       59

/* 有 USB 模式（三行等间距 4px） */
#define BT_LARGE_Y_USB      16
#define DOTS_Y_USB          44
#define USB_Y               52

static void draw_centered_img(lv_obj_t *canvas, const lv_img_dsc_t *img, int y) {
    int x = CENTER_X - img->header.w / 2;
    lv_draw_img_dsc_t dsc;
    lv_draw_img_dsc_init(&dsc);
    lv_canvas_draw_img(canvas, x, y, img, &dsc);
}

static void draw_bt_large(lv_obj_t *canvas, const struct status_state *state, int y) {
    const lv_img_dsc_t *img;
    if (state->active_profile_connected) {
        img = &bt_large_connected;
    } else if (state->active_profile_bonded) {
        img = &bt_large_disconnected;
    } else {
        img = &bt_large_searching;
    }
    draw_centered_img(canvas, img, y);
}

static void draw_device_dots(lv_obj_t *canvas, const struct status_state *state, int y) {
    for (int i = 0; i < DOT_COUNT; i++) {
        int x = DOTS_X + i * (DOT_SIZE + DOT_GAP);

        bool active = (i == state->active_profile_index) && state->active_profile_connected;

        lv_draw_rect_dsc_t fg_dsc;
        init_rect_dsc(&fg_dsc, LVGL_FOREGROUND);
        lv_canvas_draw_rect(canvas, x, y, DOT_SIZE, DOT_SIZE, &fg_dsc);

        if (!active) {
            /* 空心：中心 (DOT_SIZE-2)² 用背景色覆盖 */
            lv_draw_rect_dsc_t bg_dsc;
            init_rect_dsc(&bg_dsc, LVGL_BACKGROUND);
            lv_canvas_draw_rect(canvas, x + 1, y + 1, DOT_SIZE - 2, DOT_SIZE - 2, &bg_dsc);
        }
    }
}

static void draw_usb_icon(lv_obj_t *canvas) {
    draw_centered_img(canvas, &usb, USB_Y);
}

void draw_output_status(lv_obj_t *canvas, const struct status_state *state) {
    bool usb_mode = (state->selected_endpoint.transport == ZMK_TRANSPORT_USB);

    if (usb_mode) {
        draw_bt_large(canvas, state, BT_LARGE_Y_USB);
        draw_device_dots(canvas, state, DOTS_Y_USB);
        draw_usb_icon(canvas);
    } else {
        draw_bt_large(canvas, state, BT_LARGE_Y_NO_USB);
        draw_device_dots(canvas, state, DOTS_Y_NO_USB);
    }
}
