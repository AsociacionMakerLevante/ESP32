/*Funciones para el control de los LEDS y el pulsador.
Función que crea una tarea para controlar los LEDs rojo y verde desde el pulsador por interrupción.
La interrupción del pulsador da un semáforo a la tarea de imprimir por el puerto serie para que se ejecute.
*/

#ifndef GPIOS_H
#define GPIOS_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

//Variable que lleva la cuenta de las veces que entramos en la interrupción del pulsador.
extern volatile int contadorInterrupcion;

//Handler para la tarea que controla los LEDs y el pulsador.
extern TaskHandle_t xHandle_gpios_LEDs;

//Semáforo que controlaremos desde el botón para imprimir mensajes.
extern SemaphoreHandle_t gpiosSemPulsador;

//Parámetro que le pasamos a la función para encender o apagar el LED.
enum estadoLED {OFF, ON}; 
//Función para crear la tarea que controla los LEDs rojo y verde desde el pulsador.
void gpios_crear_tarea_LEDs(void); 
//Función para encender o apagar el LED azul.
void gpios_LED_Azul(enum estadoLED estado); 

#endif