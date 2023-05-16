#pragma once

void upload_sensor_data(char* sensor_name, char* friendly_sensor_name, char* units, float data);
char* get_sensor_req(char* sensor_name);