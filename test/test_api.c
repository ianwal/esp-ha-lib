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

#include <unity.h>

static const char *TAG = "TESTING";

// Attemps to upload and get back a float for a test entity
void test_entity_uploadreceive(void){
    TEST_ASSERT_MESSAGE(is_wifi_connected(), "WiFi is not connected");

    for(int i = 0; i < 5; i++){
    // entity name cannot contain special characters other than _ it appears
    char* test_entity_name = "sensor.esphalibtest";
    char* test_friendly_entity_name = "esp ha lib test";
    char* test_units = "Test Units";
    float test_data = (esp_random() % 100);
    
    set_ha_url(HA_URL);
    set_long_lived_access_token(LONG_LIVED_ACCESS_TOKEN);
    
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

int runUnityTests(void) {
    UNITY_BEGIN();
    RUN_TEST(test_entity_uploadreceive);
    return UNITY_END();
}

void app_main(void) {
    wifi_init_sta();

    assert(is_wifi_connected()); 
    runUnityTests();
}
