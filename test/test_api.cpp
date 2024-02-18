#include "cJSON.h"
#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "nvs_control.hpp"
#include "wifi.h"
#include <cassert>
#include <cmath>

#include "lwip/err.h"
#include "lwip/sys.h"

#include "driver/gpio.h"

#include "esp_ha_lib.hpp"
#include "secrets.h"
#include <cstdlib>
#include <string>
#include <unity.h>

namespace esphalib
{

namespace Testing
{

namespace
{
constexpr const char *TAG = "test_api";
}

namespace api = esphalib::api;
namespace config = esphalib::config;
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
        auto const state = static_cast<float>(esp_random() % 100) + ((static_cast<float>(esp_random() % 100) / 100.0f));

        HAEntity entity;
        entity.state = std::to_string(state);
        entity.entity_id = entity_id;
        entity.add_attribute("friendly_name", friendly_entity_name);
        entity.add_attribute("unit_of_measurement", unit_of_measurement);
        entity.post();

        HAEntity *newEntity{HAEntity::get(entity_id)};
        TEST_ASSERT_NOT_NULL(newEntity);
        ESP_LOGI(TAG, "Uploaded: %f", state);
        // if std::stof fails, the program will abort.
        // TODO: Use gtest and check if it is a float instead of using std::stof.
        const float fstate{std::stof(newEntity->state)};
        ESP_LOGI(TAG, "Received %f", fstate);

        // HA only returns floats to 2 decimal places, it seems.
        constexpr const float epsilon{1e-2};
        TEST_ASSERT_FLOAT_WITHIN(epsilon, state, fstate);
        delete newEntity;
}

void test_api_running(void)
{
        auto const api_status = api::get_api_status();
        auto const expected_status = api::Status_type::ONLINE;
        TEST_ASSERT_TRUE_MESSAGE(api_status == expected_status, "API is not accessible and/or not running.");
}

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
        constexpr auto entity_name = "sensor.randomsensortest";
        constexpr auto friendly_entity_name = "esp ha lib sensor test";
        constexpr auto unit_of_measurement = "test units";
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
        auto const events_response = event::get_events();
        auto const expected_status = api::RequestStatus_type::SUCCESS;
        TEST_ASSERT(events_response.status == expected_status);
}

void test_get_event_from_events()
{
        // Given events from Home Assistant
        auto const events_response = event::get_events();
        auto const expected_status = api::RequestStatus_type::SUCCESS;
        TEST_ASSERT(events_response.status == expected_status);

        // Then, a known event in Home Assistant is found.
        auto const found_event = event::get_event_from_events("homeassistant_start", events_response.response);
        TEST_ASSERT_EQUAL_STRING("homeassistant_start", found_event.event.data());

        ESP_LOGI(TAG, "event:%s listener_count:%li", found_event.event.data(), found_event.listener_count);

        // Then, an event that is not in Home Assistant is not found.
        auto const not_found_event =
            event::get_event_from_events("this_event_should_not_exist", events_response.response);
        TEST_ASSERT_EQUAL_STRING("", not_found_event.event.data());
        size_t const expected_not_found_listener_count{0U};
        TEST_ASSERT(not_found_event.listener_count == expected_not_found_listener_count);
}

// TODO: make this actually useful other than testing for crashes
void test_post_event()
{
        // event::post_event("event.test");
        rapidjson::Document d;
        d.Parse("{\"next_rising\":\"2016-05-31T03:39:14+00:00\"}");

        auto response = event::post_event("event.test", d);
        /*
        // TODO: Check if post_event response is {"message":"Event event.test fired."}
        TEST_ASSERT(response.status == api::RequestStatus_type::SUCCESS);
        auto response_message_it = response.response.FindMember("message");
        TEST_ASSERT(response_message_it != response.response.MemberEnd());
        auto const response_message_expected = "Event event.test fired.";
        TEST_ASSERT(std::strcmp((*response_message_it).value.GetString(), response_message_expected) == 0);
        */
}

// Check if able to get a config from a known good Home Assistant instance
// Note: Does not check if the config is good (nor does it matter for this test)
void test_get_config()
{
        auto const config_response = esphalib::config::get_config();
        auto const expected_status = api::RequestStatus_type::SUCCESS;
        TEST_ASSERT(config_response.status == expected_status);

        // Check that a known good object is in the response. In this case, that is version.
        auto message_it = config_response.response.FindMember("version");
        TEST_ASSERT(message_it != config_response.response.MemberEnd());
}

void test_get_states()
{
        cJSON *states = state::get_states();
        auto *jsonstr = cJSON_Print(states);
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
        TEST_ASSERT_TRUE(config::check_config() == api::Config_Status_type::VALID);
}

void setup_wifi()
{
    // Make sure Wi-Fi credentials are at least available before testing Wi-Fi
    // This does not necessarily mean they are set
    static_assert(!Secrets::NETWORK_SSID.empty(), "NETWORK_SSID is not defined.");
    static_assert(!Secrets::NETWORK_PASSWORD.empty(), "NETWORK_PASSWORD is not defined.");

    auto const is_nvs_init_success = Nvs::init_nvs();
    // Wi-Fi needs NVS init.
    TEST_ASSERT_TRUE(is_nvs_init_success);
    Wifi::wifi_init_station();
    auto const is_wifi_connected = Wifi::wait_wifi_connected(pdMS_TO_TICKS(10000));
    TEST_ASSERT_TRUE(is_wifi_connected);
}

int runUnityTests(void)
{
        api::set_ha_url(Secrets::HA_URL);
        api::set_long_lived_access_token(Secrets::LONG_LIVED_ACCESS_TOKEN);

        UNITY_BEGIN();
        // Non Wi-Fi dependent tests
        {
            RUN_TEST(test_HAEntity_print);
            RUN_TEST(test_add_entity_attribute);
        }

        // Wi-Fi dependent tests
        {
            setup_wifi();
            RUN_TEST(test_api_running);
            RUN_TEST(test_entity_uploadreceive);
            /*
            RUN_TEST(test_print_real_HAEntity);
            RUN_TEST(test_get_events);
            RUN_TEST(test_get_event_from_events);
            RUN_TEST(test_post_event);
            RUN_TEST(test_get_config);
            RUN_TEST(test_get_states);
            RUN_TEST(test_post_config);
            */
        }
        return UNITY_END();
}

extern "C" {
void app_main(void) { runUnityTests(); }
}

} // namespace Testing
} // namespace esphalib
