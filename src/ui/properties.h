#pragma once

#include "element.h"
#include "dockspace.h"
#include "core/arbiter_event.h"

typedef struct {
    ui_element element;
    arbiter_event_listener* listener;

    bool inNewComponentWizard;
    bool inAssetSelection;
} ui_properties;

ui_properties* ui_properties_create(ui_dockspace* parent);