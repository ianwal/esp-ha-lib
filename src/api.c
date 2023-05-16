#include "esp_log.h"
#include "esp_http_client.h"
#include "cJSON.h"
#include "api.h"

#define MAX_HTTP_OUTPUT_BUFFER 1024

static const char* TAG = "API";

// Set with set_ha_url()
char* ha_url = NULL;
// Set with set_long_lived_access_token
char* long_lived_access_token = NULL;

void set_ha_url(const char* new_url)
{
    free(ha_url);
    ha_url = (char*) malloc(strlen(new_url)+1);
    strcpy(ha_url, new_url);
    ESP_LOGI(TAG, "Set new ha_url to: %s", ha_url);
}

void set_long_lived_access_token(const char* new_long_lived_access_token)
{
    free(long_lived_access_token);
    long_lived_access_token = malloc(strlen(new_long_lived_access_token)+1);
    strcpy(long_lived_access_token, new_long_lived_access_token);
    ESP_LOGI(TAG, "Set new LLAT to: %s", long_lived_access_token);
}

void post_req(char* path, char* data) {
    if(!ha_url || !long_lived_access_token){
        ESP_LOGE(TAG, "Failed to upload data: ha_url or access token not set yet");
        return;
    }

    // Create API URL. Will look something like http://HA_URL/api/states/entity.entity_NAME
    char api_URL[256];
    snprintf(api_URL, 256, "%s%s", ha_url, path);

    // Long-Lived Access Token for Home Assistant API. This is generated by an administrator and is valid for 10 years. 
    const char* bearer = "Bearer ";
    char* auth_data = malloc(strlen(bearer) + strlen(long_lived_access_token)+1);
    if(!auth_data) {
        ESP_LOGE(TAG, "auth_data malloc failed.");
        return;
    }
    memcpy(auth_data, bearer, strlen(bearer));
    memcpy(auth_data + strlen(bearer), long_lived_access_token, strlen(long_lived_access_token)+1);


    // Attempt to make API request to Home Assistant
    esp_http_client_config_t home_assistant_config = {
        .url = api_URL,
        .method = HTTP_METHOD_POST,
        .timeout_ms = 5000,
        .is_async = false,
        .skip_cert_common_name_check = true,
        .disable_auto_redirect = false,
    };
    
    ESP_LOGI(TAG, "Attempting connection to %s", api_URL);

    esp_http_client_handle_t client = esp_http_client_init(&home_assistant_config);
    esp_http_client_set_header(client, "Authorization", auth_data);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, data, strlen(data));

    if(esp_http_client_perform(client) == ESP_OK){
        ESP_LOGI(TAG, "Sent %s to %s", data, api_URL);
    } else {
        ESP_LOGE(TAG, "Could not send upload entity data.");
    }

    free(auth_data);
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
}

char* get_req(char* path)
{
    if (!ha_url || !long_lived_access_token) {
        ESP_LOGE(TAG, "Failed to GET: ha_url or access token not set yet");
        return NULL;
    }
    
    
    char* local_response_buffer = malloc(MAX_HTTP_OUTPUT_BUFFER);

    if (!local_response_buffer) {
        ESP_LOGE(TAG, "Buffer malloc failed");
        return NULL;
    }

    // Create API URL. Will look something like http://HA_URL/api/states/sensor.entity_name
    char api_URL[128];
    snprintf(api_URL, 128, "%s%s", ha_url, path);
    
    // Long-Lived Access Token for Home Assistant API. This is generated by an administrator and is valid for 10 years. 
    const char* bearer = "Bearer ";
    char* auth_data = malloc(strlen(bearer) + strlen(long_lived_access_token)+1);
    if (!auth_data) {
        ESP_LOGE(TAG, "auth_data malloc failed.");
        return NULL;
    }
    memcpy(auth_data, bearer, strlen(bearer));
    memcpy(auth_data + strlen(bearer), long_lived_access_token, strlen(long_lived_access_token)+1);

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
    
    bool failed = false;
    // Attempt to make API request to Home Assistant
    esp_err_t err = esp_http_client_open(client, 0);
    if (err == ESP_OK) {
        esp_err_t fetcherr = esp_http_client_fetch_headers(client);
        if(fetcherr != ESP_FAIL){ 
            esp_http_client_read_response(client, local_response_buffer, MAX_HTTP_OUTPUT_BUFFER);
            ESP_LOGI(TAG, "Read %s, \nSize: %lld", local_response_buffer, esp_http_client_get_content_length(client));
        } else {
            ESP_LOGE(TAG, "Failed to fetch request: %s", esp_err_to_name(fetcherr));
            failed = true;
        }
    } else {
        ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
        failed = true;
    }

    free(auth_data);
    esp_http_client_close(client);
    esp_http_client_cleanup(client);

    if (failed) {
        free(local_response_buffer);
        return NULL;
    }
    
    return local_response_buffer;
}

bool get_api_status(void)
{
    char* req = get_req("/api/");
    if (!req) {
        return false;
    }

    cJSON* jsonreq = cJSON_Parse(req);
    free(req);
    
    if(cJSON_IsNull(jsonreq)) {
        cJSON_Delete(jsonreq);
        return false;
    }

    cJSON* message = cJSON_GetObjectItem(jsonreq, "message");
    if (cJSON_IsNull(message)){
        cJSON_Delete(jsonreq);
        return false;
    }
    ESP_LOGI(TAG, "%s", message->valuestring);

    if (strcmp(message->valuestring, "API running.") == 0) {
        cJSON_Delete(jsonreq);
        return true;
    }

    return false;
}