#pragma once
#include <stdbool.h>

extern char* ha_url;
extern char* long_lived_access_token;

void set_ha_url(const char* new_url);
void set_long_lived_access_token(const char* new_long_lived_access_token);

char* get_req(char* path);
void post_req(char* path, char* data);
bool get_api_status(void);