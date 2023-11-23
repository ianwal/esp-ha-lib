#include "api.hpp"
#include "cJSON.h"
#include "esp_log.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

namespace esphalib
{

namespace config
{

namespace
{
constexpr const char *TAG = "Config";

namespace api = esphalib::api;

std::string get_config_req(void)
{
        std::string req = api::get_req(api::CONFIGPATH);

        if (req.empty()) {
                ESP_LOGE(TAG, "API GET request failed");
        }
        return req;
}

cJSON *parse_config_str(const std::string &configstr)
{
        cJSON *jsonreq = cJSON_Parse(configstr.c_str());
        return jsonreq;
}

} // namespace

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
        const std::string response = api::post_req(api::CHECKCONFIGPATH, nullptr, true);

        bool result;
        if (!response.empty() && response.compare(ok_response) == 0) {
                ESP_LOGV(TAG, "%s", response.c_str());
                result = true;
        } else {
                result = false;
        }

        return result;
}

} // namespace config
} // namespace esphalib
