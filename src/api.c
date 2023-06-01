#include "esp_log.h"
#include "esp_http_client.h"
#include "cJSON.h"
#include "api.h"
#include <stdlib.h>
#include <stdbool.h>

static const char* TAG = "API";

// Set with set_ha_url()
char* ha_url = NULL;
// Set with set_long_lived_access_token
char* long_lived_access_token = NULL;

// Call this before doing anything with the library
// Sets ha url e.g. "http://IP_ADDRESS:8123"
void set_ha_url(const char* new_url)
{
    if (!new_url || strcmp(new_url, "")  == 0) {
        ESP_LOGE(TAG, "Failed to set ha_url. new_url is NULL.");
        return;
    }
    
    free(ha_url);

    size_t len = strlen(new_url);
    ha_url = strndup(new_url, len);

    if(!ha_url) {
        ESP_LOGE(TAG, "Failed to set ha_url. strndup failed.");
        return;
    }

    // Check for leading slash and remove it if present
    if (ha_url[len-1] == '/')
        ha_url[len-1] = '\0';

    ESP_LOGV(TAG, "Set new ha_url to: %s", ha_url);
}

// Call this before doing anything with the library
void set_long_lived_access_token(const char* new_long_lived_access_token)
{
    if (!new_long_lived_access_token || strcmp(new_long_lived_access_token, "")  == 0) {
        ESP_LOGE(TAG, "Failed to set long_lived_access_token. new_long_lived_access_token is NULL.");
        return;
    }
    
    free(long_lived_access_token);
    
    long_lived_access_token = strdup(new_long_lived_access_token);
    
    if(!long_lived_access_token) {
        ESP_LOGE(TAG, "Failed to set long_lived_access_token. strndup failed.");
        return;
    }

    ESP_LOGV(TAG, "Set new LLAT to: %s", long_lived_access_token);
}

char* post_req(char* path, char* data, bool return_response) {
    if(!ha_url || !long_lived_access_token){
        ESP_LOGE(TAG, "Failed to upload data: ha_url or access token not set yet");
        return NULL;
    }

    // Create API URL. Will look something like http://HA_URL/api/states/entity.entity_NAME
    char api_URL[256];
    snprintf(api_URL, 256, "%s%s", ha_url, path);

    // Long-Lived Access Token for Home Assistant API. This is generated by an administrator and is valid for 10 years. 
    const char* bearer = "Bearer ";
    char* auth_data = malloc(strlen(bearer) + strlen(long_lived_access_token)+1);
    if(!auth_data) {
        ESP_LOGE(TAG, "auth_data malloc failed.");
        return NULL;
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

    char* response_buffer = NULL;

    ESP_LOGV(TAG, "Attempting connection to %s", api_URL);

    esp_http_client_handle_t client = esp_http_client_init(&home_assistant_config);
    esp_http_client_set_header(client, "Authorization", auth_data);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    
    if (data)
        esp_http_client_set_post_field(client, data, strlen(data));
    
    //esp_err_t err = esp_http_client_perform(client);
    // POST data
    esp_err_t err = esp_http_client_perform(client);
    if(err == ESP_OK) {
        if(data)
            ESP_LOGV(TAG, "Sent %s to %s", data, api_URL);
    } else {
        ESP_LOGE(TAG, "Could not send upload entity data.");
    }

    // POST Response handling for when return_response=true
    if (return_response) {
        if(err == ESP_OK) { // only get response if the above post was OK
            // Receive POST response
            err = esp_http_client_open(client, 0);
            if(err == ESP_OK){
                esp_http_client_fetch_headers(client);
                if (return_response) {
                    int64_t content_length = esp_http_client_get_content_length(client);
                    response_buffer = malloc(content_length+1);
                    if (response_buffer) {
                        esp_err_t post_response = esp_http_client_read_response(client, response_buffer, content_length);
                        if (post_response != ESP_FAIL) {
                            response_buffer[content_length] = '\0'; // ensure response is null-terminated
                            ESP_LOGV(TAG, "Content length %lld Read: %s", content_length, response_buffer);
                        } else {
                            ESP_LOGE(TAG, "POST Response failed %s", esp_err_to_name(post_response));
                            free(response_buffer);
                            response_buffer = NULL;
                        }
                    }
                }
            } else {
                ESP_LOGE(TAG, "Could not send upload entity data.");
            }
        }
    }

    free(auth_data);
    esp_http_client_close(client);
    esp_http_client_cleanup(client);

    if (response_buffer) {
        return response_buffer;
    } else {
        return NULL;
    }
}

char* get_req(char* path)
{
    if (!ha_url || !long_lived_access_token) {
        ESP_LOGE(TAG, "Failed to GET: ha_url or access token not set yet");
        return NULL;
    }
    
    
    char* local_response_buffer = NULL;
    

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
            int64_t content_length = esp_http_client_get_content_length(client);
            local_response_buffer = malloc(content_length+1);
            //ESP_LOGI(TAG, "Buffer size %lld", esp_http_client_get_content_length(client));
            if (!local_response_buffer) {
                ESP_LOGE(TAG, "Buffer malloc failed");
                failed = true;
            } else {
                esp_http_client_read_response(client, local_response_buffer, content_length);
                local_response_buffer[content_length] = '\0'; // ensure response is null-terminated
                ESP_LOGV(TAG, "Read %s, \nSize: %lld", local_response_buffer, content_length);
            }
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
        local_response_buffer = NULL;
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
    if (cJSON_IsNull(message) || !cJSON_IsString(message)){
        cJSON_Delete(jsonreq);
        return false;
    }

    ESP_LOGV(TAG, "Status: %s", cJSON_GetStringValue(message));

    if (strncmp(cJSON_GetStringValue(message), "API running.", sizeof("API running.")) == 0) {
        cJSON_Delete(jsonreq);
        return true;
    }

    cJSON_Delete(jsonreq);
    return false;
}