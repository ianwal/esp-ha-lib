#pragma once

typedef struct {
    char event[256]; // this should be large enough for any event name
    unsigned int listener_count;
} HAEvent;

HAEvent* get_events(void);
void post_event(char* event_type, cJSON* event_data);