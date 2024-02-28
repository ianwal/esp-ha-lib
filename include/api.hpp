#pragma once

#include <cstdint>

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

// Set HA URL e.g. http://hassio.local:8123
void set_ha_url(std::string_view new_url);

// Set HA long lived access token
void set_long_lived_access_token(std::string_view new_long_lived_access_token);

// Get HA URL e.g. http://hassio.local:8123
std::string get_ha_url(void);

// Get HA long lived access token
std::string get_long_lived_access_token(void);

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
RequestResponse<std::string> post_req(std::string_view path, std::string_view data, bool return_response);
Status_type get_api_status(void);

} // namespace api
} // namespace esphalib
