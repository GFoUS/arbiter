// This code doesn't work and is unused, I may revisit it in future

#pragma once

#include "camera.h"

#include "project/ecs/ecs.h"
#include "core/arbiter/event.h"

#include "cglm/cglm.h"

typedef struct ui_viewport_t ui_viewport;

typedef struct {
    ui_element element;

    ui_camera* camera;
    ui_viewport* viewport;

    ecs_entity* selectedEntity;
} ui_guizmo;

ui_guizmo* ui_guizmo_create(ui_viewport* viewport, ui_camera* camera);