#pragma once

#include "api.hpp"

#include "rapidjson_wrapper.hpp"

#include <string_view>

namespace esphalib::internal
{

// Get parsed JSON and status for a Home Assistant API.
api::RequestResponse<rapidjson::Document> get_parsed_request(std::string_view path);

} // namespace esphalib::internal
