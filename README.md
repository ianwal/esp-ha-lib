# esp-ha-lib
WIP Library for interfacing with Home Assistants REST API using cJSON



# Installation

Using esp-idf
 - Download and extract the repository to the components folder of your ESP-IDF project
 - Include `esp_ha_lib.h` in your code


Using PlatformIO, on the `env` section of `platformio.ini`,  add the following:
```ini
lib_deps = https://github.com/ianwal/esp-ha-lib.git
```

And then finally include:

```#include "esp_ha_lib.h"```


## Usage

Before doing anything, you must set your Home Assistant URL and [Long Lived Access Token](https://developers.home-assistant.io/docs/auth_api/#long-lived-access-token) in order to access the api.

```c
#define HA_URL <your url e.g. http://homeassistant.com>
#define LONG_LIVED_ACCESS_TOKEN <your access token>

set_ha_url(HA_URL);
set_long_lived_access_token(LONG_LIVED_ACCESS_TOKEN);
```

### Entities:

Path: /api/states/<entity_id>

POST/GET using get_entity() and post_entity() using HAEntity struct

To POST an entity, the HAEntity req must have at least the entity_id. It should have the state as well since that is the main entity data value. 

To GET an entity, only the entity name is needed. For example, to get an entity named sensor.mysensor, just call get_entity("sensor.mysensor") and the corresponding HAEntity will be returned with entity data (or NULL if it fails). This must be manually freed using HAEntity_destroy().

Attributes can be added as a key:value pair using add_entity_attribute()

### Example:
To get an entity with the entity_id sun.sun

```c
// Create new entity
HAEntity* entity = HAEntity_create();

// GET request
entity = get_entity("sun.sun");

// Print
HAEntity_print(entity);

// Safely free
HAEntity_destroy(entity);
```

Output:

```sh
Entity: Attributes -

{
        "next_dawn":     "2023-05-18T11:53:26.495813+00:00",
        "next_dusk":     "2023-05-19T04:22:13.873111+00:00",
        "next_midnight": "2023-05-19T08:07:25+00:00",
        "next_noon":     "2023-05-18T20:07:20+00:00",
        "next_rising":   "2023-05-18T12:30:27.122095+00:00",
        "next_setting":  "2023-05-19T03:45:02.320003+00:00",
        "elevation":    -15.26,
        "azimuth":       36.74,
        "rising":        true,
        "friendly_name": "Sun"
}
```

---

## Progress:

GET
- [x] /api/
- [ ] /api/config
- [ ] /api/services
- [ ] /api/history/period/\<timestamp\>
- [ ] /api/logbook/\<timestamp\>
- [ ] /api/states
- [x] /api/states/\<entity_id\>
- [ ] /api/error_log
- [ ] /api/camera_proxy/\<camera entity_id\>
- [ ] /api/calendars
- [ ] /api/calendars/\<calendar entity_id\>

POST
- [ ] /api/states/\<entity_id\>
- [ ] /api/events/\<event_type\>
- [ ] /api/services/\<domain\>/\<service\>
- [ ] /api/template
- [ ] /api/config/core/check_config
- [ ] /api/intent/handle

[from home assistant docs](https://developers.home-assistant.io/docs/api/rest/)

