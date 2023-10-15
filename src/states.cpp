extern "C" {
#include "cJSON.h"
#include "esp_log.h"
}

#include "api.hpp"
#include "states.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

static constexpr const char *TAG = "States";

// ex. unit_of_measurement, friendly_name
void add_entity_attribute(const char *key, const char *value, HAEntity *entity)
{
        if (!entity) {
                ESP_LOGE(TAG, "Failed to add entity to attribute. Entity is null.");
                return;
        }

        if (entity->attributes == nullptr) {
                entity->attributes = cJSON_CreateObject();
        }

        ESP_LOGV(TAG, "Adding %s:%s to attributes", key, value);
        cJSON_AddItemToObject(entity->attributes, key, cJSON_CreateString(value));
}

std::string HAEntity::get_entity_req(const std::string &entity_name)
{
        const std::string path{std::string{STATESPATH} + "/" + entity_name};
        std::string req{get_req(path.c_str())};

        if (req.empty()) {
                ESP_LOGE(TAG, "API entity GET request failed");
        }
        return req;
}

// Parses the entity str using cJSON
// Duplicates and assings the values from the parsed cJSON, then frees the cJSON
HAEntity *HAEntity::parse_entity_str(const std::string &entitystr)
{
        HAEntity *entity = new HAEntity;
        if (!entity) {
                ESP_LOGE(TAG, "Failed to malloc HAEntity.");
                return nullptr;
        }

        cJSON *jsonreq = cJSON_Parse(entitystr.c_str());

        // Note: all the *_state_str are pointers that are freed by cJSON in the cJSON_Delete. Do not free manually.
        cJSON *json_state = cJSON_GetObjectItem(jsonreq, "state");
        if (cJSON_IsNull(json_state) || !cJSON_IsString(json_state)) {
                ESP_LOGI(TAG, "Entity has no state or is not a string.");
        } else {
                char *json_state_str = cJSON_GetStringValue(json_state);
                entity->state = std::string{json_state_str};
        }

        cJSON *json_entity_id = cJSON_GetObjectItem(jsonreq, "entity_id");
        if (cJSON_IsNull(json_entity_id) || !cJSON_IsString(json_entity_id)) {
                ESP_LOGI(TAG, "Entity has no entity_id or it is not a string.");
        } else {
                char *json_entity_id_str = cJSON_GetStringValue(json_entity_id);
                entity->entity_id = std::string{json_entity_id_str};
        }

        cJSON *json_last_changed = cJSON_GetObjectItem(jsonreq, "last_changed");
        if (cJSON_IsNull(json_last_changed) || !cJSON_IsString(json_last_changed)) {
                ESP_LOGI(TAG, "Entity has no last_changed or it is not a string.");
        } else {
                char *json_last_changed_str = cJSON_GetStringValue(json_last_changed);
                entity->last_changed = std::string{json_last_changed_str};
        }

        cJSON *json_last_updated = cJSON_GetObjectItem(jsonreq, "last_updated");
        if (cJSON_IsNull(json_last_updated) || !cJSON_IsString(json_last_updated)) {
                ESP_LOGI(TAG, "Entity has no last_updated or it is not a string.");
        } else {
                char *json_last_updated_str = cJSON_GetStringValue(json_last_updated);
                entity->last_updated = std::string{json_last_updated_str};
        }

        cJSON *json_attributes = cJSON_GetObjectItem(jsonreq, "attributes");
        if (cJSON_IsNull(json_attributes) || !cJSON_IsObject(json_attributes)) {
                ESP_LOGI(TAG, "Entity has no attributes or it is not a cJSON object.");
        } else {
                entity->attributes = cJSON_Duplicate(json_attributes, true);
        }

        cJSON_Delete(jsonreq);

        return entity;
}

HAEntity *HAEntity::get(const std::string &entity_name)
{
        const std::string entitystr{HAEntity::get_entity_req(entity_name)};
        HAEntity *entity{HAEntity::parse_entity_str(entitystr)};
        if (!entity) {
                ESP_LOGE(TAG, "Failed to get HAEntity");
        }

        return entity;
}

static std::string get_states_req(void)
{
        std::string req{get_req(STATESPATH)};

        if (req.empty()) {
                ESP_LOGE(TAG, "API states GET request failed");
        }
        return req;
}

static cJSON *parse_states_str(const std::string &states_str)
{
        cJSON *jsonreq = cJSON_Parse(states_str.c_str());
        return jsonreq;
}

// Get cJSON of all the states from Home Assistant
// Note: States might be really big. Mine is around 2400 char on a small install.
cJSON *get_states(void)
{
        const std::string states_str = get_states_req();
        cJSON *states = parse_states_str(states_str);
        return states;
}

// Create API request to HA with entity data
void HAEntity::post()
{
        if (entity_id.empty()) {
                ESP_LOGE(TAG, "Failed to post entity. Entity or entity members are null.");
                return;
        }
        cJSON *json_api_req = cJSON_CreateObject();

        cJSON_AddItemToObject(json_api_req, "state", cJSON_CreateString(state.c_str()));
        cJSON_AddItemToObject(json_api_req, "attributes", cJSON_Duplicate(attributes, true));

        char *jsonstr = cJSON_Print(json_api_req);

        const std::string path{std::string{STATESPATH} + "/" + entity_id};

        cJSON_Delete(json_api_req);
        post_req(path.c_str(), jsonstr, false);
        free(jsonstr);
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

        if (!state.empty()) {
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

        if (!last_changed.empty()) {
                ESP_LOGI(TAG, "last_changed: %s", last_changed.c_str());
        } else {
                ESP_LOGI(TAG, "no last_changed");
        }

        if (!last_updated.empty()) {
                ESP_LOGI(TAG, "last_updated: %s", last_updated.c_str());
        } else {
                ESP_LOGI(TAG, "no last_updated");
        }
}

HAEntity::~HAEntity()
{
        if (attributes != nullptr) {
                cJSON_Delete(attributes);
                attributes = nullptr;
        }
}
