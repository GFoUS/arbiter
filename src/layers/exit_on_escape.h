#pragma once

#include "aether.h"

#include "GLFW/glfw3.h"

void onKeyPress(void* data, void* userPtr) {
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
    listener = event_listener_create();
    listener->keyPress = onKeyPress;
    event_bus_listen(*(event_listener_generic*)listener, NULL);
}

void exit_on_escape_deinit() {
    free(exit_on_escape_get_listener());
}