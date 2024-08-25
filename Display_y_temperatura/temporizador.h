/*
Creamos un software timer que hacer parpadear el LED azul cada medio segundo.
De esta forma no reservamos memoria en RAM o en el heap para la creación de una nueva tarea
que haga dicha función. Usamos memoria ya reservada por el RTOS.
*/
#ifndef TEMPORIZADOR_H
#define TEMPORIZADOR_H
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "gpios.h"

//Tiempo de repetición del timer por software en ms.
#define TIEMPO_TIMER 500 

//Función que llamaremos desde main para crear un software timer.
void temporizador_crear_timer(void);

#endif