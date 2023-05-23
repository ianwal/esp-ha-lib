#pragma once

#include "cJSON.h"
#include <stdbool.h>

cJSON* get_config(void);
bool check_config(void);