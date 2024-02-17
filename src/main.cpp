#ifndef PROJECTIO_TESTING

#include <cstdio>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

extern "C" {
void app_main()
{
        while (true) {
                printf("Hello PlatformIO!\n");
                vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
}
}
#endif
