#pragma once

#include "element.h"
#include "dockspace.h"

typedef struct {
    ui_element element;
} ui_world_hierarchy;

ui_world_hierarchy* ui_world_hierarchy_create(ui_dockspace* parent);