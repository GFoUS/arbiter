#pragma once

#include "element.h"
#include "dockspace.h"
#include "core/arbiter/event.h"
#include "cglm/cglm.h"
#include "project/ecs/entity.h"

typedef struct {
    ui_element element;
    arbiter_event_listener* listener;

    ecs_entity* selectedEntity;

    bool inNewComponentWizard;
    bool inAssetSelection;

    vec3 translation;
    vec3 rotation;
    vec3 scale;
} ui_properties;

ui_properties* ui_properties_create(ui_dockspace* parent);