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
        auto const raw_req = api::get_req(api::CONFIGPATH);

        Document json_req;
        api::RequestResponse<rapidjson::Document> result;
        auto status = api::RequestStatus_type::SUCCESS;
        if (raw_req.status == api::RequestStatus_type::SUCCESS) {
                json_req.Parse(raw_req.response);
        } else {
                status = api::RequestStatus_type::FAILURE;
        }
        return api::RequestResponse<rapidjson::Document>{status, std::move(json_req)};
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
