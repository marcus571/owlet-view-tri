// boards/shields/nice_view_gem/widgets/battery.c
/*
 * battery.c — 左栏：大电池轮廓 + 实心填充 + 左侧充电闪电 + 百分比文字
 *
 * 2026-11-08 修改：
 *   - 电池图宽度 24→20，需要重新居中
 *   - 电池位置 X 16→18，实心填充 X 19→21、宽 18→14
 *   - 闪电位置利用 BOLT_AREA_RIGHT 自动调整居中
 */

#include <zephyr/kernel.h>
#include <stdio.h>
#include <string.h>

#include "battery.h"
#include "../assets/custom_fonts.h"

LV_IMG_DECLARE(battery);
LV_IMG_DECLARE(bolt_large);

/* 电池图 */
#define BATTERY_X     18
#define BATTERY_Y     4
#define BATTERY_H     45

/* 实心填充区（电池内轮廓再内缩 1px） */
#define FILL_X        21
#define FILL_W        14
#define FILL_TOP      10
#define FILL_BOTTOM   45
#define FILL_H_MAX    36

/* 闪电所在区域：左栏起点 X=4 到电池左边界 X=18 */
#define BOLT_AREA_LEFT   4
#define BOLT_AREA_RIGHT  BATTERY_X

/* 百分比文字 */
#define PCT_X         4
#define PCT_Y         52
#define PCT_W         48

static void draw_battery_outline(lv_obj_t *canvas) {
    lv_draw_img_dsc_t img_dsc;
    lv_draw_img_dsc_init(&img_dsc);
    lv_canvas_draw_img(canvas, BATTERY_X, BATTERY_Y, &battery, &img_dsc);
}

static void draw_battery_fill(lv_obj_t *canvas, uint8_t level) {
    if (level == 0) {
        return;
    }

    int fill_h = (level * FILL_H_MAX) / 100;
    if (fill_h > FILL_H_MAX) {
        fill_h = FILL_H_MAX;
    }
    if (fill_h == 0) {
        fill_h = 1;
    }

    int fill_y = FILL_BOTTOM - fill_h + 1;

    lv_draw_rect_dsc_t rect_dsc;
    init_rect_dsc(&rect_dsc, LVGL_FOREGROUND);
    lv_canvas_draw_rect(canvas, FILL_X, fill_y, FILL_W, fill_h, &rect_dsc);
}

static void draw_battery_charging(lv_obj_t *canvas) {
    int bolt_w = bolt_large.header.w;
    int bolt_h = bolt_large.header.h;

    int area_w = BOLT_AREA_RIGHT - BOLT_AREA_LEFT;
    int bolt_x = BOLT_AREA_LEFT + (area_w - bolt_w) / 2;
    int bolt_y = BATTERY_Y + (BATTERY_H - bolt_h) / 2;

    lv_draw_img_dsc_t img_dsc;
    lv_draw_img_dsc_init(&img_dsc);
    lv_canvas_draw_img(canvas, bolt_x, bolt_y, &bolt_large, &img_dsc);
}

static void draw_battery_level(lv_obj_t *canvas, uint8_t level) {
    lv_draw_label_dsc_t label_dsc;
    init_label_dsc(&label_dsc, LVGL_FOREGROUND, &pixel_operator_mono, LV_TEXT_ALIGN_CENTER);

    char text[8];
    snprintf(text, sizeof(text), "%d%%", level);
    lv_canvas_draw_text(canvas, PCT_X, PCT_Y, PCT_W, &label_dsc, text);
}

void draw_battery_status(lv_obj_t *canvas, const struct status_state *state) {
    draw_battery_outline(canvas);
    draw_battery_fill(canvas, state->battery);
    draw_battery_level(canvas, state->battery);

    if (state->charging) {
        draw_battery_charging(canvas);
    }
}
