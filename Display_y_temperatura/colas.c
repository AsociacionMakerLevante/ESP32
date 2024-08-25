/*
API: https://www.freertos.org/Documentation/02-Kernel/04-API-references/06-Queues/00-QueueManagement
     https://www.freertos.org/Documentation/02-Kernel/04-API-references/10-Semaphore-and-Mutexes/00-Semaphores
*/

#include "colas.h"

//Número de elementos de la cola de temperaturas.
#define QUEUE_LENGTH  10
//Tamaño de los elemenos de la cola (tamaño de la lectura del ADC).
#define ITEM_SIZE  sizeof( float )

//Array lo suficientemente grande para contener todos los elemenos de la cola.
uint8_t ucQueueStorageArea[ QUEUE_LENGTH * ITEM_SIZE ];
//Variable que guarda la estructura de datos de la cola.
//Explicar el uso de static en las variables globales.
static StaticQueue_t xStaticQueue; 

//Creamos un puntero para usar el semáforo desde las tareas.
SemaphoreHandle_t colasTempSemaforoHandle = NULL;
//Memoria RAM donde se guarda el estado del semáforo.
static StaticSemaphore_t colasTempSemaforo;

//Variable (puntero) global que será usada por las tareas para acceder a la cola.
QueueHandle_t colasTempHandle = NULL;

void colas_crear_cola_temp(void)
{
    //Se crea cola para guardar medidas de temperaturas.
    colasTempHandle = xQueueCreateStatic
        ( QUEUE_LENGTH,         //Número máximo de elemenos de la cola.
        ITEM_SIZE,              //Tamaño en bytes de cada elemento de la cola.
        ucQueueStorageArea,     //Apunta a un array uint8_t del tamaño de multiplicar los 2 elementos anteriores.
        &xStaticQueue );        //Debe a punta a una variable StaticQueue_t
    
    //Se crea un semáforo para gestionar el acceso a la cola anterior.
    colasTempSemaforoHandle = xSemaphoreCreateBinaryStatic(&colasTempSemaforo);

    printf("Cola de temperaturas creada.\n");   
}