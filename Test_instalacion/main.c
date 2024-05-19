#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void)
{
    uint32_t i = 0;
    while(1)
    {
        printf("Hello world! %lu\n", i++);
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}
