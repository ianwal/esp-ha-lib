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
#include "upload.h"
#include "wifi.h"

#include <unity.h>

static const char *TAG = "TESTING";

#define MAX_HTTP_OUTPUT_BUFFER 1024

char* get_sensor_req(char* sensor_name);

void test_sensor_upload(void){
    wifi_init_sta();

    char* test_sensor_name = "esp-ha-lib-test";
    char* test_friendly_sensor_name = "Test Voltage";
    char* test_units = "Test Volts";
    float test_data = (esp_random() % 100) + ((esp_random() % 100) / 100.f);
    
    upload_sensor_data(test_sensor_name, test_friendly_sensor_name, test_units, test_data);
    
    char* req = get_sensor_req(test_sensor_name);
    
    if(!req) {
        ESP_LOGE(TAG, "\nRequest failed to return.\n"); 
        return;
    }

    cJSON *jsonreq = cJSON_Parse(req);
    cJSON* state = cJSON_GetObjectItem(jsonreq, "state");
    
    const char* string = state->valuestring;
    float fstate = strtof(string, NULL);

    ESP_LOGI(TAG, "Uploaded: %f, Received %f", test_data, fstate); 
    
    float epsilon = 1e-9;
    if(fabs(test_data - fstate) < epsilon)
        ESP_LOGI(TAG, "UPLOAD TEST PASSED"); 

    cJSON_Delete(jsonreq); 
    free(req);
    TEST_ASSERT_FLOAT_WITHIN(epsilon, test_data, fstate);

    //assert(fabs(test_data - fstate) < epsilon);
}

char* get_sensor_req(char* sensor_name){
    char* local_response_buffer = pvPortMalloc(MAX_HTTP_OUTPUT_BUFFER);

    if(!local_response_buffer) {
        ESP_LOGE(TAG, "Buffer malloc failed");
        return NULL;
    }

    // Create API URL. Will look something like http://HA_URL/api/states/sensor.SENSOR_NAME
    char api_URL[128];
    snprintf(api_URL, 128, "%s/api/states/sensor.%s", HA_URL, sensor_name);
    
    // Long-Lived Access Token for Home Assistant API. This is generated by an administrator and is valid for 10 years. 
    const char* bearer = "Bearer ";
    char* auth_data = pvPortMalloc(strlen(bearer) + sizeof(LONG_LIVED_ACCESS_TOKEN));
    if(!auth_data) {
        ESP_LOGE(TAG, "auth_data malloc failed.");
        return NULL;
    }
    memcpy(auth_data, bearer, strlen(bearer));
    memcpy(auth_data + strlen(bearer), LONG_LIVED_ACCESS_TOKEN, sizeof(LONG_LIVED_ACCESS_TOKEN));

    esp_http_client_config_t config = {
        .url = api_URL,
        .method = HTTP_METHOD_GET,
        .timeout_ms = 5000,
        .is_async = false,
        .skip_cert_common_name_check = true,
        .disable_auto_redirect = false,
        .user_data = local_response_buffer,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client, "Authorization", auth_data);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    
    // Attempt to make API request to Home Assistant
    esp_err_t err = esp_http_client_open(client, 0);
    if (err == ESP_OK) {
        esp_err_t fetcherr = esp_http_client_fetch_headers(client);
        if(fetcherr != ESP_FAIL){ 
            esp_http_client_read_response(client, local_response_buffer, MAX_HTTP_OUTPUT_BUFFER);
            //ESP_LOGI(TAG, "Read %s, \nSize: %d", local_response_buffer, esp_http_client_get_content_length(client));
        } else {
            ESP_LOGE(TAG, "Failed to fetch request: %s", esp_err_to_name(fetcherr));
        }
    } else {
        ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
    }

    free(auth_data);
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
    
    return local_response_buffer;
}

int runUnityTests(void) {
    UNITY_BEGIN();
    RUN_TEST(test_sensor_upload);
    return UNITY_END();
}

void app_main(void) {
    runUnityTests();
}
