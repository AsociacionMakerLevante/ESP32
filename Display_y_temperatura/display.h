/*
Funciones para escribir en el display una temperatura que cogemos de una cola donde se almacenan.
*/
#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "colas.h"

//Handler para suspender la task del display desde otras tasks.
extern TaskHandle_t xHandleDisplay; 
//Función para crear una task que escriba en el display la temperatura leída de una cola.
void display_crear_task(void);

#endif