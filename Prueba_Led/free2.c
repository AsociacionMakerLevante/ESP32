#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

uint8_t estado=0;

#define PIN_VERDE 27
#define PIN_AZUL  26
#define RETARDO_V 500
#define RETARDO_A 300




void config_led(void)
{
    gpio_reset_pin(PIN_VERDE);
     gpio_reset_pin(PIN_AZUL);
    gpio_set_direction(PIN_AZUL,GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_VERDE,GPIO_MODE_OUTPUT);
    return;
}




void led_verde(void *pvParameters)
{
    uint8_t estado=0;
while(1){
    estado=!estado;
 gpio_set_level(PIN_VERDE,estado);
 vTaskDelay(RETARDO_V/portTICK_PERIOD_MS);

}
}



void led_azul(void *pvParameters)
{
    uint8_t estado=0;
while(1){
    estado=!estado;
 gpio_set_level(PIN_AZUL,estado);
 vTaskDelay(RETARDO_A/portTICK_PERIOD_MS);

}
}





void app_main(void)
{

config_led();
xTaskCreate(&led_azul, "tarea1", 2048, NULL, 5, NULL);
xTaskCreate(&led_verde, "tarea2", 2048, NULL, 5, NULL);



}
