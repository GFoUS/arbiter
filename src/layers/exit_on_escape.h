#pragma once

#include "aether.h"

#include "GLFW/glfw3.h"

void onKeyPress(void* data) {
    u32 key = *(u32*)data;
    if (key == GLFW_KEY_ESCAPE) {
        event_bus_submit(WINDOW_CLOSE, NULL);
    }
}

event_listener* exit_on_escape_get_listener() {
    static event_listener* listener = NULL;
    return listener;
}

void exit_on_escape_init() {
    event_listener* listener = exit_on_escape_get_listener();
    listener = malloc(sizeof(event_listener));
    CLEAR_MEMORY(listener);
    listener->keyPress = onKeyPress;
    event_bus_listen(*(event_listener_generic*)listener);
}

void exit_on_escape_deinit() {
    free(exit_on_escape_get_listener());
}