#pragma once

#include "cJSON.h"

#include <optional>
#include <string>

namespace esphalib
{

namespace state
{

class HAEntity
{
      public:
        std::string entity_id; // 256 char max
        std::string state;
        cJSON *attributes = cJSON_CreateObject(); // attributes is another json object
        std::string last_changed;    // ISO 8601 formatted datettime ex. 2016-05-30T21:43:29.204838+00:00 , 32 char max
        std::string last_updated;    // ISO 8601 formatted datettime, 32 char max

        // Create API request to HA with entity data
        void post() const;

        static std::optional<HAEntity> get(const std::string &entity_name);

        // ex. unit_of_measurement, friendly_name
        void add_attribute(const char *key, const char *value);
        void print() const;

        // Default-ctor
        HAEntity() = default;

        // Copy-Ctor
        HAEntity(HAEntity const &obj)
            : entity_id{obj.entity_id}, state{obj.state}, attributes{cJSON_Duplicate(attributes, true)},
              last_changed{obj.last_changed}, last_updated{obj.last_updated}
        {
        }

        // Copy-Assignment
        HAEntity &operator=(HAEntity const &obj)
        {
                if (this != &obj) {
                        entity_id = obj.entity_id;
                        state = obj.state;
                        attributes = cJSON_Duplicate(attributes, true);
                        last_changed = obj.last_changed;
                        last_updated = obj.last_updated;
                }
                return *this;
        }

        // Destructor
        ~HAEntity();

      private:
        static constexpr const char *TAG = "States";
        static std::optional<HAEntity> parse_entity_str(const std::string &entitystr);
        static std::string get_entity_req(const std::string &entity_name);
};

cJSON *get_states(void);

} // namespace state
} // namespace esphalib
