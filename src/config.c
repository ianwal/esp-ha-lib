#include "esp_log.h"
#include "cJSON.h"
#include "api.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#define CONFIGPATH "/api/config"
#define CHECKCONFIGPATH "/api/config/core/check_config"

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

// Returns true if the config is good
bool check_config(void)
{
    char* ok_response = "{\"result\":\"valid\",\"errors\":null}";
    char* response = post_req(CHECKCONFIGPATH, NULL, true);

    bool result;
    if (response && strcmp(ok_response, response) == 0) {
        ESP_LOGV(TAG, "%s", response);
        result = true;
    } else {
        result = false;
    }
   
    free(response);
    return result;
}