
extern "C" {
#include "cJSON.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
}
#include "api.hpp"
#include <string>

static constexpr const char *TAG = "Config";

static std::string get_config_req(void)
{
        std::string req = get_req(CONFIGPATH);

        /* TODO: Fix
        if (!req) {
                ESP_LOGE(TAG, "API GET request failed");
                return NULL;
        }
        */
        return req;
}

static cJSON *parse_config_str(const std::string &configstr)
{
        cJSON *jsonreq = cJSON_Parse(configstr.c_str());
        return jsonreq;
}

cJSON *get_config(void)
{
        const std::string configstr = get_config_req();
        cJSON *config = parse_config_str(configstr);
        return config;
}

// Returns true if the config is good
bool check_config(void)
{
        constexpr const char *ok_response = "{\"result\":\"valid\",\"errors\":null}";
        char *response = post_req(CHECKCONFIGPATH, nullptr, true);

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