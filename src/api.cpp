#include "api.hpp"
#include "cJSON.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include <cstdlib>
#include <string>

namespace esphalib
{

namespace api
{

namespace
{
constexpr const char *TAG{"API"};
}

// Set with set_ha_url()
std::string ha_url;
// Set with set_long_lived_access_token()
std::string long_lived_access_token;

// Call this before doing anything with the library
// Sets ha url e.g. "http://IP_ADDRESS:8123"
void set_ha_url(const char *new_url)
{
        if (!new_url || strncmp(new_url, "", 1) == 0) {
                ESP_LOGE(TAG, "Failed to set ha_url. new_url is NULL.");
                return;
        }

        ha_url = std::string{new_url};

        ESP_LOGV(TAG, "Set new ha_url to: %s", ha_url.c_str());
}

// Call this before doing anything with the library
void set_long_lived_access_token(const char *new_long_lived_access_token)
{
        if (!new_long_lived_access_token || strncmp(new_long_lived_access_token, "", 1) == 0) {
                ESP_LOGE(TAG, "Failed to set long_lived_access_token. new_long_lived_access_token is NULL.");
                return;
        }

        long_lived_access_token = std::string{new_long_lived_access_token};

        ESP_LOGV(TAG, "Set new LLAT to: %s", long_lived_access_token.c_str());
}

std::string post_req(const char *path, const char *data, const bool return_response)
{
        if (ha_url.empty() || long_lived_access_token.empty()) {
                ESP_LOGE(TAG, "Failed to upload data: ha_url or access token not set yet");
                return std::string{};
        }

        // Create API URL. Will look something like http://HA_URL/api/states/entity.entity_NAME
        const std::string api_URL{ha_url + path};

        constexpr const char *bearer = "Bearer ";
        const std::string auth_data{bearer + long_lived_access_token};

// Attempt to make API request to Home Assistant
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
        esp_http_client_config_t home_assistant_config = {
            .url = api_URL.c_str(),
            .method = HTTP_METHOD_POST,
            .timeout_ms = 10000,
            .disable_auto_redirect = false,
            .is_async = false,
            .skip_cert_common_name_check = true,
        };

        ESP_LOGV(TAG, "Attempting connection to %s", api_URL.c_str());

        esp_http_client_handle_t client = esp_http_client_init(&home_assistant_config);
        esp_http_client_set_header(client, "Authorization", auth_data.c_str());
        esp_http_client_set_header(client, "Content-Type", "application/json");

        if (data) {
                esp_http_client_set_post_field(client, data, strlen(data));
        }

        //  POST data
        esp_err_t err = esp_http_client_perform(client);
        if (err == ESP_OK) {
                if (data) {
                        ESP_LOGV(TAG, "Sent %s to %s", data, api_URL.c_str());
                }
        } else {
                ESP_LOGE(TAG, "Could not send upload entity data.");
        }

        char *response_buffer = nullptr;
        std::string ret_str;
        // POST Response handling for when return_response=true
        if (return_response) {
                if (err == ESP_OK) { // only get response if the above post was OK
                        // Receive POST response
                        err = esp_http_client_open(client, 0);
                        if (err == ESP_OK) {
                                esp_http_client_fetch_headers(client);
                                if (return_response) {
                                        int64_t content_length = esp_http_client_get_content_length(client);
                                        response_buffer = (char *)malloc(content_length + 1);
                                        if (response_buffer) {
                                                esp_err_t post_response = esp_http_client_read_response(
                                                    client, response_buffer, content_length);
                                                if (post_response != ESP_FAIL) {
                                                        response_buffer[content_length] =
                                                            '\0'; // ensure response is null-terminated
                                                        ret_str = std::string{response_buffer};
                                                        ESP_LOGV(TAG, "Content length %lld Read: %s", content_length,
                                                                 ret_str.c_str());
                                                } else {
                                                        ESP_LOGE(TAG, "POST Response failed %s",
                                                                 esp_err_to_name(post_response));
                                                }
                                                free(response_buffer);
                                        }
                                }
                        } else {
                                ESP_LOGE(TAG, "POST req failed. Could not send upload entity data.");
                        }
                }
        }

        esp_http_client_close(client);
        esp_http_client_cleanup(client);

        return ret_str;
}

std::string get_req(const char *path)
{
        if (ha_url.empty() || long_lived_access_token.empty()) {
                ESP_LOGE(TAG, "Failed to GET: ha_url or access token not set yet");
                return std::string{};
        }

        char *local_response_buffer = nullptr;

        // Create API URL. Will look something like http://HA_URL/api/states/entity.entity_NAME
        const std::string api_URL{ha_url + path};

        constexpr const char *bearer = "Bearer ";
        const std::string auth_data{bearer + long_lived_access_token};

#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
        esp_http_client_config_t config = {
            .url = api_URL.c_str(),
            .method = HTTP_METHOD_GET,
            .timeout_ms = 10000,
            .disable_auto_redirect = false,
            .user_data = local_response_buffer,
            .is_async = false,
            .skip_cert_common_name_check = true,
        };

        esp_http_client_handle_t client = esp_http_client_init(&config);
        esp_http_client_set_header(client, "Authorization", auth_data.c_str());
        esp_http_client_set_header(client, "Content-Type", "application/json");

        std::string ret_str;
        [[maybe_unused]] bool failed = false;
        // Attempt to make API request to Home Assistant
        const esp_err_t err = esp_http_client_open(client, 0);
        if (err == ESP_OK) {
                const esp_err_t fetcherr = esp_http_client_fetch_headers(client);
                if (fetcherr != ESP_FAIL) {
                        int64_t content_length = esp_http_client_get_content_length(client);
                        local_response_buffer = (char *)malloc(content_length + 1);
                        // ESP_LOGI(TAG, "Buffer size %lld", esp_http_client_get_content_length(client));
                        if (!local_response_buffer) {
                                ESP_LOGE(TAG, "Buffer malloc failed");
                                failed = true;
                        } else {
                                esp_http_client_read_response(client, local_response_buffer, content_length);
                                local_response_buffer[content_length] = '\0'; // ensure response is null-terminated
                                ret_str = std::string{local_response_buffer};
                                free(local_response_buffer);
                                ESP_LOGV(TAG, "Read %s, \nSize: %lld", ret_str.c_str(), content_length);
                        }
                } else {
                        ESP_LOGE(TAG, "Failed to fetch request: %s", esp_err_to_name(fetcherr));
                        failed = true;
                }
        } else {
                ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
                failed = true;
        }

        esp_http_client_close(client);
        esp_http_client_cleanup(client);

        return ret_str;
}

bool get_api_status(void)
{
        const std::string req = get_req("/api/");
        if (req.empty()) {
                return false;
        }

        cJSON *jsonreq = cJSON_Parse(req.c_str());

        if (cJSON_IsNull(jsonreq)) {
                cJSON_Delete(jsonreq);
                return false;
        }

        cJSON *message = cJSON_GetObjectItem(jsonreq, "message");
        if (cJSON_IsNull(message) || !cJSON_IsString(message)) {
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

} // namespace api
} // namespace esphalib
