#pragma once

#include "cJSON.h"

#include <string>

class HAEntity
{

      public:
        std::string entity_id; // 256 char max
        std::string state;
        cJSON *attributes = nullptr; // attributes is another json object
        std::string last_changed;    // ISO 8601 formatted datettime ex. 2016-05-30T21:43:29.204838+00:00 , 32 char max
        std::string last_updated;    // ISO 8601 formatted datettime, 32 char max

        // Create API request to HA with entity data
        void post();
        // ex. unit_of_measurement, friendly_name
        void add_attribute(const char *key, const char *value);
        void print();

        ~HAEntity();

      private:
        static constexpr const char *TAG = "States";
};

[[deprecated("Use HAEntity::add_attribute() instead.")]] void add_entity_attribute(const char *key, const char *value,
                                                                                   HAEntity *entity);
HAEntity *get_entity(const std::string &entity_name);

[[deprecated("Use HAEntity::post() instead.")]] void post_entity(HAEntity *entity);

void set_ha_url(const char *url);
void set_long_lived_access_token(const char *new_long_lived_access_token);

HAEntity *HAEntity_create(void);

cJSON *get_states(void);