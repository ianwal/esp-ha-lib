#pragma once

#include "cJSON.h"

/*
Services has a similar structure to events
The HA API returns an array of service objects. Each object contains the domain and which services it contains.

Example API output:
[
    {
      "domain": "browser",
      "services": [
        "browse_url"
      ]
    },
    {
      "domain": "keyboard",
      "services": [
        "volume_up",
        "volume_down"
      ]
    }
]
*/

typedef struct {
    char domain[256]; // this should be large enough for any domain name
    cJSON* services;
} HAService;

cJSON* get_services(void);
HAService get_service_from_domain(const char* domain, cJSON* services);