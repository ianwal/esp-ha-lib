#pragma once

#include <cstdint>
#define RAPIDJSON_HAS_STDSTRING 1
#include <document.h>
#include <string>

namespace esphalib
{

namespace api
{

constexpr auto STATUSPATH{"/api/"};
constexpr auto EVENTSPATH{"/api/events"};
constexpr auto STATESPATH{"/api/states"};
constexpr auto CONFIGPATH{"/api/config"};
constexpr auto CHECKCONFIGPATH{"/api/config/core/check_config"};

inline std::string ha_url;
inline std::string long_lived_access_token;

// Set HA URL e.g. http://hassio.local:8123
constexpr void set_ha_url(std::string_view new_url) { ha_url = new_url; }

// Set HA long lived access token
constexpr void set_long_lived_access_token(std::string_view new_long_lived_access_token)
{
        long_lived_access_token = new_long_lived_access_token;
}

// Get HA URL e.g. http://hassio.local:8123
constexpr std::string get_ha_url(void) { return ha_url; }

// Get HA long lived access token
constexpr std::string get_long_lived_access_token(void) { return long_lived_access_token; }

// TODO: Add authentication response error code
enum class Status_type : int8_t { ONLINE, OFFLINE, UNKNOWN };

// Config response status
enum class Config_Status_type : int8_t { VALID, INVALID, UNKNOWN };

// Request response status
enum class RequestStatus_type : int8_t { SUCCESS, FAILURE, UNKNOWN };

template <class T> struct RequestResponse {
        RequestStatus_type status;
        T response;
};

RequestResponse<std::string> get_req(std::string_view path);
RequestResponse<std::string> post_req(std::string_view path, std::string_view data, const bool return_response);
Status_type get_api_status(void);

// Internal stuff used by multiple files.
namespace internal
{

using namespace rapidjson;

// Returns status and the parsed JSON for a given path
inline RequestResponse<rapidjson::Document> get_parsed_request(std::string_view path)
{
        auto const raw_req = api::get_req(path);

        Document json_req;
        api::RequestResponse<rapidjson::Document> result;
        auto status = api::RequestStatus_type::SUCCESS;
        if (raw_req.status == api::RequestStatus_type::SUCCESS) {
                json_req.Parse(raw_req.response);
                if (json_req.HasParseError()) {
                        status = api::RequestStatus_type::FAILURE;
                }
        } else {
                status = api::RequestStatus_type::FAILURE;
        }

        return api::RequestResponse<rapidjson::Document>{status, std::move(json_req)};
}

} // namespace internal

} // namespace api
} // namespace esphalib
