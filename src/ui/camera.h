#pragma once

#include "core/core.h"
#include "cglm/cglm.h"
#include "element.h"

typedef struct ui_viewport_t ui_viewport;

typedef struct {
    ui_element element;
    ui_viewport* viewport;

    vec3 centre;
    float fovy;
    float near;
    float far;

    float pitch;
    float yaw;
    float distance;

    mat4 view;
    mat4 proj;
} ui_camera;

ui_camera* ui_camera_create(ui_viewport* viewport, float fovy, float near, float far);
void ui_camera_update(ui_camera* camera, float fovy, float near, float far);
void ui_camera_get_eye(ui_camera* camera, vec3 eye);
