#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "freertos/queue.h"

#define QUEUE_TAM 5

QueueHandle_t cola;
uint8_t estado = 0;

#define PIN_VERDE 27
#define PIN_AZUL 26
#define RETARDO_V 500
#define RETARDO_A 300
#define PIN_INTE 33

#define GPIO_BIT_MASK (1ULL << PIN_INTE)

void config(void)
{
    gpio_reset_pin(PIN_VERDE);
    gpio_reset_pin(PIN_AZUL);
    gpio_reset_pin(PIN_INTE);
    gpio_set_direction(PIN_AZUL, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_VERDE, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_INTE, GPIO_MODE_INPUT);

    return;
}

void led_verde(void *pvParameters)
{
    uint8_t estado = 0;
    while (1)
    {
        estado = !estado;
        gpio_set_level(PIN_VERDE, estado);
        vTaskDelay(RETARDO_V / portTICK_PERIOD_MS);
    }
}

void led_azul(void *pvParameters)
{

    uint8_t estado = 0;
    int valor;
    while (1)
    {

        if (xQueueReceive(cola, &valor, portMAX_DELAY) == pdTRUE)
        {
            estado = !estado;
            gpio_set_level(PIN_AZUL, estado);
            vTaskDelay(RETARDO_A / portTICK_PERIOD_MS);
        }
    }
}
    void leer_inte(void *pvParameters)
    {

        while (1)
        {
            int valor = gpio_get_level(PIN_INTE);
            vTaskDelay(100 / portTICK_PERIOD_MS);

            if (valor)
            {
                if (xQueueSend(cola, &valor, portMAX_DELAY) == pdTRUE)
                {
                    printf("Pongo en cola\n");
                }
            }
        }
    }

    void app_main(void)
    {
        
        config();
        gpio_config_t io_conf;
            io_conf.intr_type = GPIO_INTR_DISABLE;
            io_conf.mode = GPIO_MODE_INPUT;
            io_conf.pin_bit_mask = GPIO_BIT_MASK;
            io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
            io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
            gpio_config(&io_conf);


        

        cola = xQueueCreate(QUEUE_TAM, sizeof(int));
        
        xTaskCreate(&led_azul, "tarea1", 2048, NULL, 5, NULL);
        xTaskCreate(&led_verde, "tarea2", 2048, NULL, 5, NULL);
        xTaskCreate(&leer_inte, "tarea3", 2048, NULL, 5, NULL);
        
    }
