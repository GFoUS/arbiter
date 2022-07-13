#pragma once

#include "dockspace.h"
#include "viewport.h"
#include "menubar/menubar.h"
#include "world_hierarchy.h"
#include "properties.h"

#include "vulkan/context.h"

typedef struct {
    ui_dockspace* dockspace;
    ui_viewport* viewport;
    ui_menubar* menubar;
    ui_world_hierarchy* hierarchy;
    ui_properties* properties;

    vulkan_context* ctx;
} ui_editor;

ui_editor* ui_editor_create(vulkan_context* ctx);
void ui_editor_destroy(ui_editor* editor);

void ui_editor_render(ui_editor* editor);