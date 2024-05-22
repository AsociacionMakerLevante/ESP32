#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define led 27
uint8_t led_level=0;

esp_err_t config_led(void)
{
    gpio_reset_pin(led);
    gpio_set_direction(led,GPIO_MODE_OUTPUT);
    return ESP_OK;
}
esp_err_t control_led(void)
{
  led_level=!led_level;
  gpio_set_level(led,led_level);
  return ESP_OK;

}

void app_main()
{
 config_led();
 while(1){
 vTaskDelay(200/portTICK_PERIOD_MS);
 control_led();
 }
}