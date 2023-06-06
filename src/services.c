#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "cJSON.h"
#include "api.h"
#include "services.h"

static const char* TAG = "Services";

#define SERVICESPATH "/api/services"

static char* get_services_req(void)
{
    char* req = get_req(SERVICESPATH);
    
    if (!req) {
        ESP_LOGE(TAG, "API services GET request failed");
        return NULL;
    }
    //ESP_LOGI(TAG, "%s", req);

    return req;
}

static cJSON* parse_services_str(char* servicesstr)
{
    if (!servicesstr)
        return NULL;

    cJSON* jsonreq = cJSON_Parse(servicesstr);
    return jsonreq;
}

cJSON* get_services(void)
{
    char* servicesstr = get_services_req();
    cJSON* services = parse_services_str(servicesstr);
    free(servicesstr);
    return services;
}

// Get single event by name from a cJSON array of services
HAService get_service_from_domain(const char* domain, cJSON* services)
{
    HAService service = {.domain="", .services=NULL};

    cJSON *item = NULL;
    cJSON_ArrayForEach(item, services)
    {
        ESP_LOGE(TAG, "%s", cJSON_Print(item));
        if (strcmp(item->valuestring, domain) == 0)
        {
            strcpy(service.domain, domain);
            service.services = cJSON_GetObjectItem(item, "services");
            //printf("### currentDevSN exists in DevSNList_t  : %s\n", item->valuestring);
            break;
        }
    }

    return service;
}

/*
// Get single event by name from a cJSON array of events
HAEvent get_event_from_events(char* event_type, cJSON* events)
{
    // TODO: Add safety checks for cJSON object (isarray, etc.)   
    // Should the event.event be set to something like "Not Found" as the default case?
    HAEvent event = {.event="", .listener_count=0};
    for(int i = 0; i < cJSON_GetArraySize(events); i++) {
        cJSON* item = cJSON_GetArrayItem(events, i);
        char* itemstring = cJSON_GetStringValue(cJSON_GetObjectItem(item, "event"));
        // Check if event_type matches the event name
        if(strcmp(itemstring, event_type) == 0) {
            strcpy(event.event, itemstring);
            event.listener_count = (unsigned int)cJSON_GetNumberValue(cJSON_GetObjectItem(item, "listener_count"));
            break;
        }
    }

    return event;
}
*/