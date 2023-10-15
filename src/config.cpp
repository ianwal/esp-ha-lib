
extern "C" {
#include "cJSON.h"
#include "esp_log.h"
}
#include "api.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

static constexpr const char *TAG = "Config";

static std::string get_config_req(void)
{
        std::string req = get_req(CONFIGPATH);

        if (req.empty()) {
                ESP_LOGE(TAG, "API GET request failed");
        }
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
        const std::string response = post_req(CHECKCONFIGPATH, nullptr, true);

        bool result;
        if (!response.empty() && response.compare(ok_response) == 0) {
                ESP_LOGV(TAG, "%s", response.c_str());
                result = true;
        } else {
                result = false;
        }

        return result;
}