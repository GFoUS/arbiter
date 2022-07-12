#pragma once

#include "core/core.h"
#include "vulkan/context.h"

typedef enum {
    FONT_WEIGHT_LIGHT = 0,
    FONT_WEIGHT_REGULAR,
    FONT_WEIGHT_MEDIUM,
    FONT_WEIGHT_SEMIBOLD,
    FONT_WEIGHT_BOLD
} ui_font_weight;

void ui_font_load(vulkan_context* ctx, const char* basePath);