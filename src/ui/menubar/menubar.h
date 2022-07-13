#pragma once

#include "../element.h"

typedef struct {
    ui_element element;

    bool inNewProjectWizard;
    bool inAddEntityWizard;
    char* buf;

} ui_menubar;

ui_menubar* ui_menubar_create();