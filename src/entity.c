#include "esp_log.h"
#include "esp_http_client.h"
#include "entity.h"
#include "cJSON.h"
#include "api.h"

static const char *TAG = "Entity";

#define statespath "/api/states/"

void upload_entity_data(char* entity_name, char* friendly_entity_name, char* units, float data){
    // TODO: MAKE THIS OUT OF AN HAENTITY*
    // Create API request to home assistant with entity data  
    char api_req[256];
    snprintf(api_req, 256, "{\"state\": \"%.2f\", \"attributes\": {\"unit_of_measurement\": \"%s\", \"friendly_name\": \"%s\"}}", data, units, friendly_entity_name);

    char path[sizeof(statespath)+strlen(entity_name)+1];
    snprintf(path, sizeof(statespath)+strlen(entity_name)+1, "%s%s", statespath, entity_name);
    
    post_req(path, api_req);
}

// ex. unit_of_measurement, friendly_name
void add_entity_attribute(char* key, char* value, HAEntity* entity)
{
    if (!entity){
        ESP_LOGE(TAG, "BAD ENTITY");
        return;
    }
    
    if(!entity->attributes)
    {
        ESP_LOGE(TAG, "Created array, YAY!");
        entity->attributes = cJSON_CreateObject();
    }
    
    ESP_LOGE(TAG, "Adding item to array");
    cJSON_AddItemToObject(entity->attributes, key, cJSON_CreateString(value));
}

static char* get_entity_req(char* entity_name){
    char path[256+sizeof(statespath)];
    snprintf(path, 256+sizeof(statespath), "%s%s", statespath, entity_name);
    char* req = get_req(path);
    
    if(!req) {
        ESP_LOGE(TAG, "API entity GET request failed");
        return NULL;
    }

    return req;
}

// Parses the entity str using cJSON
// Duplicates and assings the values from the parsed cJSOn, then frees the cJSON
// Returned HAEntity must be manually freed with HAEntity_destroy()
static HAEntity* parse_entity_str(char* entitystr)
{
    if(!entitystr)
        return NULL;

    HAEntity* entity = malloc(sizeof(HAEntity));
    if(!entity) {
        ESP_LOGE(TAG, "Failed to malloc HAEntity.");
        return NULL;
    }

    cJSON* jsonreq = cJSON_Parse(entitystr);
    free(entitystr);

    cJSON* state = cJSON_GetObjectItem(jsonreq, "state");
    if(cJSON_IsNull(state) || !cJSON_IsString(state)) {
        ESP_LOGI(TAG, "Entity has no state or is not a string.");
        entity->state = NULL;
    } else {
        entity->state = strdup(cJSON_GetStringValue(state));
    }

    cJSON* entity_id = cJSON_GetObjectItem(jsonreq, "entity_id");
    if(cJSON_IsNull(entity_id) || !cJSON_IsString(entity_id)){
        ESP_LOGI(TAG, "Entity has no entity_id or it is not a string.");
        strcpy(entity->entity_id, "");
    } else {
        // Safe string copy
        strncpy(entity->entity_id, cJSON_GetStringValue(entity_id), sizeof(entity->entity_id));
        entity->entity_id[sizeof(entity->entity_id) - 1] = '\0';
    }

    cJSON* last_changed = cJSON_GetObjectItem(jsonreq, "last_changed");
    if(cJSON_IsNull(last_changed) || !cJSON_IsString(last_changed)){
        ESP_LOGI(TAG, "Entity has no last_changed or it is not a string.");
        strcpy(entity->last_changed, "");
    } else {
        strncpy(entity->last_changed, cJSON_GetStringValue(last_changed), sizeof(entity->last_changed));
        entity->last_changed[sizeof(entity->last_changed) - 1] = '\0';
    }

    cJSON* last_updated = cJSON_GetObjectItem(jsonreq, "last_updated");
    if(cJSON_IsNull(last_updated) || !cJSON_IsString(last_updated)){
        ESP_LOGI(TAG, "Entity has no last_updated or it is not a string.");
        strcpy(entity->last_updated, "");
    } else {
        strncpy(entity->last_updated, cJSON_GetStringValue(last_updated), sizeof(entity->last_updated));
        entity->last_updated[sizeof(entity->last_updated) - 1] = '\0';
    }

    cJSON* attributes = cJSON_GetObjectItem(jsonreq, "attributes");
    if(cJSON_IsNull(attributes) || !cJSON_IsObject(attributes)) {
        ESP_LOGI(TAG, "Entity has no attributes or it is not a cJSON object.");
        entity->attributes = NULL;
    } else {
        entity->attributes = cJSON_Duplicate(attributes, true);
    }


    cJSON_Delete(jsonreq);

    return entity;
}

HAEntity* get_entity(char* entity_name)
{
    char* entitystr = get_entity_req(entity_name);
    HAEntity* entity = parse_entity_str(entitystr);
    if(!entity) {
        ESP_LOGE(TAG, "Failed to get HAEntity");
        return NULL;
    }

    return entity;
}

// Frees HAEntity
void HAEntity_destroy(HAEntity* item)
{
    free(item->state);
    cJSON_Delete(item->attributes);
    free(item);
}

// Print HAEntity
void print_HAEntity(HAEntity* item)
{
    ESP_LOGV(TAG, "Printing HAEntity");

    if (!item) {
        ESP_LOGI(TAG, "Cannot print HAEntity: Entity is NULL.");
        return;
    }

    ESP_LOGI(TAG, "entity_id: %s", item->entity_id);

    if (item->state) {
        ESP_LOGI(TAG, "state: %s", item->state);
    } else {
        ESP_LOGI(TAG, "no state");
    }

    if (cJSON_IsNull(item->attributes) || !cJSON_IsObject(item->attributes)) {
        ESP_LOGI(TAG, "no attributes");
    } else {
        char* jsonstr = cJSON_Print(item->attributes);
        ESP_LOGE(TAG, "Attributes - %s", jsonstr);
        free(jsonstr);
    }

    ESP_LOGI(TAG, "last_changed: %s", item->last_changed);
    ESP_LOGI(TAG, "last_updated: %s", item->last_updated);
}