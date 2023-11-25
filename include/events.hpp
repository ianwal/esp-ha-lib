#pragma once

#include "cJSON.h"

#include "api.hpp"
#include <array>
#include <cstdint>
#include <document.h>

namespace esphalib
{

namespace event
{

// 256 should be large enough for any event name
constexpr size_t MAX_EVENT_NAME_SIZE{256U};

class HAEvent
{
      public:
        std::array<char, MAX_EVENT_NAME_SIZE> event{'\0'};
        int32_t listener_count = 0;
};

// Returns success or failure and the parsed JSON for the config.
api::RequestResponse<rapidjson::Document> get_events(void);

// Get a single event by an event name from a valid JSON array from the events API response
// Returns an event with an empty name if it is not found
HAEvent get_event_from_events(std::string_view event_type, rapidjson::Document const &events);

void post_event(const char *event_type, cJSON *event_data);

} // namespace event
} // namespace esphalib
