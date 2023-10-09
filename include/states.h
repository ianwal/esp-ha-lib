#pragma once

#include "cJSON.h"

typedef struct {
        char entity_id[256]; // definite 256 max from what I searched
        char *state;
        cJSON *attributes;     // attributes is another json object
        char last_changed[48]; // ISO 8601 formatted datettime ex. 2016-05-30T21:43:29.204838+00:00 = 32 char
        char last_updated[48]; // ISO 8601 as well
} HAEntity;

void HAEntity_delete(HAEntity *item);
void add_entity_attribute(const char *key, const char *value, HAEntity *entity);
HAEntity *get_entity(const char *entity_name);

void post_entity(HAEntity *entity);

void set_ha_url(const char *url);
void set_long_lived_access_token(const char *new_long_lived_access_token);

HAEntity *HAEntity_create(void);

void HAEntity_print(HAEntity *item);

cJSON *get_states(void);