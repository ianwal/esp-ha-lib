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

#include "secrets.h"
#include "sensor.h"
#include "wifi.h"

#include <unity.h>

static const char *TAG = "TESTING";

// Attemps to upload and get back a float for a test sensor
void test_sensor_uploadreceive(void){
    wifi_init_sta();

    TEST_ASSERT_MESSAGE(is_wifi_connected(), "WiFi is not connected");
    
    // sensor name cannot contain special characters other than _ it appears
    char* test_sensor_name = "esphalibtest";
    char* test_friendly_sensor_name = "esp ha lib test";
    char* test_units = "Test Units";
    float test_data = (esp_random() % 100);
     
    upload_sensor_data(test_sensor_name, test_friendly_sensor_name, test_units, test_data);
    
    char* req = get_sensor_req(test_sensor_name);
    TEST_ASSERT_NOT_NULL_MESSAGE(req, "GET request failed");

    cJSON *jsonreq = cJSON_Parse(req);
    TEST_ASSERT_NOT_NULL_MESSAGE(jsonreq, "Failed to convert JSON to cJSON");
    cJSON* state = cJSON_GetObjectItem(jsonreq, "state");
    TEST_ASSERT_NOT_NULL_MESSAGE(state, "Could not find sensor state from GET request");
   
    const char* string = state->valuestring;
    TEST_ASSERT_NOT_NULL_MESSAGE(string, "Could not get valuestring from cJSON state");
    
    float fstate = strtof(string, NULL);

    ESP_LOGI(TAG, "Uploaded: %f, Received %f", test_data, fstate); 

    float epsilon = 1e-9;
    TEST_ASSERT_FLOAT_WITHIN(epsilon, test_data, fstate);

    cJSON_Delete(jsonreq); 
    free(req);
}


int runUnityTests(void) {
    UNITY_BEGIN();
    RUN_TEST(test_sensor_uploadreceive);
    return UNITY_END();
}

void app_main(void) {
    runUnityTests();
}
