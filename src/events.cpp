#include "events.hpp"
#include "api.hpp"
#include "cJSON.h"
#include "esp_log.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

namespace esphalib
{
namespace event
{

namespace
{
constexpr const char *TAG = "Events";
std::string get_events_req(void)
{
        auto req = api::get_req(api::EVENTSPATH);

        if (req.response.empty() || req.status != api::RequestStatus_type::SUCCESS) {
                ESP_LOGE(TAG, "API events GET request failed");
        }
        return req.response;
}

cJSON *parse_events_str(const std::string &eventsstr)
{
        cJSON *jsonreq = nullptr;
        if (!eventsstr.empty()) {
                jsonreq = cJSON_Parse(eventsstr.c_str());
        }
        return jsonreq;
}
} // namespace

cJSON *get_events(void)
{
        auto req = api::get_req(api::EVENTSPATH);

        if (req.response.empty() || req.status != api::RequestStatus_type::SUCCESS) {
                ESP_LOGE(TAG, "API events GET request failed");
        }
        const std::string eventsstr = get_events_req();
        cJSON *events = parse_events_str(eventsstr);
        return events;
}

// Get single event by name from a cJSON array of events
HAEvent get_event_from_events(const char *event_type, cJSON *events)
{
        // TODO: Add safety checks for cJSON object (isarray, etc.)
        // Should the event.event be set to something like "Not Found" as the default case?
        HAEvent event = {.event = "", .listener_count = 0};
        for (int i = 0; i < cJSON_GetArraySize(events); i++) {
                cJSON *item = cJSON_GetArrayItem(events, i);
                char *itemstring = cJSON_GetStringValue(cJSON_GetObjectItem(item, "event"));
                // Check if event_type matches the event name
                if (itemstring != nullptr && std::strcmp(itemstring, event_type) == 0) {
                        // Copy itemstring into event.event
                        for (size_t i = 0; auto &e : event.event) {
                                auto const isi = itemstring[i];
                                e = isi;
                                if (isi == '\0') {
                                        break;
                                }
                                ++i;
                        }
                        // ensure null-terminated if itemstring is longer than event.event,
                        // but this should really not happen
                        event.event.back() = '\0';

                        event.listener_count =
                            static_cast<int32_t>(cJSON_GetNumberValue(cJSON_GetObjectItem(item, "listener_count")));
                        break;
                }
        }

        return event;
}

// Create API request to home assistant with events data
// Fires an event with event_type. You can pass an optional JSON object to be used as event_data
void post_event(const char *event_type, cJSON *event_data)
{
        cJSON *json_api_req = event_data ? cJSON_Duplicate(event_data, true) : cJSON_CreateObject();

        char *jsonstr = cJSON_Print(json_api_req);
        // ESP_LOGI(TAG, "JSON Str - %s", jsonstr);

        char path[sizeof(api::EVENTSPATH) + std::strlen(event_type) + 1 + 1]; // +1 for the / in the path
        snprintf(path, sizeof(api::EVENTSPATH) + std::strlen(event_type) + 1 + 1, "%s/%s", api::EVENTSPATH, event_type);

        // ESP_LOGI(TAG, "Path - %s", path);

        api::post_req(path, jsonstr, false);
        free(jsonstr);
        cJSON_Delete(json_api_req);
}
} // namespace event
} // namespace esphalib
