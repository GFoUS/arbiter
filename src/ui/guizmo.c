#include "guizmo.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"
#include "cimguizmo.h"

#include "viewport.h"

void ui_guizmo_on_entity_selected(void* entityPtr, void* guizmoPtr) {
    ui_guizmo* guizmo = (ui_guizmo*)guizmoPtr;
    guizmo->selectedEntity = (ecs_entity*)entityPtr;
}

void ui_guizmo_render(ui_element* guizmoElement, void(*body)(ui_element*)) {
    ui_guizmo* guizmo = (ui_guizmo*)guizmoElement;

    body(&guizmo->element);

    if (guizmo->selectedEntity != NULL) {
        ecs_component_transform* transform = (ecs_component_transform*)ecs_entity_get_component(guizmo->selectedEntity, COMPONENT_TYPE_TRANSFORM);
        ecs_component_model* model = (ecs_component_model*)ecs_entity_get_component(guizmo->selectedEntity, COMPONENT_TYPE_MODEL);
        if (transform != NULL && model != NULL && model->asset->numTimesLoaded != 0) {
            ImGuizmo_Enable(true);
            mat4 modelTransform;
            project_asset_model* asset = (project_asset_model*)model->asset->loadedData;
            model_get_node_matrix(asset->model, modelTransform);
            glm_mat4_mul(transform->matrix, modelTransform, modelTransform);
            ImGuizmo_Manipulate((float*)guizmo->camera->view, (float*)guizmo->camera->proj, TRANSLATE, LOCAL, (float*)modelTransform, NULL, NULL, NULL, NULL);
        } 
    }

    ImGuizmo_Enable(false);
}

ui_guizmo* ui_guizmo_create(ui_viewport* viewport, ui_camera* camera) {
    ui_guizmo* guizmo = malloc(sizeof(ui_guizmo));
    CLEAR_MEMORY(guizmo);

    ui_element_config config = {0};
    config.type = UI_ELEMENT_GUIZMO;
    config.parent = &viewport->element;
    config.renderCallback = ui_guizmo_render;
    ui_element_create(&guizmo->element, &config);

    arbiter_event_listener* listener = (arbiter_event_listener*)event_listener_create();
    listener->entitySelected = ui_guizmo_on_entity_selected;
    event_bus_listen(*(event_listener_generic*)listener, (void*)guizmo);

    guizmo->viewport = viewport;
    guizmo->camera = camera;

    return guizmo;    
}