#include "esp_ha_lib.hpp"
#include "esp_log.h"
#include "esp_random.h"
#include "secrets.h"
#include "wifi.h"

#include <gtest/gtest.h>

namespace esphalib
{
namespace Testing
{

namespace api = esphalib::api;
namespace config = esphalib::config;
namespace event = esphalib::event;
namespace state = esphalib::state;

using esphalib::event::HAEvent;
using esphalib::state::HAEntity;

namespace
{

constexpr auto TAG = "TESTING";

} // namespace

TEST(esphalibtest, DefaultTest) { EXPECT_TRUE(true); }

// Tests adding a new entity attribute to an HAEntity
TEST(esphalibtest, AddEntityAttribute)
{
        HAEntity entity;
        constexpr auto entity_name = "sensor.randomsensortest";
        constexpr auto friendly_entity_name = "esp ha lib sensor test";
        constexpr auto unit_of_measurement = "test units";
        entity.add_attribute("entity_name", entity_name);
        entity.add_attribute("friendly_entity_name", friendly_entity_name);
        entity.add_attribute("unit_of_measurement", unit_of_measurement);

        EXPECT_STREQ(entity_name, cJSON_GetStringValue(cJSON_GetObjectItem(entity.attributes, "entity_name")));
        EXPECT_STREQ(friendly_entity_name,
                     cJSON_GetStringValue(cJSON_GetObjectItem(entity.attributes, "friendly_entity_name")));
        EXPECT_STREQ(unit_of_measurement,
                     cJSON_GetStringValue(cJSON_GetObjectItem(entity.attributes, "unit_of_measurement")));
}

// Attempts to upload and get back a float for a test entity
TEST(esphalibtest_integration, EntityUploadReceive)
{
        // entity name cannot contain special characters other than _ or . it seems
        constexpr auto entity_id = "sensor.esphalibtest";
        constexpr auto friendly_entity_name = "esp ha lib test";
        constexpr auto unit_of_measurement = "Test Units";
        auto const state = static_cast<float>(esp_random() % 100) + ((static_cast<float>(esp_random() % 100) / 100.0f));

        HAEntity entity;
        entity.state = std::to_string(state);
        entity.entity_id = entity_id;
        entity.add_attribute("friendly_name", friendly_entity_name);
        entity.add_attribute("unit_of_measurement", unit_of_measurement);
        entity.post();
/*
        auto const new_entity = HAEntity::get(entity_id);
        EXPECT_TRUE(new_entity.has_value());
        ESP_LOGI(TAG, "Uploaded: %f", state);
        // if std::stof fails, the program will abort.
        // TODO: Check if it is a float instead of / before using std::stof.
        const float fstate{std::stof(new_entity.value().state)};
        ESP_LOGI(TAG, "Received %f", fstate);

        // HA only returns floats to 2 decimal places, it seems.
        constexpr auto epsilon{1e-2f};
        EXPECT_NEAR(fstate, state, epsilon);
        */
}

void init_gtest()
{
        ::testing::InitGoogleTest();
        if (RUN_ALL_TESTS()) {
        };
}

extern "C" {
int app_main(int argc, char **argv)
{
        // Setup Home Assistant info.
        {
                static_assert(!Secrets::LONG_LIVED_ACCESS_TOKEN.empty());
                static_assert(!Secrets::HA_URL.empty());
                static_assert(Secrets::HA_URL.back() != '/', "HA URL must not have a leading slash.");
                esphalib::api::set_ha_url(Secrets::HA_URL);
                esphalib::api::set_long_lived_access_token(Secrets::LONG_LIVED_ACCESS_TOKEN);
        }

        // Setup WiFi
        {
                Wifi::wifi_init_station();
        }

        init_gtest();
        // Always return zero-code and allow PlatformIO to parse results.
        return 0;
}
}

} // namespace Testing
} // namespace esphalib
