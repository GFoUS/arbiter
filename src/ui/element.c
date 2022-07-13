#include "element.h"

void ui_element_create(ui_element* element, ui_element_config* config) {
    static nextID = 0;

    element->id = nextID++;
    element->type = config->type;
    element->parent = config->parent;
    element->children = malloc(0);

    element->destroyCallback = config->destroyCallback;
    element->renderCallback = config->renderCallback;

    if (element->parent != NULL) {
        element->parent->numChildren++;
        element->parent->children = realloc(element->parent->children, sizeof(ui_element*) * element->parent->numChildren);
        element->parent->children[element->parent->numChildren - 1] = element;
    }
}

void ui_element_destroy(ui_element* element) {
    if (element->destroyCallback) element->destroyCallback(element);

    for (u32 i = 0; i < element->numChildren; i++) {
        ui_element_destroy(element->children[i]);
    }
    free(element->children);
    
    free(element);
}

void ui_element_render_body(ui_element* element) {
    for (u32 i = 0; i < element->numChildren; i++) {
        if (element->children[i]->renderCallback) element->children[i]->renderCallback(element->children[i], ui_element_render_body);
    }
}

void ui_element_render(ui_element* element) {
    if (element->renderCallback) element->renderCallback(element, ui_element_render_body);
}

