#include "properties.h"

#include "project/ecs/ecs.h"
#include "project/project.h"
#include "font.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"

static ecs_entity* selectedEntity = NULL;
void on_entity_selected(void* entity) {
    selectedEntity = (ecs_entity*)entity;
}

void ui_properties_render_component(ui_properties* properties, ecs_component* component) {
    switch (component->type) {
    case(COMPONENT_TYPE_MODEL) : {
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

        break;
    }
    case(COMPONENT_TYPE_TAG) : {
        ecs_component_tag* tag = (ecs_component_tag*)component;
        ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue;
        char buf[MAX_TAG_LENGTH];
        CLEAR_MEMORY_ARRAY(buf, MAX_TAG_LENGTH);
        memcpy(buf, tag->tag, sizeof(char) * strlen(tag->tag));
        if (igInputText("Tag", buf, MAX_TAG_LENGTH, flags, NULL, NULL)) {
            ecs_component_tag_update(tag, buf);
        }
    }
    }
}

void ui_properties_render(ui_element* propertiesElement, void(*body)(ui_element*)) {
    ui_properties* properties = (ui_properties*)propertiesElement;
    
    igBegin("Properties", &propertiesElement->open, 0);

    if (selectedEntity != NULL) {
        ImGuiIO* io = igGetIO();
        igPushFont(io->Fonts->Fonts.Data[FONT_WEIGHT_BOLD]);
        ImVec2 buttonSize = {32,0};
        if (igButton("+", buttonSize)) {
            properties->inNewComponentWizard = true;
        }
        igPopFont();

        for (u32 i = 0; i < selectedEntity->numComponents; i++) {
            ui_properties_render_component(properties, selectedEntity->components[i]);
        }


        if (properties->inNewComponentWizard) {
            if (!igIsPopupOpen_Str("Component Type?", 0)) igOpenPopup_Str("Component Type?", 0);

            igBeginPopupModal("Component Type?", NULL, ImGuiWindowFlags_AlwaysAutoResize);

            igText("Model");
            if (igIsItemClicked(ImGuiMouseButton_Left)) {
                ecs_component_model_create(selectedEntity, NULL);
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
                        ecs_component_model* model = (ecs_component_model*)ecs_entity_get_component(selectedEntity, COMPONENT_TYPE_MODEL);
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
    listener->entitySelected = on_entity_selected;
    event_bus_listen(*(event_listener_generic*)listener);

    return properties;
}