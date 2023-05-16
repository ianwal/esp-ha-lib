#pragma once

void upload_entity_data(char* sensor_name, char* friendly_sensor_name, char* units, float data);
char* get_entity_req(char* sensor_name);
void set_ha_url(const char* url);
void set_long_lived_access_token(const char* new_long_lived_access_token);
char* get_entity_state(char* sensor_name);