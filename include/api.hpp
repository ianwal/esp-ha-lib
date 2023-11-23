#pragma once

#include <cstdint>
#include <string>

namespace esphalib
{

namespace api
{

constexpr const char *EVENTSPATH = "/api/events";
constexpr const char *STATESPATH = "/api/states";
constexpr const char *CONFIGPATH = "/api/config";
constexpr const char *CHECKCONFIGPATH = "/api/config/core/check_config";

extern std::string ha_url;
extern std::string long_lived_access_token;

void set_ha_url(const char *new_url);
void set_long_lived_access_token(const char *new_long_lived_access_token);

enum class APIStatus_type : int8_t { ONLINE, OFFLINE, UNKNOWN };

std::string get_req(const char *path);
std::string post_req(const char *path, const char *data, const bool return_response);
APIStatus_type get_api_status(void);

} // namespace api
} // namespace esphalib
