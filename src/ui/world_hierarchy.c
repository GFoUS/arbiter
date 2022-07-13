#include "world_hierarchy.h"

#include "project/ecs/ecs.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"

void ui_world_hierarchy_render(ui_element* hierarchyElement, void(*body)(ui_element*)) {
    ui_world_hierarchy* hierarchy = (ui_world_hierarchy*)hierarchyElement;

    igBegin("World Hierarchy", &hierarchy->element.open, 0);

    ecs_world* world = ecs_world_get();
    for (u32 i = 0; i < world->numEntities; i++) {
        ecs_component_tag* tagComponent = ecs_entity_get_component(world->entities[i], COMPONENT_TYPE_TAG);
        if (tagComponent != NULL) {
            if (igTreeNodeEx_Str(tagComponent->tag, ImGuiTreeNodeFlags_OpenOnArrow)) {
                igTreePop();
            }

            if (igIsItemClicked(ImGuiMouseButton_Left)) {
                INFO("Clicked on entity: %s", tagComponent->tag);
            }
        }
    }

    body(&hierarchy->element);

    igEnd();
}

ui_world_hierarchy* ui_world_hierarchy_create(ui_dockspace* parent) {
    ui_world_hierarchy* hierarchy = malloc(sizeof(ui_world_hierarchy));
    CLEAR_MEMORY(hierarchy);

    ui_element_config config;
    CLEAR_MEMORY(&config);
    config.type = UI_ELEMENT_WORLD_HIERARCHY;
    config.parent = &parent->element;
    config.renderCallback = ui_world_hierarchy_render;

    ui_element_create(&hierarchy->element, &config);

    return hierarchy;
}