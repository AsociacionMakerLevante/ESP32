/*
API: 
https://www.freertos.org/Documentation/02-Kernel/04-API-references/01-Task-creation/02-xTaskCreateStatic
https://www.freertos.org/Documentation/02-Kernel/04-API-references/10-Semaphore-and-Mutexes/12-xSemaphoreTake
*/
#include "imprimir.h"

//Tamaño del stack individual de cada tarea.
#define STACK_SIZE 2048
//Reserva en RAM del stack and task control block (TCB) para las tareas creadas estáticas
static StackType_t stack_tarea_imprimir[STACK_SIZE];
static StaticTask_t TCB_tarea_imprimir;
//Declaramos los handlers para las tasks estáticas por si queremos suspenderlas desde otras tasks.
TaskHandle_t xHandle_tarea_imprimir = NULL;

//Definimos la función asignada a la tarea para imprimir mensajes.
static void imprimir_info(void *pvParameters);

//Función para crear la tarea desde el main.
void imprimir_crear_tarea_info(void)
{
    xHandle_tarea_imprimir = xTaskCreateStatic(
    &imprimir_info,             //Función que se asigna a al tarea.
    "tareaImprimir",            //Nombre que le asignamos a la tarea (debug). 
    STACK_SIZE,                 //Tamaño del stack de la tarea (reservado de manera estática previamente).
    NULL,                       //Parámetros pasados a la tarea en su creación.
    tskIDLE_PRIORITY + 1,       //Prioridad de la tarea. 
    stack_tarea_imprimir,       //Array para el stack de la tarea reservado previamente. 
    &TCB_tarea_imprimir);       //Memoria reservada previamente para el TCB (task control block) de la tarea.
    
    printf("Tarea para imprimir por el puerto serie creada.\n");  
}

//Tarea para imprimir mensajes desde el pulsador mediante el uso de un semáforo.
//Comparar con la tarea que controlar los LEDs usando una variable global en lugar de un semáforo.
static void imprimir_info(void *pvParameters)
{
    while(1)
    {
        //Esperamos un semáforo que da el pulsador. La tarea permanece bloqueada mientras el semáforo no esté libre.
        //portMAX_Delay es un retardo de espera de varios días.
        if(xSemaphoreTake(gpiosSemPulsador, portMAX_DELAY))
        {
            printf("Numero de veces que se ha pulsado el pulsador: %d.\n", contadorInterrupcion);
        }
    }
}