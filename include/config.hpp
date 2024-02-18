#pragma once

#include "api.hpp"

#define RAPIDJSON_HAS_STDSTRING 1
#include <document.h>

namespace esphalib
{

namespace config
{

api::RequestResponse<rapidjson::Document> get_config(void);
api::Config_Status_type check_config(void);

} // namespace config
} // namespace esphalib
