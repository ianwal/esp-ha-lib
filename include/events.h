#pragma once

#include "cJSON.h"

typedef struct {
    char event[256]; // this should be large enough for any event name
    unsigned int listener_count;
} HAEvent;

cJSON *get_events(void);
HAEvent get_event_from_events(char* event_type, cJSON* events);
void post_event(char* event_type, cJSON* event_data);