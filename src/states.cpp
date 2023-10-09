extern "C" {
#include "cJSON.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
}

#include "api.hpp"
#include "states.hpp"
#include <string>

static constexpr const char *TAG = "States";

// Create API request to home assistant with entity data
void post_entity(HAEntity &entity)
{
        if (!entity.entity_id[0]) {
                ESP_LOGE(TAG, "Failed to post entity. Entity members are invalid/null.");
                return;
        }
        cJSON *json_api_req = cJSON_CreateObject();

        cJSON_AddItemToObject(json_api_req, "state", cJSON_CreateString(entity.state.c_str()));
        cJSON_AddItemToObject(json_api_req, "attributes", cJSON_Duplicate(entity.attributes, true));

        char *jsonstr = cJSON_Print(json_api_req);
        // ESP_LOGI(TAG, "JSON Str - %s", jsonstr);

        char path[std::char_traits<char>::length(STATESPATH) + entity.entity_id.size() + 1 +
                  1]; // +1 for the / in the path
        snprintf(path, std::char_traits<char>::length(STATESPATH) + entity.entity_id.size() + 1 + 1, "%s/%s",
                 STATESPATH, entity.entity_id.c_str());

        // ESP_LOGI(TAG, "Path - %s", path);

        post_req(path, jsonstr, false);
        free(jsonstr);
        cJSON_Delete(json_api_req);
}

// ex. unit_of_measurement, friendly_name
void add_entity_attribute(const char *key, const char *value, HAEntity *entity)
{
        if (!entity) {
                ESP_LOGE(TAG, "Failed to add entity to attribute. Entity is null.");
                return;
        }

        if (!entity->attributes) {
                entity->attributes = cJSON_CreateObject();
        }

        ESP_LOGV(TAG, "Adding %s:%s to attributes", key, value);
        cJSON_AddItemToObject(entity->attributes, key, cJSON_CreateString(value));
}

static std::string get_entity_req(const std::string &entity_name)
{
        char path[256 + std::char_traits<char>::length(STATESPATH) + 1]; // +1 for the / in the path
        snprintf(path, 256 + std::char_traits<char>::length(STATESPATH) + 1, "%s/%s", STATESPATH, entity_name.c_str());
        std::string req = get_req(path);

        /* TODO: check error with std::string
        if (!req) {
                ESP_LOGE(TAG, "API entity GET request failed");
                return NULL;
        }
        */
        return req;
}

// Parses the entity str using cJSON
// Duplicates and assings the values from the parsed cJSON, then frees the cJSON
// Returned HAEntity must be manually freed with HAEntity_destroy()
static HAEntity *parse_entity_str(const std::string &entitystr)
{
        HAEntity *entity = new HAEntity;
        if (!entity) {
                ESP_LOGE(TAG, "Failed to malloc HAEntity.");
                return NULL;
        }

        cJSON *jsonreq = cJSON_Parse(entitystr.c_str());

        cJSON *state = cJSON_GetObjectItem(jsonreq, "state");
        if (cJSON_IsNull(state) || !cJSON_IsString(state)) {
                ESP_LOGI(TAG, "Entity has no state or is not a string.");
        } else {
                entity->state = cJSON_GetStringValue(state);
        }

        cJSON *entity_id = cJSON_GetObjectItem(jsonreq, "entity_id");
        if (cJSON_IsNull(entity_id) || !cJSON_IsString(entity_id)) {
                ESP_LOGI(TAG, "Entity has no entity_id or it is not a string.");
                entity->entity_id = "";
        } else {
                entity->entity_id = cJSON_GetStringValue(entity_id);
        }

        cJSON *last_changed = cJSON_GetObjectItem(jsonreq, "last_changed");
        if (cJSON_IsNull(last_changed) || !cJSON_IsString(last_changed)) {
                ESP_LOGI(TAG, "Entity has no last_changed or it is not a string.");
                entity->last_changed = "";
        } else {
                entity->last_changed = cJSON_GetStringValue(last_changed);
        }

        cJSON *last_updated = cJSON_GetObjectItem(jsonreq, "last_updated");
        if (cJSON_IsNull(last_updated) || !cJSON_IsString(last_updated)) {
                ESP_LOGI(TAG, "Entity has no last_updated or it is not a string.");
                entity->last_updated = "";
        } else {
                entity->last_updated = cJSON_GetStringValue(last_updated);
        }

        cJSON *attributes = cJSON_GetObjectItem(jsonreq, "attributes");
        if (cJSON_IsNull(attributes) || !cJSON_IsObject(attributes)) {
                ESP_LOGI(TAG, "Entity has no attributes or it is not a cJSON object.");
        } else {
                entity->attributes = cJSON_Duplicate(attributes, true);
        }

        cJSON_Delete(jsonreq);

        return entity;
}

HAEntity *get_entity(const std::string &entity_name)
{
        std::string entitystr = get_entity_req(entity_name);
        HAEntity *entity = parse_entity_str(entitystr);
        if (!entity) {
                ESP_LOGE(TAG, "Failed to get HAEntity");
                return NULL;
        }

        return entity;
}

static std::string get_states_req(void)
{
        std::string req = get_req(STATESPATH);

        /* TODO: Check for empty string which means it failed
          if (!req) {
                  ESP_LOGE(TAG, "API states GET request failed");
                  return NULL;
          }
        */
        return req;
}

static cJSON *parse_states_str(std::string statesstr)
{
        cJSON *jsonreq = cJSON_Parse(statesstr.c_str());
        return jsonreq;
}

// Get cJSON of all the states from Home Assistant
// Note: States might be really big. Mine is around 2400 char on a small install.
cJSON *get_states(void)
{
        std::string statesstr = get_states_req();
        cJSON *states = parse_states_str(statesstr);
        return states;
}

// Create API request to HA with entity data
void HAEntity::post()
{
        if (!entity_id[0]) {
                ESP_LOGE(TAG, "Failed to post entity. Entity or entity members are null.");
                return;
        }
        cJSON *json_api_req = cJSON_CreateObject();

        cJSON_AddItemToObject(json_api_req, "state", cJSON_CreateString(state.c_str()));
        cJSON_AddItemToObject(json_api_req, "attributes", cJSON_Duplicate(attributes, true));

        char *jsonstr = cJSON_Print(json_api_req);
        // ESP_LOGI(TAG, "JSON Str - %s", jsonstr);

        char path[std::char_traits<char>::length(STATESPATH) + entity_id.size() + 1 + 1]; // +1 for the / in the path
        snprintf(path, std::char_traits<char>::length(STATESPATH) + entity_id.size() + 1 + 1, "%s/%s", STATESPATH,
                 entity_id.c_str());

        // ESP_LOGI(TAG, "Path - %s", path);

        post_req(path, jsonstr, false);
        free(jsonstr);
        cJSON_Delete(json_api_req);
}

// ex. unit_of_measurement, friendly_name
void HAEntity::add_attribute(const char *key, const char *value)
{
        if (!attributes) {
                attributes = cJSON_CreateObject();
        }

        ESP_LOGV(TAG, "Adding %s:%s to attributes", key, value);
        cJSON_AddItemToObject(attributes, key, cJSON_CreateString(value));
}

void HAEntity::print()
{

        ESP_LOGV(TAG, "Printing HAEntity:");
        ESP_LOGI(TAG, "entity_id: %s", entity_id.c_str());

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
                ESP_LOGI(TAG, "last_changed: %s", last_changed.c_str());
        } else {
                ESP_LOGI(TAG, "no last_changed");
        }

        if (last_updated[0] != '\0') {
                ESP_LOGI(TAG, "last_updated: %s", last_updated.c_str());
        } else {
                ESP_LOGI(TAG, "no last_updated");
        }
}

HAEntity::~HAEntity()
{
        if (attributes != nullptr) {
                cJSON_Delete(attributes);
        }
}
