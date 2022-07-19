#include "properties.h"

#include "project/ecs/ecs.h"
#include "project/project.h"
#include "font.h"
#include "colors.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"

void ui_properties_on_entity_selected(void* entity, void* selectedEntityPtr) {
    ecs_entity** selectedEntity = (ecs_entity**)selectedEntityPtr;
    *selectedEntity = (ecs_entity*)entity;
}

bool ui_properties_draw_xyz(const char* label, vec3 values) {
    ImVec2 zero = {0, 0};
    bool valueChanged = false;
            
    igPushID_Str(label);
    
    igPushMultiItemsWidths(3, igCalcItemWidth());

    igPushStyleColor_Vec4(ImGuiCol_Button, color_to_imvec4(RED600));
    igPushStyleColor_Vec4(ImGuiCol_ButtonActive, color_to_imvec4(RED600));
    igPushStyleColor_Vec4(ImGuiCol_ButtonHovered, color_to_imvec4(RED600));
    igButton("X", zero);
    igPopStyleColor(3); 

    igSameLine(0.0f, 0.0f);

    igPushStyleColor_Vec4(ImGuiCol_FrameBg, color_to_imvec4(RED300));
    igPushStyleColor_Vec4(ImGuiCol_FrameBgActive, color_to_imvec4(RED300));
    igPushStyleColor_Vec4(ImGuiCol_FrameBgHovered, color_to_imvec4(RED300));
    if (igDragFloat("##x_drag", &values[0], 0.1f, -FLT_MAX, +FLT_MAX, NULL, 0)) valueChanged = true;
    igPopStyleColor(3); 
    igPopItemWidth();

    igSameLine(0.0f, 0.0f);

    igPushStyleColor_Vec4(ImGuiCol_Button, color_to_imvec4(GREEN600));
    igPushStyleColor_Vec4(ImGuiCol_ButtonActive, color_to_imvec4(GREEN600));
    igPushStyleColor_Vec4(ImGuiCol_ButtonHovered, color_to_imvec4(GREEN600));
    igButton("Y", zero);
    igPopStyleColor(3); 

    igSameLine(0.0f, 0.0f);

    igPushStyleColor_Vec4(ImGuiCol_FrameBg, color_to_imvec4(GREEN300));
    igPushStyleColor_Vec4(ImGuiCol_FrameBgActive, color_to_imvec4(GREEN300));
    igPushStyleColor_Vec4(ImGuiCol_FrameBgHovered, color_to_imvec4(GREEN300));
    if (igDragFloat("##y_drag", &values[1], 0.1f, -FLT_MAX, +FLT_MAX, NULL, 0)) valueChanged = true;
    igPopStyleColor(3); 
    igPopItemWidth();

    igSameLine(0.0f, 0.0f);

    igPushStyleColor_Vec4(ImGuiCol_Button, color_to_imvec4(BLUE700));
    igPushStyleColor_Vec4(ImGuiCol_ButtonActive, color_to_imvec4(BLUE700));
    igPushStyleColor_Vec4(ImGuiCol_ButtonHovered, color_to_imvec4(BLUE700));
    igButton("Z", zero);
    igPopStyleColor(3); 

    igSameLine(0.0f, 0.0f);

    igPushStyleColor_Vec4(ImGuiCol_FrameBg, color_to_imvec4(BLUE400));
    igPushStyleColor_Vec4(ImGuiCol_FrameBgActive, color_to_imvec4(BLUE400));
    igPushStyleColor_Vec4(ImGuiCol_FrameBgHovered, color_to_imvec4(BLUE400));
    if (igDragFloat("##z_drag", &values[2], 0.1f, -FLT_MAX, +FLT_MAX, NULL, 0)) valueChanged = true;
    igPopStyleColor(3); 
    igPopItemWidth();

    igSameLine(0.0f, 0.0f);

    igPopID();

    return valueChanged;
}

void ui_properties_render_component(ui_properties* properties, ecs_component* component) {
    switch (component->type) {
    case (COMPONENT_TYPE_MODEL) : {
        if (igTreeNode_Str("Model")) {
            ecs_component_model* model = (ecs_component_model*)component;
            ImVec2 buttonSize;
            CLEAR_MEMORY(&buttonSize);

            if (model->asset) {
                igText("Model: %s", model->asset->path);
            } else {
                igText("Model: Not Selected");
            }

            if (igButton("Set Model", buttonSize)) {
                properties->inAssetSelection = true;
            }

            igTreePop();
        }

        igSeparator();
        break;
    }
    case (COMPONENT_TYPE_TAG) : {
        if (igTreeNode_Str("Tag")) {
            ecs_component_tag* tag = (ecs_component_tag*)component;
            ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue;
            char buf[MAX_TAG_LENGTH];
            CLEAR_MEMORY_ARRAY(buf, MAX_TAG_LENGTH);
            memcpy(buf, tag->tag, sizeof(char) * strlen(tag->tag));
            if (igInputText("Tag", buf, MAX_TAG_LENGTH, flags, NULL, NULL)) {
                ecs_component_tag_update(tag, buf);
            }

            igTreePop();
        }
        
        igSeparator();
        break;
    }
    case (COMPONENT_TYPE_TRANSFORM) : {
        if (igTreeNode_Str("Transform")) {
            ecs_component_transform* transform = (ecs_component_transform*)component;
            glm_vec3_copy(transform->translation, properties->translation);
            glm_vec3_copy(transform->rotation, properties->rotation);
            glm_vec3_copy(transform->scale, properties->scale);

            bool valueChanged = false;

            if (ui_properties_draw_xyz("Translation", properties->translation)) valueChanged = true;
            igText("Translation");

            if (ui_properties_draw_xyz("Rotation", properties->rotation)) valueChanged = true;
            igText("Rotation");

            if (ui_properties_draw_xyz("Scale", properties->scale)) valueChanged = true;
            igText("Scale");

            if (valueChanged) {
                ecs_component_transform_update(transform, properties->translation, properties->rotation, properties->scale);
            }

            igTreePop();
        }

        igSeparator();
        break;
    }
    }
}

void ui_properties_render(ui_element* propertiesElement, void(*body)(ui_element*)) {
    ui_properties* properties = (ui_properties*)propertiesElement;
    
    igBegin("Properties", &propertiesElement->open, 0);

    if (properties->selectedEntity != NULL) {
        ImGuiIO* io = igGetIO();
        igPushFont(io->Fonts->Fonts.Data[FONT_WEIGHT_BOLD]);
        ImVec2 buttonSize = {32,0};
        if (igButton("+", buttonSize)) {
            properties->inNewComponentWizard = true;
        }
        igPopFont();
        igSeparator();

        for (u32 i = 0; i < properties->selectedEntity->numComponents; i++) {
            ui_properties_render_component(properties, properties->selectedEntity->components[i]);
        }


        if (properties->inNewComponentWizard) {
            if (!igIsPopupOpen_Str("Component Type?", 0)) igOpenPopup_Str("Component Type?", 0);

            igBeginPopupModal("Component Type?", NULL, ImGuiWindowFlags_AlwaysAutoResize);

            igText("Model");
            if (igIsItemClicked(ImGuiMouseButton_Left)) {
                ecs_component_model_create(properties->selectedEntity, NULL);
                properties->inNewComponentWizard = false;
            }

            igText("Transform");
            if (igIsItemClicked(ImGuiMouseButton_Left)) {
                ecs_component_transform_create(properties->selectedEntity);
                properties->inNewComponentWizard = false;
            }

            igEndPopup();
        }

        if (properties->inAssetSelection) {
            if (!igIsPopupOpen_Str("Select Model", 0)) igOpenPopup_Str("Select Model", 0);

            igBeginPopupModal("Select Model", NULL, ImGuiWindowFlags_AlwaysAutoResize);

            ImVec2 assetButtonSize = {0,0};
            project_project* project = project_get();
            for (u32 i = 0; i < project->numAssets; i++) {
                if (project->assets[i].type == ASSET_TYPE_MODEL) {
                    if (igButton(project->assets[i].path, assetButtonSize)) {
                        ecs_component_model* model = (ecs_component_model*)ecs_entity_get_component(properties->selectedEntity, COMPONENT_TYPE_MODEL);
                        model->asset = &project->assets[i];
                        properties->inAssetSelection = false;
                    }
                }
            }

            igEndPopup();
        }
        
    }

    body(&properties->element);
    igEnd();
}

void ui_properties_destroy(ui_element* propertiesElement) {
    ui_properties* properties = (ui_properties*)propertiesElement;
    free(properties->listener);
}

ui_properties* ui_properties_create(ui_dockspace* parent) {
    ui_properties* properties = malloc(sizeof(ui_properties));
    CLEAR_MEMORY(properties);

    ui_element_config config;
    CLEAR_MEMORY(&config);
    config.type = UI_ELEMENT_PROPERTIES;
    config.parent = (ui_element*)parent;
    config.renderCallback = ui_properties_render;
    ui_element_create(&properties->element, &config);

    arbiter_event_listener* listener = (arbiter_event_listener*)event_listener_create();
    listener->entitySelected = ui_properties_on_entity_selected;
    event_bus_listen(*(event_listener_generic*)listener, &properties->selectedEntity);

    return properties;
}