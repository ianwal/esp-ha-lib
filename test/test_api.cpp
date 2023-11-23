
extern "C" {
#include "wifi.h"
}
#include "cJSON.h"
#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include <assert.h>
#include <math.h>

#include "lwip/err.h"
#include "lwip/sys.h"

#include "driver/gpio.h"

#include "esp_ha_lib.hpp"
#include "secrets.hpp"
#include "wifisecrets.h"
#include <stdlib.h>
#include <string>
#include <unity.h>

namespace esphalib
{

namespace testapi
{

namespace
{
constexpr const char *TAG = "test_api";
}

namespace api = esphalib::api;
namespace event = esphalib::event;
namespace state = esphalib::state;

using esphalib::event::HAEvent;
using esphalib::state::HAEntity;

// Attempts to upload and get back a float for a test entity
void test_entity_uploadreceive(void)
{
        // entity name cannot contain special characters other than _ or . it seems
        constexpr const char *entity_id = "sensor.esphalibtest";
        constexpr const char *friendly_entity_name = "esp ha lib test";
        constexpr const char *unit_of_measurement = "Test Units";
        const float state = (esp_random() % 100);

        HAEntity entity;
        entity.state = std::to_string(state);
        entity.entity_id = entity_id;
        entity.add_attribute("friendly_name", friendly_entity_name);
        entity.add_attribute("unit_of_measurement", unit_of_measurement);
        entity.post();

        HAEntity *newEntity{HAEntity::get(entity_id)};
        TEST_ASSERT_NOT_NULL(newEntity);
        if (newEntity != nullptr) {
                // if std::stof fails, the program will abort
                const float fstate{std::stof(newEntity->state)};
                ESP_LOGI(TAG, "Uploaded: %f, Received %f", state, fstate);

                // HA only stores floats to 2 decimal places it seems
                constexpr const float epsilon{1e-2};
                TEST_ASSERT_FLOAT_WITHIN(epsilon, state, fstate);
        }
        delete newEntity;
}

void test_wifi_credentials_filled(void)
{
        TEST_ASSERT_NOT_EQUAL_MESSAGE(strcmp(NETWORK_SSID, ""), 0, "NETWORK_SSID is not set.");
        TEST_ASSERT_NOT_EQUAL_MESSAGE(strcmp(NETWORK_PASSWORD, ""), 0, "NETWORK_PASSWORD is not set.");
}

// Just used for testing, does not test the library
void test_wifi_connected(void) { TEST_ASSERT_MESSAGE(is_wifi_connected(), "WiFi is not connected"); }

void test_api_running(void) { TEST_ASSERT_MESSAGE(api::get_api_status(), "API is not accessible and/or not running."); }

void test_HAEntity_print(void)
{
        HAEntity entity{
            .entity_id = "print_test1",
            .state = "on!",
            .attributes = cJSON_Parse("{\"unit_of_measurement\":\"Test Units\",\"friendly_name\":\"esp ha libtest\"}"),
            .last_changed = const_cast<char *>("2023-05-17T10:41:28.855123+00:00"),
            .last_updated = const_cast<char *>("2021-02-12T10:41:28.422190+00:00")};
        entity.print();
}

// Tests printing a real HAEntity from Home Assistant
// Needs entity "sun.sun" which I think is built in or a substitute on a live home assistant to connect to
void test_print_real_HAEntity(void)
{
        HAEntity *entity{HAEntity::get("sun.sun")};
        TEST_ASSERT_NOT_NULL(entity);
        entity->print();
        delete entity;
}

// Tests adding a new entity attribute to an HAEntity
void test_add_entity_attribute(void)
{
        HAEntity entity;
        constexpr const char *entity_name = "sensor.randomsensortest";
        constexpr const char *friendly_entity_name = "esp ha lib sensor test";
        constexpr const char *unit_of_measurement = "test units";
        entity.add_attribute("entity_name", entity_name);
        entity.add_attribute("friendly_entity_name", friendly_entity_name);
        entity.add_attribute("unit_of_measurement", unit_of_measurement);

        TEST_ASSERT_EQUAL_STRING(entity_name,
                                 cJSON_GetStringValue(cJSON_GetObjectItem(entity.attributes, "entity_name")));
        TEST_ASSERT_EQUAL_STRING(friendly_entity_name,
                                 cJSON_GetStringValue(cJSON_GetObjectItem(entity.attributes, "friendly_entity_name")));
        TEST_ASSERT_EQUAL_STRING(unit_of_measurement,
                                 cJSON_GetStringValue(cJSON_GetObjectItem(entity.attributes, "unit_of_measurement")));
}

void test_get_events()
{
        cJSON *events = event::get_events();
        TEST_ASSERT_NOT_NULL_MESSAGE(events, "get_events() was NULL");

        char *jsonstr = cJSON_Print(events);

        // if jsonstr is null, means either get_events failed or somehow the json was parsed wrong
        TEST_ASSERT_NOT_NULL_MESSAGE(jsonstr, "get_events() was NULL");

        ESP_LOGI(TAG, "Events - %s", jsonstr);

        constexpr const char *ha_start_event_name = "homeassistant_start";
        auto eve = event::get_event_from_events(ha_start_event_name, events);
        ESP_LOGI(TAG, "event:%s listener_count:%d", eve.event, eve.listener_count);

        free(jsonstr);

        cJSON_Delete(events);
}

void test_get_event_from_events()
{
        cJSON *events = event::get_events();

        auto eve = event::get_event_from_events("homeassistant_start", events);
        TEST_ASSERT_EQUAL_STRING("homeassistant_start", eve.event);

        ESP_LOGI(TAG, "event:%s listener_count:%d", eve.event, eve.listener_count);

        cJSON_Delete(events);
}

void test_post_event() { event::post_event("event.test", NULL); }

void test_get_config()
{
        cJSON *config = esphalib::config::get_config();
        auto jsonstr = cJSON_Print(config);
        TEST_ASSERT_NOT_NULL_MESSAGE(jsonstr, "get_config() was NULL");
        if (jsonstr != nullptr) {
                ESP_LOGI(TAG, "Config - %s", jsonstr);
        }
        TEST_ASSERT_NOT_EQUAL_MESSAGE(0, cJSON_GetArraySize(config), "Config GET failed.");
        free(jsonstr);
        cJSON_Delete(config);
}

void test_get_states()
{
        cJSON *states = state::get_states();
        auto jsonstr = cJSON_Print(states);
        TEST_ASSERT_NOT_NULL_MESSAGE(jsonstr, "get_states() was NULL");
        if (jsonstr != nullptr) {
                ESP_LOGI(TAG, "States - %s", jsonstr);
        }
        TEST_ASSERT_NOT_EQUAL_MESSAGE(0, cJSON_GetArraySize(states), "States GET failed.");
        free(jsonstr);
        cJSON_Delete(states);
}

void test_post_config()
{
        // from good home_assistant config. if config is bad this test will fail and it's not necessarily the fault of
        // the library
        TEST_ASSERT_TRUE(esphalib::config::check_config());
}

// Sets and checks to make sure the url and long lived access token are set before doing tests
// Tests that follow will fail if they're not set which is good, but it's annoying to debug.
void test_set_url_and_token()
{
        TEST_ASSERT_NOT_EQUAL_MESSAGE(strcmp(HA_URL, ""), 0, "HA_URL is not set.");
        TEST_ASSERT_NOT_EQUAL_MESSAGE(strcmp(LONG_LIVED_ACCESS_TOKEN, ""), 0, "LONG_LIVED_ACCESS_TOKEN is not set.");

        api::set_ha_url(HA_URL);
        api::set_long_lived_access_token(LONG_LIVED_ACCESS_TOKEN);

        TEST_ASSERT_EQUAL_STRING_MESSAGE(HA_URL, api::ha_url.c_str(), "HA_URL failed to be set.");
        TEST_ASSERT_EQUAL_STRING_MESSAGE(LONG_LIVED_ACCESS_TOKEN, api::long_lived_access_token.c_str(),
                                         "long_lived_access_token failed to be set.");
}

int runUnityTests(void)
{
        UNITY_BEGIN();
        // Non Wi-Fi dependent tests
        RUN_TEST(test_HAEntity_print);
        RUN_TEST(test_add_entity_attribute);

        // Wi-Fi dependent tests
        RUN_TEST(test_wifi_credentials_filled);
        wifi_init_sta();
        RUN_TEST(test_wifi_connected);
        RUN_TEST(test_set_url_and_token);
        RUN_TEST(test_api_running);
        RUN_TEST(test_entity_uploadreceive);
        RUN_TEST(test_print_real_HAEntity);
        RUN_TEST(test_get_events);
        RUN_TEST(test_get_event_from_events);
        RUN_TEST(test_post_event);
        RUN_TEST(test_get_config);
        RUN_TEST(test_get_states);
        RUN_TEST(test_post_config);
        return UNITY_END();
}

extern "C" {
void app_main(void) { runUnityTests(); }
}

} // namespace testapi
} // namespace esphalib
