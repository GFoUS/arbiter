#include "editor.h"

#define CIMGUI_USE_GLFW
#define CIMGUI_USE_VULKAN
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"
#include "cimgui_impl.h"

#include "cimguizmo.h"

#include "font.h"

ui_editor* ui_editor_create(vulkan_context* ctx) {
    ui_editor* editor = malloc(sizeof(ui_editor));
    CLEAR_MEMORY(editor);

    editor->ctx = ctx;

    editor->dockspace = ui_dockspace_create();
    editor->viewport = ui_viewport_create(editor->dockspace, ctx);
    editor->menubar = ui_menubar_create();
    editor->hierarchy = ui_world_hierarchy_create(editor->dockspace);
    editor->properties = ui_properties_create(editor->dockspace);

    ui_font_load(ctx, "assets/fonts/JetBrainsMono/JetBrainsMono-");

    return editor;
}

void ui_editor_destroy(ui_editor* editor) {
    ui_element_destroy((ui_element*)editor->dockspace);
    ui_element_destroy((ui_element*)editor->menubar);

    free(editor);
}

void ui_editor_render(ui_editor* editor) {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    igNewFrame();
    ImGuizmo_BeginFrame();

    ImGuiIO* io = igGetIO();
    igPushFont(io->Fonts->Fonts.Data[FONT_WEIGHT_MEDIUM]);

    ui_element_render((ui_element*)editor->menubar);
    ui_element_render((ui_element*)editor->dockspace);

    igPopFont();

    igRender();
}