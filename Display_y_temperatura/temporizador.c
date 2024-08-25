/*
API:
https://www.freertos.org/Documentation/02-Kernel/04-API-references/11-Software-timers/01-xTimerCreate
*/
#include "temporizador.h"

//Función que a la que llama el timer por software cada ver que ocurre.
static void temporizador_callback_timer(TimerHandle_t xTimer);

//Función que llamamos para crear el timer por software.
void temporizador_crear_timer(void)
{
    //Creamos un software timer dinámico de repetición. Cada callback se ejecutará en la tarea TmrSvc de FreeRTOS.
    //Explicar lo que es un callback. 
    TimerHandle_t repeatHandle =
	xTimerCreate(	"repeatTimer",			            //nombre para el timer
					TIEMPO_TIMER /portTICK_PERIOD_MS,	//periodo del timer
					pdTRUE,						        //auto-reload flag
					NULL,						        //ID del timer
					temporizador_callback_timer);		//callback function  

    xTimerStart(repeatHandle, 0); //Inicializamos el timer por software.

    printf("Timer de repetición creado e inicializado.\n"); 
}

//Función que hace parpadear el led azul, se llama cada vez que ocurre el timer de repetición.
static void temporizador_callback_timer(TimerHandle_t xTimer)
{
    static enum estadoLED estado_azul = OFF;
    gpios_LED_Azul(estado_azul);
    //Los enum son valores númerico, a OFF se le asigna el valor 0 en este caso.
    estado_azul = !estado_azul;
}