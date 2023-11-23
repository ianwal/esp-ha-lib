#pragma once

#include "cJSON.h"

namespace esphalib
{

namespace event
{

class HAEvent
{
      public:
        char event[256]; // this should be large enough for any event name
        unsigned int listener_count;
};

cJSON *get_events(void);
HAEvent get_event_from_events(const char *event_type, cJSON *events);
void post_event(const char *event_type, cJSON *event_data);

} // namespace event
} // namespace esphalib
