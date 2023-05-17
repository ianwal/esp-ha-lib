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

#include "entity.h"
#include "wifi.h"
#include "secrets.h"
#include "api.h"

#include <unity.h>

static const char *TAG = "TESTING";

// Attemps to upload and get back a float for a test entity
void test_entity_uploadreceive(void){
    TEST_ASSERT_MESSAGE(is_wifi_connected(), "WiFi is not connected");

    for(int i = 0; i < 2; i++){
    // entity name cannot contain special characters other than _ it appears
    char* test_entity_name = "sensor.esphalibtest";
    char* test_friendly_entity_name = "esp ha lib test";
    char* test_units = "Test Units";
    float test_data = (esp_random() % 100);
    
    upload_entity_data(test_entity_name, test_friendly_entity_name, test_units, test_data);

    HAEntity* entity = malloc(sizeof(HAEntity));
    entity = get_entity(test_entity_name);
    float fstate = strtof(entity->state, NULL);   
    
    ESP_LOGI(TAG, "Uploaded: %f, Received %f", test_data, fstate); 
    
    // HA only stores floats to 2 decimal places it seems
    float epsilon = 1e-2;

    HAEntity_destroy(entity);
    TEST_ASSERT_FLOAT_WITHIN(epsilon, test_data, fstate);
    }
}

void test_api_running(void) {
    TEST_ASSERT_MESSAGE(is_wifi_connected(), "WiFi is not connected");
    
    TEST_ASSERT_MESSAGE(get_api_status(), "API is running and accessible.");
}

void test_print_HAEntity(void)
{
    HAEntity entity = {.entity_id="print_test1", .last_updated="2021-02-12T10:41:28.422190+00:00", .last_changed="2023-05-17T10:41:28.855123+00:00"};
    entity.attributes = cJSON_Parse("{\"unit_of_measurement\":\"Test Units\",\"friendly_name\":\"esp ha lib test\"}");
    entity.state = strdup("on!");
    print_HAEntity(&entity);
    
    cJSON_Delete(entity.attributes);
    free(entity.state);
}

// Tests printing a real HAEntity from Home Assistant
// Needs entity "sensor.esphalibtest" or a substitute on a live home assistant to connect to
void test_print_real_HAEntity(void)
{
    HAEntity* entity = malloc(sizeof(HAEntity));
    entity = get_entity("sensor.esphalibtest");
    print_HAEntity(entity);
    HAEntity_destroy(entity);
}

// Tests adding a new entity attribute to an HAEntity
void test_add_entity_attribute(void)
{
    HAEntity* entity = malloc(sizeof(HAEntity));
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
    RUN_TEST(test_print_HAEntity);
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