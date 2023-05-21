#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "entity.h"
#include "cJSON.h"
#include "api.h"
#include "events.h"

static const char* TAG = "Events";

#define eventspath "/api/events/"

static char* get_events_req(void)
{
    char* req = get_req(eventspath);
    
    if (!req) {
        ESP_LOGE(TAG, "API events GET request failed");
        return NULL;
    }

    return req;
}

static HAEvent* parse_events_str(char* eventsstr)
{
    if (!eventsstr)
        return NULL;

    cJSON* jsonreq = cJSON_Parse(eventsstr);
    free(eventsstr);

    HAEvent* events = malloc(sizeof(HAEvent) * cJSON_GetArraySize(jsonreq));
    //ESP_LOGI(TAG, "ARRAY SIZE %d", cJSON_GetArraySize(jsonreq));

    for(int i = 0; i < cJSON_GetArraySize(jsonreq); i++) {
        HAEvent event;
        cJSON* item = cJSON_GetArrayItem(jsonreq, i);
        strcpy(event.event, cJSON_GetStringValue(cJSON_GetObjectItem(item, "event")));
        //ESP_LOGI(TAG, "event %s", event.event);
        event.listener_count = (unsigned int)cJSON_GetNumberValue(cJSON_GetObjectItem(item, "listener_count"));
        //ESP_LOGI(TAG, "listener_count %d", event.listener_count);
        events[i] = event;
    }

    cJSON_Delete(jsonreq);
    return events;
}

HAEvent* get_events(void)
{
    char* eventstr = get_events_req();
    return parse_events_str(eventstr);
}

// Create API request to home assistant with events data  
// Fires an event with event_type. You can pass an optional JSON object to be used as event_data
void post_event(char* event_type, cJSON* event_data)
{
    cJSON* json_api_req;
    if(event_data) {
        json_api_req = cJSON_Duplicate(event_data, true);
    } else {
        json_api_req = cJSON_CreateObject();
    }

    char* jsonstr = cJSON_Print(json_api_req);
    //ESP_LOGI(TAG, "JSON Str - %s", jsonstr);

    char path[sizeof(eventspath)+strlen(event_type)+1];
    snprintf(path, sizeof(eventspath)+strlen(event_type)+1, "%s%s", eventspath, event_type);
    
    //ESP_LOGI(TAG, "Path - %s", path);
    
    post_req(path, jsonstr);
    free(jsonstr);
    cJSON_Delete(json_api_req);
}