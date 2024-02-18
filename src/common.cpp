#include "api.hpp"

#include "common.hpp"
#include <string_view>

namespace esphalib::internal
{

using api::RequestResponse;
using rapidjson::Document;

// Returns status and the parsed JSON for a given path
RequestResponse<Document> get_parsed_request(std::string_view path)
{
        Document json_req;
        RequestResponse<Document> result;
        auto status = api::RequestStatus_type::SUCCESS;
        auto const raw_req = api::get_req(path);
        if (raw_req.status == api::RequestStatus_type::SUCCESS) {
                json_req.Parse(raw_req.response);
                if (json_req.HasParseError()) {
                        status = api::RequestStatus_type::FAILURE;
                }
        } else {
                status = api::RequestStatus_type::FAILURE;
        }

        return RequestResponse<Document>{status, std::move(json_req)};
}

} // namespace esphalib::internal
