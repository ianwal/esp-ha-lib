#include "esp_log.h"
#include "entity.h"
#include "cJSON.h"
#include "api.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#define CONFIGPATH "/api/config"

static const char *TAG = "Config";

static char* get_config_req(void)
{
    char* req = get_req(CONFIGPATH);
    
    if (!req) {
        ESP_LOGE(TAG, "API GET request failed");
        return NULL;
    }

    return req;
}

static cJSON* parse_config_str(char* configstr)
{
    if (!configstr)
        return NULL;

    cJSON* jsonreq = cJSON_Parse(configstr);
    return jsonreq;
}

cJSON* get_config(void)
{
    char* configstr = get_config_req();
    cJSON* config = parse_config_str(configstr);
    free(configstr);
    return config;
}