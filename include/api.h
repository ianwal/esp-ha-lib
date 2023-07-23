#pragma once
#include <stdbool.h>

#define EVENTSPATH "/api/events"
#define STATESPATH "/api/states"
#define CONFIGPATH "/api/config"
#define CHECKCONFIGPATH "/api/config/core/check_config"

extern char *ha_url;
extern char *long_lived_access_token;

void set_ha_url(const char *new_url);
void set_long_lived_access_token(const char *new_long_lived_access_token);

char *get_req(const char *path);
char *post_req(const char *path, const char *data, bool return_response);
bool get_api_status(void);