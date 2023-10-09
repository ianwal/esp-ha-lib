#pragma once

#include "cJSON.h"

#include <string>

class HAEntity
{

      public:
        char entity_id[256] = ""; // definite 256 max from what I searched
        std::string state;
        cJSON *attributes = nullptr; // attributes is another json object
        char last_changed[48] = "";  // ISO 8601 formatted datettime ex. 2016-05-30T21:43:29.204838+00:00 = 32 char
        char last_updated[48] = "";  // ISO 8601 formatted datettime

        // Create API request to HA with entity data
        void post()
        {
                if (!entity_id) {
                        ESP_LOGE(TAG, "Failed to post entity. Entity or entity members are null.");
                        return;
                }
                cJSON *json_api_req = cJSON_CreateObject();

                cJSON_AddItemToObject(json_api_req, "state", cJSON_CreateString(state.c_str()));
                cJSON_AddItemToObject(json_api_req, "attributes", cJSON_Duplicate(attributes, true));

                char *jsonstr = cJSON_Print(json_api_req);
                // ESP_LOGI(TAG, "JSON Str - %s", jsonstr);

                char path[std::char_traits<char>::length(STATESPATH) + strlen(entity_id) + 1 +
                          1]; // +1 for the / in the path
                snprintf(path, std::char_traits<char>::length(STATESPATH) + strlen(entity_id) + 1 + 1, "%s/%s",
                         STATESPATH, entity_id);

                // ESP_LOGI(TAG, "Path - %s", path);

                post_req(path, jsonstr, false);
                free(jsonstr);
                cJSON_Delete(json_api_req);
        }

        // ex. unit_of_measurement, friendly_name
        void add_attribute(const char *key, const char *value)
        {
                if (!attributes) {
                        attributes = cJSON_CreateObject();
                }

                ESP_LOGV(TAG, "Adding %s:%s to attributes", key, value);
                cJSON_AddItemToObject(attributes, key, cJSON_CreateString(value));
        }

        void print()
        {

                ESP_LOGV(TAG, "Printing HAEntity:");
                ESP_LOGI(TAG, "entity_id: %s", entity_id);

                if (state.size() > 0) {
                        ESP_LOGI(TAG, "state: %s", state.c_str());
                } else {
                        ESP_LOGI(TAG, "no state");
                }

                if (cJSON_IsNull(attributes) || !cJSON_IsObject(attributes)) {
                        ESP_LOGI(TAG, "no attributes");
                } else {
                        char *jsonstr = cJSON_Print(attributes);
                        ESP_LOGI(TAG, "Attributes - %s", jsonstr);
                        free(jsonstr);
                }

                if (last_changed[0] != '\0') {
                        ESP_LOGI(TAG, "last_changed: %s", last_changed);
                } else {
                        ESP_LOGI(TAG, "no last_changed");
                }

                if (last_updated[0] != '\0') {
                        ESP_LOGI(TAG, "last_updated: %s", last_updated);
                } else {
                        ESP_LOGI(TAG, "no last_updated");
                }
        }

        ~HAEntity()
        {
                if (attributes != nullptr) {
                        cJSON_Delete(attributes);
                }
        }

      private:
        static constexpr const char *TAG = "States";
};

[[deprecated("Use HAEntity::add_attribute() instead.")]] void add_entity_attribute(const char *key, const char *value,
                                                                                   HAEntity *entity);
HAEntity *get_entity(const char *entity_name);

[[deprecated("Use HAEntity::post() instead.")]] void post_entity(HAEntity *entity);

void set_ha_url(const char *url);
void set_long_lived_access_token(const char *new_long_lived_access_token);

HAEntity *HAEntity_create(void);

cJSON *get_states(void);