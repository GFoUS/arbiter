#pragma once

#include "exit_on_escape.h"

void layers_init() {
    exit_on_escape_init();
}

void layers_deinit() {
    exit_on_escape_deinit();
}