// boards/shields/nice_view_gem/widgets/layer.h
#pragma once

#include <lvgl.h>
#include "util.h"

struct layer_status_state {
    uint8_t index;
    const char *label;
    uint32_t layer_mask; /* 新增：用于指示所有激活的层 */
};

void draw_layer_status(lv_obj_t *canvas, const struct status_state *state);
