/*
Se crea una tarea que imprima mensajes por el puerto serie cuando se le da un semáforo desde el pulsador.
*/
#ifndef IMPRIMIR_H
#define IMPRIMIR_H

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gpios.h"

//Puntero para suspender o reanudar la tarea desde otras tasks.
extern TaskHandle_t xHandle_tarea_imprimir;

//Función que crea la tarea para imprimir mensajes.
void imprimir_crear_tarea_info(void);

#endif