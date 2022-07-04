#pragma once

#include "dockspace.h"
#include "viewport.h"

#include "vulkan/context.h"

typedef struct {
    ui_dockspace* dockspace;
    ui_viewport* viewport;

    vulkan_context* ctx;
} ui_editor;

ui_editor* ui_editor_create(vulkan_context* ctx);
void ui_editor_destroy(ui_editor* editor);

void ui_editor_render(ui_editor* editor);