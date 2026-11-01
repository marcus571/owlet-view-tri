/*
 * layer.c
 * 2026-10-31 修改记录：
 *   将 "LAY" 标签和图层数字拆开绘制。
 *   标签在 X=0，数字在 X=26，实现与上方两行像素级完美左对齐。
 */
#include <zephyr/kernel.h>
#include <string.h>
#include "layer.h"
#include "../assets/custom_fonts.h"

void draw_layer_status(lv_obj_t *canvas, const struct status_state *state) {
    lv_draw_label_dsc_t label_dsc;
    init_label_dsc(&label_dsc, LVGL_FOREGROUND, &pixel_operator_mono, LV_TEXT_ALIGN_LEFT);

    char text[10] = {};

    if (state->layer_label == NULL || strlen(state->layer_label) == 0) {
        // 如果没有自定义层名，分两段绘制以实现对齐
        lv_canvas_draw_text(canvas, 0, 51, 26, &label_dsc, "LAY");
        sprintf(text, "%i", state->layer_index);
        lv_canvas_draw_text(canvas, 26, 51, 42, &label_dsc, text);
    } else {
        // 如果有自定义层名，直接绘制名称
        strncpy(text, state->layer_label, 9);
        to_uppercase(text);
        lv_canvas_draw_text(canvas, 0, 51, 68, &label_dsc, text);
    }
}
