#pragma once

#include <cstdint>
#define RAPIDJSON_HAS_STDSTRING 1
#include <document.h>
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

enum class Status_type : int8_t { ONLINE, OFFLINE, UNKNOWN };
enum class RequestStatus_type : int8_t { SUCCESS, FAILURE, UNKNOWN };

template <class T> struct RequestResponse {
        RequestStatus_type status;
        T response;
};

RequestResponse<std::string> get_req(const char *path);
std::string post_req(const char *path, const char *data, const bool return_response);
Status_type get_api_status(void);

namespace internal
{
using namespace rapidjson;
// Returns status and the parsed JSON for a given path
inline RequestResponse<rapidjson::Document> get_parsed_request(const char *path)
{
        auto const raw_req = api::get_req(path);

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
} // namespace internal

} // namespace api
} // namespace esphalib
