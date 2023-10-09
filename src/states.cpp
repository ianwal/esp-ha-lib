extern "C" {
#include "cJSON.h"
#include "esp_log.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
}
#include "api.hpp"

#include "states.hpp"

static const char *TAG = "States";

// Create API request to home assistant with entity data
void post_entity(HAEntity *entity)
{
        if (!entity || !entity->entity_id) {
                ESP_LOGE(TAG, "Failed to post entity. Entity or entity members are null.");
                return;
        }
        cJSON *json_api_req = cJSON_CreateObject();

        cJSON_AddItemToObject(json_api_req, "state", cJSON_CreateString(entity->state.c_str()));
        cJSON_AddItemToObject(json_api_req, "attributes", cJSON_Duplicate(entity->attributes, true));

        char *jsonstr = cJSON_Print(json_api_req);
        ESP_LOGI(TAG, "JSON Str - %s", jsonstr);

        char path[sizeof(STATESPATH) + strlen(entity->entity_id) + 1 + 1]; // +1 for the / in the path
        snprintf(path, sizeof(STATESPATH) + strlen(entity->entity_id) + 1 + 1, "%s/%s", STATESPATH, entity->entity_id);

        ESP_LOGI(TAG, "Path - %s", path);

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

static char *get_entity_req(const char *entity_name)
{
        char path[256 + sizeof(STATESPATH) + 1]; // +1 for the / in the path
        snprintf(path, 256 + sizeof(STATESPATH) + 1, "%s/%s", STATESPATH, entity_name);
        char *req = get_req(path);

        if (!req) {
                ESP_LOGE(TAG, "API entity GET request failed");
                return NULL;
        }

        return req;
}

// Parses the entity str using cJSON
// Duplicates and assings the values from the parsed cJSOn, then frees the cJSON
// Returned HAEntity must be manually freed with HAEntity_destroy()
static HAEntity *parse_entity_str(char *entitystr)
{
        if (!entitystr)
                return NULL;

        HAEntity *entity = HAEntity_create();
        if (!entity) {
                ESP_LOGE(TAG, "Failed to malloc HAEntity.");
                return NULL;
        }

        cJSON *jsonreq = cJSON_Parse(entitystr);
        free(entitystr);

        cJSON *state = cJSON_GetObjectItem(jsonreq, "state");
        if (cJSON_IsNull(state) || !cJSON_IsString(state)) {
                ESP_LOGI(TAG, "Entity has no state or is not a string.");
        } else {
                entity->state = std::string(cJSON_GetStringValue(state));
        }

        cJSON *entity_id = cJSON_GetObjectItem(jsonreq, "entity_id");
        if (cJSON_IsNull(entity_id) || !cJSON_IsString(entity_id)) {
                ESP_LOGI(TAG, "Entity has no entity_id or it is not a string.");
                strcpy(entity->entity_id, "");
        } else {
                // Safe string copy
                strncpy(entity->entity_id, cJSON_GetStringValue(entity_id), sizeof(entity->entity_id));
                entity->entity_id[sizeof(entity->entity_id) - 1] = '\0';
        }

        cJSON *last_changed = cJSON_GetObjectItem(jsonreq, "last_changed");
        if (cJSON_IsNull(last_changed) || !cJSON_IsString(last_changed)) {
                ESP_LOGI(TAG, "Entity has no last_changed or it is not a string.");
                strcpy(entity->last_changed, "");
        } else {
                strncpy(entity->last_changed, cJSON_GetStringValue(last_changed), sizeof(entity->last_changed));
                entity->last_changed[sizeof(entity->last_changed) - 1] = '\0';
        }

        cJSON *last_updated = cJSON_GetObjectItem(jsonreq, "last_updated");
        if (cJSON_IsNull(last_updated) || !cJSON_IsString(last_updated)) {
                ESP_LOGI(TAG, "Entity has no last_updated or it is not a string.");
                strcpy(entity->last_updated, "");
        } else {
                strncpy(entity->last_updated, cJSON_GetStringValue(last_updated), sizeof(entity->last_updated));
                entity->last_updated[sizeof(entity->last_updated) - 1] = '\0';
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

HAEntity *get_entity(const char *entity_name)
{
        char *entitystr = get_entity_req(entity_name);
        HAEntity *entity = parse_entity_str(entitystr);
        if (!entity) {
                ESP_LOGE(TAG, "Failed to get HAEntity");
                return NULL;
        }

        return entity;
}

// Create a new HAEntity
HAEntity *HAEntity_create(void)
{
        HAEntity *newEntity = new HAEntity;
        strcpy(newEntity->last_changed, "");
        strcpy(newEntity->last_updated, "");
        return newEntity;
}

// Frees HAEntity
void HAEntity_delete(HAEntity *entity)
{
        /*
            if (item == NULL) {
                    return;
            }
            if (item->attributes != NULL) {
                    cJSON_Delete(item->attributes);
            }
            free(item);
            item = NULL;
            */
        delete entity;
}

// Print HAEntity
void HAEntity_print(HAEntity *item)
{
        ESP_LOGV(TAG, "Printing HAEntity");

        if (!item) {
                ESP_LOGI(TAG, "Cannot print HAEntity: Entity is NULL.");
                return;
        }

        ESP_LOGI(TAG, "entity_id: %s", item->entity_id);

        if (item->state.size() > 0) {
                ESP_LOGI(TAG, "state: %s", item->state.c_str());
        } else {
                ESP_LOGI(TAG, "no state");
        }

        if (cJSON_IsNull(item->attributes) || !cJSON_IsObject(item->attributes)) {
                ESP_LOGI(TAG, "no attributes");
        } else {
                char *jsonstr = cJSON_Print(item->attributes);
                ESP_LOGI(TAG, "Attributes - %s", jsonstr);
                free(jsonstr);
        }

        if (item->last_changed[0]) {
                ESP_LOGI(TAG, "last_changed: %s", item->last_changed);
        } else {
                ESP_LOGI(TAG, "no last_changed");
        }

        if (item->last_updated[0]) {
                ESP_LOGI(TAG, "last_updated: %s", item->last_updated);
        } else {
                ESP_LOGI(TAG, "no last_updated");
        }
}

static char *get_states_req(void)
{
        char *req = get_req(STATESPATH);

        if (!req) {
                ESP_LOGE(TAG, "API states GET request failed");
                return NULL;
        }

        return req;
}

static cJSON *parse_states_str(char *statesstr)
{
        if (!statesstr)
                return NULL;

        cJSON *jsonreq = cJSON_Parse(statesstr);
        return jsonreq;
}

// Get cJSON of all the states from Home Assistant
// Note: States might be really big. Mine is around 2400 char on a small install.
cJSON *get_states(void)
{
        char *statesstr = get_states_req();
        cJSON *states = parse_states_str(statesstr);
        free(statesstr);
        return states;
}