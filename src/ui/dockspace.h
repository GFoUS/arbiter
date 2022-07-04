#pragma once

#include "element.h"

typedef struct {
    ui_element element;
} ui_dockspace;

ui_dockspace* ui_dockspace_create();