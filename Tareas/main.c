#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

//Definimos a que número de pin del MCU conectamos los LEDs y pulsador.
#define LED_AZUL 0
#define LED_VERDE 1
#define LED_ROJO 2
#define PULSADOR 4

//Tamaño del stack individual de cada tarea.
#define STACK_SIZE 2048

//Inicializamos el hardware (periféricos) del microcontrolador.
void MCUInit(void);

//Función (task) para conmutar el led azul.
void taskLedAzul(void *pvParameters);

//Función (task)para conmutar el led rojo.
void taskLedRojo(void *pvParameters);

//Función (task) para conmutar el led verde.
void taskLedVerde(void *pvParameters);

//Función (task) para leer el pulsador.
void taskPulsador(void *pvParameters);

//Función (task) para imprimir mensajes al principio del programa.
void taskImprimir(void *pvParameters);

//Estas variables globales se usan para borrar o suspender la tasks desde otras tasks.
TaskHandle_t taskAzulHandle;
TaskHandle_t taskVerdeHandle;

//Define el stack and task control block (TCB) para la task del led rojo
static StackType_t taskRojoStack[STACK_SIZE];
static StaticTask_t taskRojoTCB;

void app_main(void)
{
    //Se inicia el hardware del microcontrolador.
    MCUInit();

    //Creamos las tareas que se van a ejecutar en paralelo.

    //Tarea en memoria dinámica sin comprobar su valor de retorno al crearla.
    xTaskCreate(&taskLedVerde, "tarea1", STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &taskVerdeHandle);

    //Tarea en memoria dinámica comprobando su valor de retorno al crearla.
    if(xTaskCreate(&taskLedAzul, "tarea2", STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &taskAzulHandle)!= pdPASS)
    { while(1); }

    //Tarea en memoria estática. Siempre se crea al ser memoria estática, si no se puede crear daría
    //error en el proceso de linkado.
    xTaskCreateStatic(&taskLedRojo, "tarea3", STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, taskRojoStack, &taskRojoTCB);

    //Creamos la tarea para leer el pulsador por polling. Es adecuado hacerlo mediante ISR.
    xTaskCreate(&taskPulsador, "tarea4", STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);

    //Creamos la tarea para imprimer mensajes por el puerto serie.
    xTaskCreate(&taskImprimir, "tarea5", STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
}

//Configuramos el hardware del microcontrolador.
//Esta función se llamará  solo una vez al iniciarse el programa.
void MCUInit(void)
{
    //Configuramos el pin del led como salida.
    gpio_reset_pin(LED_VERDE);
    gpio_set_direction(LED_VERDE, GPIO_MODE_OUTPUT);

    //Configuramos el pin del led como salida.
    gpio_reset_pin(LED_ROJO);
    gpio_set_direction(LED_ROJO, GPIO_MODE_OUTPUT);

    //Configuramos el pin del led como salida.
    gpio_reset_pin(LED_AZUL);
    gpio_set_direction(LED_AZUL, GPIO_MODE_OUTPUT);

    //Configuramos el pin del pulsador como entrada.
    gpio_reset_pin(PULSADOR);
    gpio_set_direction(PULSADOR, GPIO_MODE_INPUT);
    //Deshabilita la resistencia de pull-down del pin.
    gpio_pulldown_dis(PULSADOR);
    //Habilita la resistencia de pull-up del pin.
    gpio_pullup_en(PULSADOR);

    printf("Microcontrolador inicializado.\n");
}

//Función (task) para conmutar el led azul.
void taskLedAzul(void *pvParameters)
{
    uint8_t estadoLed = 0;
    while(1)
    {
        estadoLed = !estadoLed;
        gpio_set_level(LED_AZUL, estadoLed);
        //Retardo de 500 milisegundos. Precisión baja. La tarea se bloquea.
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

//Función (task) para conmutar el led rojo.
void taskLedRojo(void *pvParameters)
{
    uint8_t estadoLed = 0;
    uint8_t contador = 0;
    uint8_t firstRun = 0;
    while(1)
    {
        estadoLed = !estadoLed;
        gpio_set_level(LED_ROJO, estadoLed);
        //Retardo de 500 milisegundos. Precisión baja. La tarea se bloquea.
        vTaskDelay(500 / portTICK_PERIOD_MS);
        if((++contador > 20) && (firstRun == 0))
        {
            vTaskDelete(taskAzulHandle);
            printf("Tarea del LED azul eliminada.\n");
            firstRun = 1;
        }
    }
}

//Función (task) para conmutar el led verde.
void taskLedVerde(void *pvParameters)
{
    uint8_t estadoLed = 0;
    while(1)
    {
        estadoLed = !estadoLed;
        gpio_set_level(LED_VERDE, estadoLed);
        //Retardo de 500 milisegundos. Precisión baja. La tarea se bloquea.
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

//Función (task) leer el pulsador por polling.
//Lo adecuado es leerlo mediante una ISR (próxima sesión).
void taskPulsador(void *pvParameters)
{
    uint8_t estado = 0;
    while(1)
    {
        if(gpio_get_level(PULSADOR) == 0)
        {
            if(estado == 0)
            {
                vTaskSuspend(taskVerdeHandle);
                printf("LED verde inactivo (tarea suspendida).\n");
                estado = 1;
            }
            else
            {
                vTaskResume(taskVerdeHandle);
                printf("LED verde activo (tarea resumida).\n");
                estado = 0;
            }
        }
        //Retardo de 1 segundo. Precisión baja. La tarea se bloquea.
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

//Función (task) para imprimir mensajes al principio del programa
//y destruirse una vez impresos los mensajes.
void taskImprimir(void *pvParameters)
{
    uint8_t contador = 0;

    printf("Imprimiendo el valor %d.\n", contador++);
    //Retardo de 1 segundo. Precisión baja. La tarea se bloquea.
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    printf("Imprimiendo el valor %d.\n", contador++);
    //Retardo de 1 segundo. Precisión baja. La tarea se bloquea.
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    printf("Imprimiendo el valor %d.\n", contador++);
    //Retardo de 1 segundo. Precisión baja. La tarea se bloquea.
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    printf("Imprimiendo el valor %d.\n", contador++);
    //Retardo de 1 segundo. Precisión baja. La tarea se bloquea.
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    //La tarea se elimina ella misma.
    printf("Se elimina esta tarea de imprimir.\n");
    vTaskDelete(NULL); //Comentar esta línea para ver lo que ocurre.
}