// boards/shields/nice_view_gem/widgets/output.c
/*
 * output.c — 中栏：大号蓝牙状态图标 + 5 个设备点 + USB 图标
 *
 * 2026-11-08 修改：
 *   - 移除留白，中栏中心 X=79。
 */

#include <zephyr/kernel.h>
#include "output.h"
#include "../assets/custom_fonts.h"

LV_IMG_DECLARE(bt_large_connected);
LV_IMG_DECLARE(bt_large_disconnected);
LV_IMG_DECLARE(bt_large_searching);
LV_IMG_DECLARE(usb);

#define CENTER_X       79

/* 5 个设备点（5×5，间距 4） */
#define DOT_COUNT      5
#define DOT_SIZE       5
#define DOT_GAP        4
#define DOTS_TOTAL_W   (DOT_COUNT * DOT_SIZE + (DOT_COUNT - 1) * DOT_GAP)
#define DOTS_X         (CENTER_X - DOTS_TOTAL_W / 2)

/* 有 USB 模式 Y */
#define BT_LARGE_Y_USB      7
#define DOTS_Y_USB          43
#define USB_Y               52

/* 无 USB 模式 Y */
#define BT_LARGE_Y_NO_USB   15
#define DOTS_Y_NO_USB       58

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
