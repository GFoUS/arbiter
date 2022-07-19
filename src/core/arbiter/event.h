#pragma once

#include "core/event.h"

typedef enum {
    ENTITY_SELECTED = LAST_EVENT_TYPE
} arbiter_event_type;

typedef struct {
    event_listener listener;
    void(*entitySelected)(void*, void*);
} arbiter_event_listener;

