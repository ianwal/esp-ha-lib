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
} // namespace

api::RequestResponse<rapidjson::Document> get_events(void)
{
        return api::internal::get_parsed_request(api::EVENTSPATH);
}

HAEvent get_event_from_events(std::string_view event_type, rapidjson::Document const &events_json)
{
        HAEvent event;

        for (auto &event_entry : events_json.GetArray()) {
                if (auto event_name_it = event_entry.FindMember("event"); event_name_it != event_entry.MemberEnd()) {
                        auto const event_name = (*event_name_it).value.GetString();
                        // if we find the event name we're searching for
                        if (event_type.compare(event_name) == 0) {
                                // Copy event name into event.event
                                for (size_t i = 0; auto &e : event.event) {
                                        auto const isi = event_name[i];
                                        e = isi;
                                        if (isi == '\0') {
                                                break;
                                        }
                                        ++i;
                                }
                                // ensure null-terminated even if itemstring is longer than event.event,
                                // but that should really not happen
                                event.event.back() = '\0';

                                // Get listener count
                                if (auto listener_count_it = event_entry.FindMember("listener_count");
                                    listener_count_it != event_entry.MemberEnd()) {
                                        auto const listener_count = (*listener_count_it).value.GetInt();
                                        event.listener_count = listener_count;
                                }
                                break;
                        }
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
