#pragma once

#include "freertos/FreeRTOS.h"
#include <cstddef>

namespace esphalib
{
namespace Testing
{
namespace Wifi
{

bool wait_wifi_connected(TickType_t timeout);
void wifi_init_station(void);
void stop_wifi(void);

} // namespace Wifi
} // namespace Testing
} // namespace esphalib
