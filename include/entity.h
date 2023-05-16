#pragma once

typedef struct {
    char entity_id[256]; // definite 256 max from what I searched
    char* state; 
    char* attributes;
    char* last_changed[48]; // ISO 8601 formatted datettime ex. 2016-05-30T21:43:29.204838+00:00 = 32 char
    char* last_updated[48]; // ISO 8601 formatted datettime ex. 2016-05-30T21:43:29.204838+00:00 = 32 char
} HAEntity;

void HAEntity_destroy(HAEntity* item);
HAEntity* get_entity(char* entity_name);

void upload_entity_data(char* entity_name, char* friendly_entity_name, char* units, float data);

void set_ha_url(const char* url);
void set_long_lived_access_token(const char* new_long_lived_access_token);