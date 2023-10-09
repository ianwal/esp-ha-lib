#include "events.hpp"

extern "C" {
#include "cJSON.h"
#include "esp_log.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
}
#include "api.hpp"

static const char *TAG = "Events";

static char *get_events_req(void)
{
        char *req = get_req(EVENTSPATH);

        if (!req) {
                ESP_LOGE(TAG, "API events GET request failed");
                return NULL;
        }

        return req;
}

static cJSON *parse_events_str(char *eventsstr)
{
        if (!eventsstr)
                return NULL;

        cJSON *jsonreq = cJSON_Parse(eventsstr);
        return jsonreq;
}

cJSON *get_events(void)
{
        char *eventsstr = get_events_req();
        cJSON *events = parse_events_str(eventsstr);
        free(eventsstr);
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
                if (strcmp(itemstring, event_type) == 0) {
                        strcpy(event.event, itemstring);
                        event.listener_count =
                            (unsigned int)cJSON_GetNumberValue(cJSON_GetObjectItem(item, "listener_count"));
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

        char path[sizeof(EVENTSPATH) + strlen(event_type) + 1 + 1]; // +1 for the / in the path
        snprintf(path, sizeof(EVENTSPATH) + strlen(event_type) + 1 + 1, "%s/%s", EVENTSPATH, event_type);

        // ESP_LOGI(TAG, "Path - %s", path);

        post_req(path, jsonstr, false);
        free(jsonstr);
        cJSON_Delete(json_api_req);
}