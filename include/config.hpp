#pragma once

#include "cJSON.h"

namespace esphalib
{

namespace config
{

cJSON *get_config(void);
bool check_config(void);

} // namespace config
} // namespace esphalib
