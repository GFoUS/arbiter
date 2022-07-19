#include "camera.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"

#include "project/ecs/ecs.h"

#include "viewport.h"

#define PAN_SENSITIVITY 1000.0f
#define ORBIT_SENSITIVITY 400.0f
#define ZOOM_SENSITIVITY 10.0f

// Note: Pitch is up/down, yaw is left/right
void ui_camera_get_eye(ui_camera* camera, vec3 eye) {
    glm_vec3_zero(eye);
    camera->pitch = clamp_float(camera->pitch, 0.0001f, (float)(PI - 0.0001f));
    vec3 angles = {0.0f, camera->yaw, camera->pitch};
    mat4 rotation;
    glm_euler(angles, rotation);

    vec3 up = {0.0f, 1.0f, 0.0f};
    glm_vec3_rotate_m4(rotation, up, eye);

    vec3 distance = {camera->distance, camera->distance, camera->distance};
    glm_vec3_mul(eye, distance, eye);
    glm_vec3_add(eye, camera->centre, eye);
}

void ui_camera_make_matrices(ui_camera* camera) {
    vec3 up = {0.0f, -1.0f, 0.0f};
    vec3 eye; ui_camera_get_eye(camera, eye);
    glm_lookat(eye, camera->centre, up, camera->view);
    glm_perspective(camera->fovy, (float)camera->viewport->sceneImage->width / (float)camera->viewport->sceneImage->height, camera->near, camera->far, camera->proj);
}

void ui_camera_render(ui_element* cameraElement, void(*body)(ui_element*)) {
    ui_camera* camera = (ui_camera*)cameraElement;

    if (igIsItemHovered(0)) {
        // Pan
        {
            static vec2 prevMouseDelta = {0.0f, 0.0f};
            ImVec2 imDelta;
            igGetMouseDragDelta(&imDelta, ImGuiMouseButton_Left, 0.0f);

            if (imDelta.x != 0 || imDelta.y != 0) {
                vec2 mouseDelta;
                glm_vec2_sub((float*)&imDelta, prevMouseDelta, mouseDelta);
                
                vec3 pan = {mouseDelta[1] / PAN_SENSITIVITY, 0.0f, mouseDelta[0] / PAN_SENSITIVITY};

                vec3 angles = {0.0f, camera->yaw, camera->pitch};
                mat4 rotation;
                glm_euler(angles, rotation);
                glm_vec3_rotate_m4(rotation, pan, pan);

                vec3 distance = {camera->distance, camera->distance, camera->distance};
                glm_vec3_mul(pan, distance, pan);
                
                glm_vec3_add(camera->centre, pan, camera->centre);

                ui_camera_make_matrices(camera);
            }

            glm_vec2_copy((float*)&imDelta, prevMouseDelta);
        }

        // Orbit
        {
            static vec2 prevMouseDelta = {0.0f, 0.0f};
            ImVec2 imDelta;
            igGetMouseDragDelta(&imDelta, ImGuiMouseButton_Middle, 0.0f);

            if (imDelta.x != 0 || imDelta.y != 0) {
                vec2 mouseDelta;
                glm_vec2_sub((float*)&imDelta, prevMouseDelta, mouseDelta);

                camera->yaw   += mouseDelta[0] / ORBIT_SENSITIVITY;
                camera->pitch -= mouseDelta[1] / ORBIT_SENSITIVITY;
                ui_camera_make_matrices(camera);
            }

            glm_vec2_copy((float*)&imDelta, prevMouseDelta);
        }

        // Zoom
        {
            ImGuiIO* io = igGetIO();
            if (io->MouseWheel) {
                camera->distance *= 1 + (io->MouseWheel / ZOOM_SENSITIVITY);
                ui_camera_make_matrices(camera);
            }
        }
    }

    body(&camera->element);
}

void ui_camera_on_entity_selected(void* entityPtr, void* cameraPtr) {
    ecs_entity* entity = (ecs_entity*)entityPtr;
    ui_camera* camera = (ui_camera*)cameraPtr;

    ecs_component_model* model = (ecs_component_model*)ecs_entity_get_component(entity, COMPONENT_TYPE_MODEL);
    ecs_component_transform* transform = (ecs_component_transform*)ecs_entity_get_component(entity, COMPONENT_TYPE_TRANSFORM);
    if (model != NULL && transform != NULL) {
        vec3 centre;
        glm_vec3_zero(centre);
        glm_mat4_mulv3(transform->matrix, centre, 1.0f, centre);

        // Also need to do model's transform
        if (model->asset->numTimesLoaded != 0) {
            model_model* m = ((project_asset_model*)model->asset->loadedData)->model;
            mat4 modelTransform;
            model_get_node_matrix(m, modelTransform);
            glm_mat4_mulv3(modelTransform, centre, 1.0f, centre);
        }

        glm_vec3_copy(centre, camera->centre);
        camera->pitch = 0.0001f;
        camera->yaw = 0.0001f;
        ui_camera_make_matrices(camera);
    }
}

ui_camera* ui_camera_create(ui_viewport* viewport, float fovy, float near, float far) {
    ui_camera* camera = malloc(sizeof(ui_camera));
    CLEAR_MEMORY(camera);
    glm_mat4_identity(camera->view);
    camera->viewport = viewport;
    camera->distance = 10.0f;
    camera->pitch = 0.0001f; // if its exactly zero, nothing is visable
    camera->yaw = 0.0001f;

    ui_element_config config;
    CLEAR_MEMORY(&config);
    config.type = UI_ELEMENT_CAMERA;
    config.parent = &viewport->element;
    config.renderCallback = ui_camera_render;
    ui_element_create(&camera->element, &config);

    arbiter_event_listener* listener = (arbiter_event_listener*)event_listener_create();
    listener->entitySelected = ui_camera_on_entity_selected;
    event_bus_listen(*(event_listener_generic*)listener, camera);

    ui_camera_update(camera, fovy, near, far);

    return camera;
}

void ui_camera_update(ui_camera* camera, float fovy, float near, float far) {
    camera->fovy = fovy;
    camera->near = near;
    camera->far = far;
    ui_camera_make_matrices(camera);
}