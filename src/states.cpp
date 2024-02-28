#include "states.hpp"
#include "api.hpp"
#include "cJSON.h"
#include "esp_log.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

namespace esphalib
{

namespace state
{

namespace
{

constexpr auto TAG = "States";

std::string get_states_req()
{
        auto const req = api::get_req(api::STATESPATH);
        return req.response;
}

cJSON *parse_states_str(const std::string &states_str)
{
        cJSON *jsonreq = nullptr;
        if (!states_str.empty()) {
                jsonreq = cJSON_Parse(states_str.c_str());
        }
        return jsonreq;
}

} // namespace

// ex. unit_of_measurement, friendly_name
void add_entity_attribute(const char *key, const char *value, HAEntity& entity)
{
        if (entity.attributes == nullptr) {
                entity.attributes = cJSON_CreateObject();
        }

        ESP_LOGV(TAG, "Adding %s:%s to attributes", key, value);
        cJSON_AddItemToObject(entity.attributes, key, cJSON_CreateString(value));
}

std::string HAEntity::get_entity_req(const std::string &entity_name)
{
        const std::string path{std::string{api::STATESPATH} + "/" + entity_name};
        auto req = api::get_req(path.c_str());

        if (req.response.empty() || req.status != api::RequestStatus_type::SUCCESS) {
                ESP_LOGE(TAG, "API entity GET request failed");
        }
        return req.response;
}

// Parses the entity str using cJSON
// Duplicates and assings the values from the parsed cJSON, then frees the cJSON
std::optional<HAEntity> HAEntity::parse_entity_str(const std::string &entitystr)
{
        cJSON *jsonreq = cJSON_Parse(entitystr.c_str());
        if (jsonreq == nullptr) {
                return std::nullopt;
        }

        HAEntity entity;

        // Note: all the *_state_str are pointers that are freed by cJSON in the cJSON_Delete. Do not free manually.
        cJSON *json_state = cJSON_GetObjectItem(jsonreq, "state");
        if (cJSON_IsNull(json_state) || !cJSON_IsString(json_state)) {
                ESP_LOGI(TAG, "Entity has no state or is not a string.");
        } else {
                char *json_state_str = cJSON_GetStringValue(json_state);
                entity.state = std::string{json_state_str};
        }

        cJSON *json_entity_id = cJSON_GetObjectItem(jsonreq, "entity_id");
        if (cJSON_IsNull(json_entity_id) || !cJSON_IsString(json_entity_id)) {
                ESP_LOGI(TAG, "Entity has no entity_id or it is not a string.");
        } else {
                char *json_entity_id_str = cJSON_GetStringValue(json_entity_id);
                entity.entity_id = std::string{json_entity_id_str};
        }

        cJSON *json_last_changed = cJSON_GetObjectItem(jsonreq, "last_changed");
        if (cJSON_IsNull(json_last_changed) || !cJSON_IsString(json_last_changed)) {
                ESP_LOGI(TAG, "Entity has no last_changed or it is not a string.");
        } else {
                char *json_last_changed_str = cJSON_GetStringValue(json_last_changed);
                entity.last_changed = std::string{json_last_changed_str};
        }

        cJSON *json_last_updated = cJSON_GetObjectItem(jsonreq, "last_updated");
        if (cJSON_IsNull(json_last_updated) || !cJSON_IsString(json_last_updated)) {
                ESP_LOGI(TAG, "Entity has no last_updated or it is not a string.");
        } else {
                char *json_last_updated_str = cJSON_GetStringValue(json_last_updated);
                entity.last_updated = std::string{json_last_updated_str};
        }

        cJSON *json_attributes = cJSON_GetObjectItem(jsonreq, "attributes");
        if (cJSON_IsNull(json_attributes) || !cJSON_IsObject(json_attributes)) {
                ESP_LOGI(TAG, "Entity has no attributes or it is not a cJSON object.");
        } else {
                entity.attributes = cJSON_Duplicate(json_attributes, true);
        }

        cJSON_Delete(jsonreq);

        return entity;
}

std::optional<HAEntity> HAEntity::get(const std::string &entity_name)
{
        const std::string entity_str{HAEntity::get_entity_req(entity_name)};
        return HAEntity::parse_entity_str(entity_str);
}

// Get cJSON of all the states from Home Assistant
// Note: States might be really big. Mine is around 2400 char on a small install.
cJSON *get_states()
{
        auto const states_str = get_states_req();
        auto *states = parse_states_str(states_str);
        return states;
}

// Create API request to HA with entity data
void HAEntity::post() const
{
        if (entity_id.empty() || (attributes == nullptr)) {
                ESP_LOGE(TAG, "Failed to post entity. Entity or entity members are null.");
                return;
        }
        auto *json_api_req = cJSON_CreateObject();

        cJSON_AddItemToObject(json_api_req, "state", cJSON_CreateString(state.c_str()));
        cJSON_AddItemToObject(json_api_req, "attributes", cJSON_Duplicate(attributes, true));

        auto *const jsonstr = cJSON_Print(json_api_req);

        auto const path = std::string{api::STATESPATH} + "/" + entity_id;

        api::post_req(path, jsonstr, false);
        //cJSON_free(jsonstr);
        cJSON_Delete(json_api_req);
}

// ex. unit_of_measurement, friendly_name
void HAEntity::add_attribute(const char *const key, const char *const value)
{
        if (attributes == nullptr) {
                attributes = cJSON_CreateObject();
        }

        ESP_LOGV(TAG, "Adding %s:%s to attributes", key, value);
        cJSON_AddItemToObject(attributes, key, cJSON_CreateString(value));
}

void HAEntity::print() const
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
                auto *const jsonstr = cJSON_Print(attributes);
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

} // namespace state
} // namespace esphalib
