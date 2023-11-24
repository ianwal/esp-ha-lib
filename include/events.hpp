#pragma once

#include "cJSON.h"

#include <array>
#include <cstdint>

namespace esphalib
{

namespace event
{

namespace
{
constexpr size_t MAX_EVENT_NAME_SIZE{256U};
} // namespace

class HAEvent
{
      public:
        std::array<char, MAX_EVENT_NAME_SIZE> event{'\0'}; // 256 should be large enough for any event name
        int32_t listener_count;
};

cJSON *get_events(void);
HAEvent get_event_from_events(const char *event_type, cJSON *events);
void post_event(const char *event_type, cJSON *event_data);

} // namespace event
} // namespace esphalib
