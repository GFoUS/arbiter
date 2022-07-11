#include "menubar.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"

#include "project/project.h"

void ui_menubar_render(ui_element* menubarElement, void(*body)(ui_element*)) {
    ui_menubar* menubar = (ui_menubar*)menubarElement;

    if (igBeginMainMenuBar()) {
        if (igBeginMenu("File", true)) {
            if (igMenuItem_Bool("New Project", NULL, false, true)) {
                menubar->inNewProjectWizard = true;
                if (!menubar->newProjectName) free(menubar->newProjectName);
                menubar->newProjectName = malloc(sizeof(char) * 256);
                CLEAR_MEMORY_ARRAY(menubar->newProjectName, 256);
            }

            if (igMenuItem_Bool("Open Project", "Ctrl+O", false, true)) {
                fs_dialog_filter filter = {.name = "Project Files", .extension = "aproject"};
                const char* path = fs_open_dialog(1, &filter);
                if (path) project_load(path);
            }

            if (igMenuItem_Bool("Save Project", "Ctrl+S", false, !!project_get())) {
                fs_dialog_filter filter = {.name = "Project Files", .extension = "aproject"};
                const char* path = fs_save_dialog(1, &filter, project_get()->name);
                if (path) project_save(path);
            }

            igSeparator();

            if (igBeginMenu("Import", !!project_get())) {
                if (igMenuItem_Bool("Model", NULL, false, true)) {
                    fs_dialog_filter filter = {.name = "glTF Models", .extension = "gltf, glb"};
                    const char* path = fs_open_dialog(1, &filter);
                    if (path) project_add_asset(ASSET_TYPE_MODEL, path);
                }
                igEndMenu();
            }

            igEndMenu();
        }
        igEndMainMenuBar();
    }

    if (menubar->inNewProjectWizard) {
        if (!igIsPopupOpen_Str("Name?", 0)) igOpenPopup_Str("Name?", 0);

        igBeginPopupModal("Name?", NULL, ImGuiWindowFlags_AlwaysAutoResize);

        ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue;
        if (igInputText("Name: ", menubar->newProjectName, sizeof(char) * 256, flags, NULL, NULL)) {
            project_create(menubar->newProjectName);

            menubar->inNewProjectWizard = false;
        }

        igEndPopup();
    }

    body(menubarElement);
}

void ui_menubar_destroy(ui_element* menubarElement) {
    ui_menubar* menubar = (ui_menubar*)menubarElement;
    free(menubar->newProjectName);
}

ui_menubar* ui_menubar_create() {
    ui_menubar* menubar = malloc(sizeof(ui_menubar));
    CLEAR_MEMORY(menubar);

    ui_element_config config;
    config.type = UI_ELEMENT_MENU_BAR;
    config.parent = NULL;
    config.destroyCallback = ui_menubar_destroy;
    config.renderCallback = ui_menubar_render;
    ui_element_create((ui_element*)menubar, &config);

    menubar->inNewProjectWizard = false;
    menubar->newProjectName = NULL;

    return menubar;
}