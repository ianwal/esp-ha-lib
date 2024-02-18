#include "api.hpp"
#include "common.hpp"
#include "esp_log.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

namespace esphalib::config
{

namespace
{
constexpr auto TAG = "Config";
} // namespace

using RequestStatus_type = api::RequestStatus_type;
using Config_Status_type = api::Config_Status_type;

// Returns success or failure and the parsed JSON for the config.
api::RequestResponse<rapidjson::Document> get_config(void)
{
        return esphalib::internal::get_parsed_request(api::CONFIGPATH);
}

// Get the config status
Config_Status_type check_config(void)
{
        // TODO: Print error from "result" if config is invalid
        constexpr auto valid_response = "{\"result\":\"valid\",\"errors\":null}";
        auto const response = api::post_req(api::CHECKCONFIGPATH, "", true);

        Config_Status_type result;
        if (response.status == RequestStatus_type::SUCCESS) {
                if (!response.response.empty() && response.response.compare(valid_response) == 0) {
                        result = Config_Status_type::VALID;
                } else {
                        // TODO: Compare to invalid response
                        result = Config_Status_type::INVALID;
                }
        } else {
                result = Config_Status_type::UNKNOWN;
        }

        return result;
}

} // namespace esphalib::config
