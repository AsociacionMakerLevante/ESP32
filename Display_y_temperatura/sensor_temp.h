/*
Funciones para leer el sensor de temperatura mediante el uso del ADC.
Se crea una tarea que manda una lectura del sesor de forma periódica a una cola que sera usada 
por el LCD para representar la temperatura. Para gestionar el acceso a la cosa se usa un semáforo.
*/
#ifndef SENSOR_TEMP_H
#define SENSOR_TEMP_H

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "colas.h"

extern TaskHandle_t xHandleSenTemp;

//Función que llamamos desde el main para crear la tarea del sensor de temperatura.
void sensor_temperatura_crear_tarea(void);

#endif