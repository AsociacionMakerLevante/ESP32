#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

uint8_t estado=0;

#define PIN_LED 27

void config_led(void)
{
    gpio_reset_pin(PIN_LED);
    gpio_set_direction(PIN_LED,GPIO_MODE_OUTPUT);
    return;
}

void control_led(void)
{
estado=!estado;
gpio_set_level(PIN_LED,estado);
return;

}
void app_main(void)
{

config_led();
while(1){
    control_led();
    vTaskDelay(25/portTICK_PERIOD_MS);
}

}
