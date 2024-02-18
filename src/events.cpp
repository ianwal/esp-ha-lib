#include "events.hpp"
#include "api.hpp"
#include "cJSON.h"
#include "common.hpp"
#include "esp_log.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <stringbuffer.h>
#include <writer.h>

namespace esphalib::event
{

namespace
{
constexpr auto TAG = "Events";
} // namespace

using RequestStatus_type = api::RequestStatus_type;
using Document = rapidjson::Document;
using api::RequestResponse;

RequestResponse<rapidjson::Document> get_events(void)
{
        return esphalib::internal::get_parsed_request(api::EVENTSPATH);
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
// Fires an event with event_type. You can pass a JSON object to be used as event_data.
RequestResponse<rapidjson::Document> post_event(std::string_view event_type, rapidjson::Document const &event_data)
{
        // ESP_LOGI(TAG, "JSON Str - %s", jsonstr);

        // char path[std::strlen(api::EVENTSPATH) + event_type.size() + 1 + 1]; // +1 for the / in the path
        // snprintf(path, std::strlen(api::EVENTSPATH) + event_type.size() + 1 + 1, "%s/%s", api::EVENTSPATH,
        // event_type);
        auto const path = std::string{api::EVENTSPATH} + "/" + std::string{event_type};

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        event_data.Accept(writer);
        auto const response = api::post_req(path, buffer.GetString(), true);
        Document response_json;
        if (response.status == RequestStatus_type::SUCCESS && response.response.size() != 0) {
                response_json.Parse(response.response);
                if (!response_json.HasParseError()) {
                        return {response.status, std::move(response_json)};
                }
        }
        return {response.status, {}};
        // return {response.status, std::move(response_json)};
}

// Create API request to Home Assistant and fires an event with event_type.
RequestResponse<rapidjson::Document> post_event(std::string_view event_type)
{
        auto const path = std::string{api::EVENTSPATH} + "/" + std::string{event_type};
        auto const response = api::post_req(path, "", false);
        Document response_json;
        if (response.status == RequestStatus_type::SUCCESS) {
                response_json.Parse(response.response);
        }
        // Required to move response_json because rapidjson::GenericDocument has deleted copy constructor.
        return {response.status, std::move(response_json)};
}

} // namespace esphalib::event
