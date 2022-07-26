#pragma once

#include "core/core.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"
#include "cglm/cglm.h"

// All these predefined colors are taken from TailwindCSS, specifically here: https://tailwindcss.com/docs/customizing-colors

typedef struct {
    vec4 color;
    bool isNormalized;
} color_color;

#define SLATE500 (color_color){ 100, 116, 139, 255 }
#define SLATE600 (color_color){ 71,  85,  105, 255 }
#define SLATE700 (color_color){ 51,  65,  85,  255 }
#define SLATE800 (color_color){ 30,  41,  59,  255 }
#define SLATE900 (color_color){ 15,  23,  42,  255 }

#define STONE700 (color_color){ 68,  64,  60,  255 }
#define STONE800 (color_color){ 41,  37,  36,  255 }
#define STONE900 (color_color){ 28,  25,  23,  255 }

#define RED300   (color_color){ 252, 165, 165, 255 }
#define RED500   (color_color){ 239, 68,  68,  255 }
#define RED600   (color_color){ 220, 38,  38,  255 }

#define GREEN300 (color_color){ 110, 231, 183, 255 }
#define GREEN500 (color_color){ 34,  197, 94,  255 }
#define GREEN600 (color_color){ 22,  163, 74,  255 }

#define TEAL500  (color_color){ 20,  184, 166, 255 }

#define BLUE400  (color_color){ 96,  165, 250, 255 }
#define BLUE500  (color_color){ 59,  130, 246, 255 }
#define BLUE600  (color_color){ 37,  99,  235, 255 }
#define BLUE700  (color_color){ 29,  78,  216, 255 }


color_color normalize_color(color_color color);
ImVec4 color_to_imvec4(color_color color);