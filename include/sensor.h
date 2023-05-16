#pragma once

void upload_sensor_data(char* sensor_name, char* friendly_sensor_name, char* units, float data);
char* get_sensor_req(char* sensor_name);
void set_ha_url(const char* url);
void set_long_lived_access_token(const char* new_long_lived_access_token);