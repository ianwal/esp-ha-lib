#include "api.hpp"
#include "cJSON.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include <cstdlib>
#include <document.h>
#include <string>

namespace esphalib
{

namespace api
{

namespace
{
constexpr const char *TAG{"API"};
int timeout_ms = 10000;
}

using namespace rapidjson;

std::string post_req(std::string_view path, std::string_view data, const bool return_response)
{
        if (get_ha_url().empty() || get_long_lived_access_token().empty()) {
                ESP_LOGE(TAG, "Failed to upload data: ha_url or access token not set yet");
                return std::string{};
        }

        // Create API URL. Will look something like http://HA_URL/api/states/entity.entity_NAME
        const std::string api_URL{get_ha_url() + std::string{path}};

        constexpr const char *bearer = "Bearer ";
        const std::string auth_data{bearer + get_long_lived_access_token()};

// Attempt to make API request to Home Assistant
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
        esp_http_client_config_t home_assistant_config = {
            .url = api_URL.c_str(),
            .method = HTTP_METHOD_POST,
            .timeout_ms = timeout_ms,
            .disable_auto_redirect = false,
            .is_async = false,
            .skip_cert_common_name_check = true,
        };

        ESP_LOGV(TAG, "Attempting connection to %s", api_URL.c_str());

        esp_http_client_handle_t client = esp_http_client_init(&home_assistant_config);
        esp_http_client_set_header(client, "Authorization", auth_data.c_str());
        esp_http_client_set_header(client, "Content-Type", "application/json");

        // Set POST field if data is available
        if (data.size() > 0) {
                esp_http_client_set_post_field(client, data.data(), data.size());
        }

        //  POST data
        esp_err_t err = esp_http_client_perform(client);
        if (err == ESP_OK) {
                if (data.size() > 0) {
                        ESP_LOGV(TAG, "Sent %s to %s", data.data(), api_URL.c_str());
                }
        } else {
                ESP_LOGE(TAG, "Could not send upload entity data.");
        }

        std::string ret_str;
        // POST Response handling for when return_response=true
        if (return_response) {
                if (err == ESP_OK) { // only get response if the above post was OK
                        // Receive POST response
                        err = esp_http_client_open(client, 0);
                        if (err == ESP_OK) {
                                esp_http_client_fetch_headers(client);
                                if (return_response) {
                                        auto const content_length = esp_http_client_get_content_length(client);
                                        auto response_buffer = new char[content_length + 1];
                                        if (response_buffer != nullptr) {
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
                                                delete[] response_buffer;
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

RequestResponse<std::string> get_req(std::string_view path)
{
        if (get_ha_url().empty() || get_long_lived_access_token().empty()) {
                ESP_LOGE(TAG, "Failed to GET: ha_url or access token not set yet");
                return {RequestStatus_type::FAILURE, std::string{}};
        }

        char *local_response_buffer = nullptr;

        // Create API URL. Will look something like http://HA_URL/api/states/entity.entity_NAME
        const std::string api_URL{get_ha_url() + std::string{path}};

        constexpr const char *bearer = "Bearer ";
        const std::string auth_data{bearer + get_long_lived_access_token()};

#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
        esp_http_client_config_t config = {
            .url = api_URL.c_str(),
            .method = HTTP_METHOD_GET,
            .timeout_ms = timeout_ms,
            .disable_auto_redirect = false,
            .user_data = local_response_buffer,
            .is_async = false,
            .skip_cert_common_name_check = true,
        };

        esp_http_client_handle_t client = esp_http_client_init(&config);
        esp_http_client_set_header(client, "Authorization", auth_data.c_str());
        esp_http_client_set_header(client, "Content-Type", "application/json");

        RequestStatus_type request_status{RequestStatus_type::UNKNOWN};
        std::string ret_str;
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
                                request_status = RequestStatus_type::FAILURE;
                        } else {
                                esp_http_client_read_response(client, local_response_buffer, content_length);
                                local_response_buffer[content_length] = '\0'; // ensure response is null-terminated
                                ret_str = std::string{local_response_buffer};
                                free(local_response_buffer);
                                ESP_LOGV(TAG, "Read %s, \nSize: %lld", ret_str.c_str(), content_length);
                                request_status = RequestStatus_type::SUCCESS;
                        }
                } else {
                        ESP_LOGE(TAG, "Failed to fetch request: %s", esp_err_to_name(fetcherr));
                        request_status = RequestStatus_type::FAILURE;
                }
        } else {
                ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
                request_status = RequestStatus_type::FAILURE;
        }

        esp_http_client_close(client);
        esp_http_client_cleanup(client);

        return {request_status, ret_str};
}

// Return the status of the API
// Requires that HA is network accessible
//
// NOTE: There's no point in returning the parsed JSON or string response of the request because the only JSON member
// just states if the API is running or not.
Status_type get_api_status(void)
{
        auto const req = get_req(STATUSPATH);

        auto status = Status_type::OFFLINE;
        if (req.response.empty() || req.status != api::RequestStatus_type::SUCCESS) {
                status = Status_type::UNKNOWN;
        } else {
                rapidjson::Document d;
                d.Parse(req.response);
                if (auto message_it = d.FindMember("message"); message_it != d.MemberEnd()) {
                        auto const message = (*message_it).value.GetString();
                        constexpr const char *API_RUNNING_STR = "API running.";
                        if (strcmp(message, API_RUNNING_STR) == 0) {
                                // Success! API request was valid and the string says it is running.
                                status = Status_type::ONLINE;
                        }
                }
        }
        return status;
}

} // namespace api
} // namespace esphalib
