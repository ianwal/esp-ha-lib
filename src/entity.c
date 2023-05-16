#include "esp_log.h"
#include "esp_http_client.h"
#include "entity.h"
#include "cJSON.h"
#include "api.h"

static const char *TAG = "Upload";

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

// TODO: ADD THE REST OF THE STRUCT OTHER THAN STATE
// Parses the entity str using cJSON
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
        entity->state = malloc(strlen(state->valuestring)+1);
        strcpy(entity->state, state->valuestring);
    }

    cJSON* entity_id = cJSON_GetObjectItem(jsonreq, "entity_id");
    if(cJSON_IsNull(entity_id) || !cJSON_IsString(entity_id)){
        ESP_LOGI(TAG, "Entity has no entity_id or it is not a string.");
        strcpy(entity->entity_id, "");
    } else {
        strcpy(entity->entity_id, entity_id->valuestring);
    }
    
    cJSON* last_changed = cJSON_GetObjectItem(jsonreq, "last_changed");
    if(cJSON_IsNull(last_changed) || !cJSON_IsString(last_changed)){
        ESP_LOGI(TAG, "Entity has no last_changed or it is not a string.");
        strcpy(entity->last_changed, "");
    } else {
        strcpy(entity->last_changed, last_changed->valuestring);
    }
    
    cJSON* last_updated = cJSON_GetObjectItem(jsonreq, "last_updated");
    if(cJSON_IsNull(last_updated) || !cJSON_IsString(last_updated)){
        ESP_LOGI(TAG, "Entity has no last_updated or it is not a string.");
        strcpy(entity->last_updated, "");
    } else {
        strcpy(entity->last_updated, last_updated->valuestring);
    }

    cJSON* attributes = cJSON_GetObjectItem(jsonreq, "attributes");
    if(cJSON_IsNull(attributes) || !cJSON_IsObject(attributes)) {
        ESP_LOGI(TAG, "Entity has no attributes or it is not a cJSON object.");
        entity->attributes = NULL;
    } else {
        entity->attributes = cJSON_Duplicate(attributes, true);
        /*
        for(int i =0; i < cJSON_GetArraySize(attributes); i++) {
            ESP_LOGI(TAG, "attribute: %s", cJSON_GetArrayItem(attributes, i)->string);
        }
        */
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
void HAEntity_destroy(HAEntity* item){
    free(item->state);
    cJSON_Delete(item->attributes);
    free(item);
}