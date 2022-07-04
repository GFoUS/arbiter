#include "editor.h"

#define CIMGUI_USE_GLFW
#define CIMGUI_USE_VULKAN
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"
#include "cimgui_impl.h"

ui_editor* ui_editor_create(vulkan_context* ctx) {
    ui_editor* editor = malloc(sizeof(ui_editor));
    CLEAR_MEMORY(editor);

    editor->ctx = ctx;

    editor->dockspace = ui_dockspace_create();
    editor->viewport = ui_viewport_create(editor->dockspace, ctx);

    return editor;
}

void ui_editor_destroy(ui_editor* editor) {
    ui_element_destroy((ui_element*)editor->dockspace);

    free(editor);
}

void ui_editor_render(ui_editor* editor) {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    igNewFrame();

    ui_element_render((ui_element*)editor->dockspace);

    igRender();
}