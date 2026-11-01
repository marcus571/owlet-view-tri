// boards/shields/nice_view_gem/widgets/layer.h
#pragma once

#include <lvgl.h>
#include "util.h"

struct layer_status_state {
    uint8_t index;
    uint32_t layer_mask;
};

void draw_layer_status(lv_obj_t *canvas, const struct status_state *state);
