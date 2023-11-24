#pragma once

#include "api.hpp"
#include <document.h>

namespace esphalib
{

namespace config
{

api::RequestResponse<rapidjson::Document> get_config(void);
bool check_config(void);

} // namespace config
} // namespace esphalib
