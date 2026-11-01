// boards/shields/nice_view_gem/widgets/layer.c
/*
 * layer.c — 右栏：图层图标 + 动态激活点阵
 *
 * 屏幕布局（160×68 横屏，右栏中心 X=132）：
 *   图层图标 30×30   Y 14..43（X 117..146）
 *   点阵 5×5         Y 58..62（底部对齐中栏无 USB 时的底部）
 *
 * 动态点阵逻辑：
 *   - 只提取当前激活的层。
 *   - 如果超过 5 个激活层，只显示优先级最高的 5 个。
 *   - 最高优先层显示为实心，其余激活层显示为空心。
 *   - 整体水平居中于右栏中心。
 */

#include <zephyr/kernel.h>
#include "layer.h"
#include "../assets/custom_fonts.h"

LV_IMG_DECLARE(layer);

#define CENTER_X       132
#define LAYER_ICON_X   117
#define LAYER_ICON_Y   14

#define DOTS_Y         58
#define DOT_SIZE       5
#define DOT_GAP        4
#define MAX_DOTS       5

static void draw_layer_icon(lv_obj_t *canvas) {
    lv_draw_img_dsc_t dsc;
    lv_draw_img_dsc_init(&dsc);
    lv_canvas_draw_img(canvas, LAYER_ICON_X, LAYER_ICON_Y, &layer, &dsc);
}

static void draw_layer_dots(lv_obj_t *canvas, const struct status_state *state) {
    uint32_t mask = state->layer_mask;
    uint8_t highest = state->layer_index;
    uint8_t active_layers[32];
    int count = 0;

    /* 收集所有激活层的索引 */
    for (int i = 0; i < 32; i++) {
        if (mask & (1U << i)) {
            active_layers[count++] = i;
        }
    }

    if (count == 0) {
        return;
    }

    /* 如果超过 5 个激活层，只保留优先级最高的 5 个（即数组末尾的 5 个） */
    int start_idx = 0;
    if (count > MAX_DOTS) {
        start_idx = count - MAX_DOTS;
        count = MAX_DOTS;
    }

    /* 计算整体居中位置 */
    int total_w = count * DOT_SIZE + (count - 1) * DOT_GAP;
    int start_x = CENTER_X - total_w / 2;

    for (int j = 0; j < count; j++) {
        int layer_idx = active_layers[start_idx + j];
        int x = start_x + j * (DOT_SIZE + DOT_GAP);

        /* 画实心外框 */
        lv_draw_rect_dsc_t fg_dsc;
        init_rect_dsc(&fg_dsc, LVGL_FOREGROUND);
        lv_canvas_draw_rect(canvas, x, DOTS_Y, DOT_SIZE, DOT_SIZE, &fg_dsc);

        /* 如果不是最高优先级层，挖去中间，变成空心 */
        if (layer_idx != highest) {
            lv_draw_rect_dsc_t bg_dsc;
            init_rect_dsc(&bg_dsc, LVGL_BACKGROUND);
            lv_canvas_draw_rect(canvas, x + 1, DOTS_Y + 1, DOT_SIZE - 2, DOT_SIZE - 2, &bg_dsc);
        }
    }
}

void draw_layer_status(lv_obj_t *canvas, const struct status_state *state) {
    draw_layer_icon(canvas);
    draw_layer_dots(canvas, state);
}
