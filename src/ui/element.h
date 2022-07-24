#pragma once

#include "aether.h"

typedef enum {
    UI_ELEMENT_DOCKSPACE = 0,
    UI_ELEMENT_VIEWPORT,
    UI_ELEMENT_MENU_BAR,
    UI_ELEMENT_WORLD_HIERARCHY,
    UI_ELEMENT_PROPERTIES,
    UI_ELEMENT_CAMERA,
    UI_ELEMENT_GUIZMO
} ui_element_type;

typedef struct ui_element {
    u64 id;
    ui_element_type type;
    struct ui_element* parent;
    u32 numChildren;
    struct ui_element** children;

    bool open;

    void(*destroyCallback)(struct ui_element*);
    void(*renderCallback)(struct ui_element*, void(*body)(struct ui_element*));
} ui_element;

typedef struct {
    ui_element_type type;
    ui_element* parent;
    void(*destroyCallback)(ui_element*);
    void(*renderCallback)(ui_element*, void(*)(ui_element*));
} ui_element_config;

void ui_element_create(ui_element* element, ui_element_config* config);
void ui_element_destroy(ui_element* element);

void ui_element_render(ui_element* element);


