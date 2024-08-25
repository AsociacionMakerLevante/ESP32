/*
Funciones para crear una cola y un semáforo para comunicar las tareas del sensor de temperatura
con las tareas del display.
*/
#ifndef COLAS_H
#define COLAS_H

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

//Variables globales que serán usadas por las tareas para acceder a la cola y el semáforo creados.
extern QueueHandle_t colasTempHandle;
extern SemaphoreHandle_t colasTempSemaforoHandle;

//Crea una cola para guardar las temperaturas medidas y un semáforo que sincroniza las tareas que usan dicha cola.
void colas_crear_cola_temp(void);

#endif