#include <assert.h>
#include <math.h>
#include "cJSON.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "nvs_flash.h"
#include "esp_http_client.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "driver/gpio.h"

#include "esp_ha_lib.h"
#include "wifi.h"
#include "secrets.h"

#include <unity.h>

static const char *TAG = "TESTING";

// Attemps to upload and get back a float for a test entity
void test_entity_uploadreceive(void){
    // entity name cannot contain special characters other than _ or . it appears
    char* entity_id = "sensor.esphalibtest";
    char* friendly_entity_name = "esp ha lib test";
    char* unit_of_measurement = "Test Units";
    float state = (esp_random() % 100);
    
    HAEntity* entity = HAEntity_create();
    entity->state = malloc(8);
    snprintf(entity->state, 8, "%.2f", state);
    strcpy(entity->entity_id, entity_id);
    add_entity_attribute("friendly_name", friendly_entity_name, entity);
    add_entity_attribute("unit_of_measurement", unit_of_measurement, entity);

    post_entity(entity);

    HAEntity* newEntity = get_entity(entity_id);
    float fstate = strtof(newEntity->state, NULL);   
    
    ESP_LOGI(TAG, "Uploaded: %f, Received %f", state, fstate); 
    
    // HA only stores floats to 2 decimal places it seems
    float epsilon = 1e-2;

    HAEntity_destroy(entity);
    HAEntity_destroy(newEntity);
    TEST_ASSERT_FLOAT_WITHIN(epsilon, state, fstate);
}

void test_api_running(void) {
    TEST_ASSERT_MESSAGE(is_wifi_connected(), "WiFi is not connected");
    
    TEST_ASSERT_MESSAGE(get_api_status(), "API is not accessible and/or not running.");
}

void test_HAEntity_print(void)
{
    HAEntity entity = {.entity_id="print_test1", .last_updated="2021-02-12T10:41:28.422190+00:00", .last_changed="2023-05-17T10:41:28.855123+00:00"};
    entity.attributes = cJSON_Parse("{\"unit_of_measurement\":\"Test Units\",\"friendly_name\":\"esp ha lib test\"}");
    entity.state = strdup("on!");
    HAEntity_print(&entity);
    
    cJSON_Delete(entity.attributes);
    free(entity.state);
}

// Tests printing a real HAEntity from Home Assistant
// Needs entity "sun.sun" which I think is built in or a substitute on a live home assistant to connect to
void test_print_real_HAEntity(void)
{
    HAEntity* entity = HAEntity_create();
    entity = get_entity("sun.sun");
    HAEntity_print(entity);
    HAEntity_destroy(entity);
}

// Tests adding a new entity attribute to an HAEntity
void test_add_entity_attribute(void)
{
    HAEntity* entity = HAEntity_create();
    char* entity_name = "sensor.randomsensortest";
    char* friendly_entity_name = "esp ha lib sensor test";
    char* unit_of_measurement = "test units";
    add_entity_attribute("entity_name", entity_name, entity);
    add_entity_attribute("friendly_entity_name", friendly_entity_name, entity);
    add_entity_attribute("unit_of_measurement", unit_of_measurement, entity);
    
    TEST_ASSERT_EQUAL_STRING(entity_name, cJSON_GetStringValue(cJSON_GetObjectItem(entity->attributes, "entity_name")));
    TEST_ASSERT_EQUAL_STRING(friendly_entity_name, cJSON_GetStringValue(cJSON_GetObjectItem(entity->attributes, "friendly_entity_name")));
    TEST_ASSERT_EQUAL_STRING(unit_of_measurement, cJSON_GetStringValue(cJSON_GetObjectItem(entity->attributes, "unit_of_measurement")));
    
    HAEntity_destroy(entity);
}

int runUnityTests(void) {
    UNITY_BEGIN();
    RUN_TEST(test_add_entity_attribute);
    RUN_TEST(test_HAEntity_print);
    RUN_TEST(test_api_running);
    RUN_TEST(test_entity_uploadreceive);
    RUN_TEST(test_print_real_HAEntity);
    return UNITY_END();
}

void app_main(void) {
    wifi_init_sta();

    set_ha_url(HA_URL);
    set_long_lived_access_token(LONG_LIVED_ACCESS_TOKEN);

    runUnityTests();
}