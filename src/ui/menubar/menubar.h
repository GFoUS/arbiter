#pragma once

#include "../element.h"

typedef struct {
    ui_element element;

    bool inNewProjectWizard;
    char* newProjectName;
} ui_menubar;

ui_menubar* ui_menubar_create();