#include "colors.h"

color_color normalize_color(color_color color) {
    if (color.isNormalized) return color;

    color_color out;
    CLEAR_MEMORY(&out);
    for (u32 i = 0; i < 4; i++) {
        out.color[i] = color.color[i] / 255.0f;
    }
    out.isNormalized = true;
    return out;
}

ImVec4 color_to_imvec4(color_color color) {
    return *(ImVec4*)normalize_color(color).color;
}