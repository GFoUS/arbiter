#include "dockspace.h"

#define CIMGUI_USE_GLFW
#define CIMGUI_USE_VULKAN
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"

void ui_dockspace_render(ui_element* dockspaceElement, void(*body)(ui_element*)) {
    ui_dockspace* dockspace = (ui_dockspace*)dockspaceElement;

    ImVec2 zero = {.x=0, .y=0};
    const ImGuiViewport* viewport = igGetMainViewport();
    igSetNextWindowPos(viewport->WorkPos, ImGuiCond_Always, zero);
    igSetNextWindowSize(viewport->WorkSize, ImGuiCond_Always);
    igSetNextWindowViewport(viewport->ID);
    igPushStyleVar_Float(ImGuiStyleVar_WindowRounding, 0.0f);
    igPushStyleVar_Float(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus 
            | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDocking;

    igPushStyleVar_Vec2(ImGuiStyleVar_WindowPadding, zero);
    igBegin("Dockspace", &dockspace->element.open, windowFlags);
    igPopStyleVar(3);

    ImGuiID dockspaceID = igGetID_Str("Dockspace");
    igDockSpace(dockspaceID, zero, 0, NULL);

    body((ui_element*)dockspace);

    igEnd();
}

void ui_dockspace_destroy(ui_element* dockspaceElement) {}

ui_dockspace* ui_dockspace_create() {
    ui_dockspace* dockspace = malloc(sizeof(ui_dockspace));
    CLEAR_MEMORY(dockspace);

    ui_element_config config;
    config.type = UI_ELEMENT_DOCKSPACE;
    config.parent = NULL;
    config.destroyCallback = ui_dockspace_destroy;
    config.renderCallback = ui_dockspace_render;
    ui_element_create(&dockspace->element, &config);

    return dockspace;
}
