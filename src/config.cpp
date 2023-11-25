#include "api.hpp"
#include "esp_log.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <document.h>
#include <string>

namespace esphalib
{

namespace config
{

namespace
{
constexpr const char *TAG = "Config";

namespace api = esphalib::api;

} // namespace

using namespace rapidjson;

// Returns success or failure and the parsed JSON for the config.
api::RequestResponse<rapidjson::Document> get_config(void)
{
        return api::internal::get_parsed_request(api::CONFIGPATH);
}

// Returns true if the config is good
bool check_config(void)
{
        constexpr const char *ok_response = "{\"result\":\"valid\",\"errors\":null}";
        auto const response = api::post_req(api::CHECKCONFIGPATH, "", true);

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
