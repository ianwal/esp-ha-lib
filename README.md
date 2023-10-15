# esp-ha-lib
WIP CPP library to interface with [Home Assistant's REST API](https://developers.home-assistant.io/docs/api/rest/) using [cJSON](https://github.com/DaveGamble/cJSON)



# Installation

Using esp-idf
 - Download and extract the repository to the components folder of your ESP-IDF project
 - Include `esp_ha_lib.hpp` in your code


Using PlatformIO, on the `env` section of `platformio.ini`,  add the following:
```ini
lib_deps = https://github.com/ianwal/esp-ha-lib.git
```

And then finally include:

```#include "esp_ha_lib.hpp"```


## Usage

Before doing anything, you must set your Home Assistant URL and [Long Lived Access Token](https://developers.home-assistant.io/docs/auth_api/#long-lived-access-token) in order to access the api.

```c
constexpr const char *HA_URL = <your url e.g. http://homeassistant.com>;
constexpr const char *LONG_LIVED_ACCESS_TOKEN = <your access token>;

set_ha_url(HA_URL);
set_long_lived_access_token(LONG_LIVED_ACCESS_TOKEN);
```

### Entities:

Path: /api/states/<entity_id>

POST/GET using HAEntity::post() and HAEntity::get().

To POST an entity, the HAEntity req must have at least the entity_id. It should have the state as well since that is the main entity data value. 

To GET an entity, only the entity name is needed. For example, to get an entity named sensor.mysensor, just call HAEntity::get("sensor.mysensor") and the corresponding HAEntity will be returned with entity data (or nullptr if it fails).

Attributes can be added as a key:value pair using HAEntity::add_attribute()

### Example:
To get an entity with the entity_id "sun.sun"

```c
// Create new entity with GET request
HAEntity* entity{HAEntity::get("sun.sun")};

// Print
entity.print();

// Safely free
delete entity;
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
- [x] /api/config
- [x] /api/events
- [ ] /api/services
- [ ] /api/history/period/\<timestamp\>
- [ ] /api/logbook/\<timestamp\>
- [x] /api/states
- [x] /api/states/\<entity_id\>
- [ ] /api/error_log
- [ ] /api/camera_proxy/\<camera entity_id\>
- [ ] /api/calendars
- [ ] /api/calendars/\<calendar entity_id\>

POST
- [x] /api/states/\<entity_id\>
- [x] /api/events/\<event_type\>
- [ ] /api/services/\<domain\>/\<service\>
- [ ] /api/template
- [x] /api/config/core/check_config
- [ ] /api/intent/handle

[from home assistant docs](https://developers.home-assistant.io/docs/api/rest/)

## Development:

Use the official Home Assistant docker container for testing.

At this time, it is required for the ESP32 and development PC to share the same network. The ESP32 needs to be able to connect through Wi-Fi. I am looking into a way to circumvent this.

Requirements: 
- docker and docker-compose

### Steps:

1. Fill in your secrets in `src/secrets.hpp`, `test/secrets.hpp`, and `wifisecrets.h`
    - `HA_URL` is the HOST PC IP e.g. http://192.168.1.72:8123
        - I am looking into a way to avoid/automate this
    - Long Lived Access Token is already generated and does not need to be filled in
2. Launch the docker container by running 

```sh
docker run -d --name homeassistant --privileged --restart=unless-stopped -e TZ=America/Los_Angeles -v ./docker/config:/config -p 8123:8123 ghcr.io/home-assistant/home-assistant:stable
```

or use the docker-compose in docker/

3. Test using PlatformIO.

If you would like to connect to the Home Assistant frontend:

username: `user`

password: `1`
