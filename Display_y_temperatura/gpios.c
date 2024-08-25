/*
Funciones para el control de los LEDS y el pulsador.
APIS: 
https://docs.espressif.com/projects/esp-idf/en/stable/esp32c3/api-reference/peripherals/gpio.html#
https://www.freertos.org/Documentation/02-Kernel/04-API-references/10-Semaphore-and-Mutexes/17-xSemaphoreGiveFromISR
https://www.freertos.org/Documentation/02-Kernel/04-API-references/01-Task-creation/02-xTaskCreateStatic
Sobre las interrupciones:
https://docs.espressif.com/projects/esp-idf/en/v5.0/esp32c3/api-reference/system/intr_alloc.html
https://github.com/pycom/esp-idf-2.0/blob/master/components/esp32/include/esp_intr_alloc.h
*/
#include "gpios.h"
//Hablar sobre donde poner los siguientes includes.
#include <stdio.h>
#include "driver/gpio.h"
#include "esp_attr.h" //IRAM_ATTR: lo usamos para poner el código de la ISR del botón en RAM en lugar de FLASH.

//Definimos a que número de pin del MCU conectamos los LEDs y el pulsador.
#define LED_AZUL 0
#define LED_VERDE 1
#define LED_ROJO 2
#define PULSADOR 4
#define ESP_INTR_FLAG_DEFAULT 0
//Stack size de la tarea que controlar los LEDs con el pulsador.
#define STACK_SIZE 2048

//Reserva en RAM del stack and task control block (TCB) para las tareas creadas estáticas
static StackType_t stack_tarea_LEDs[STACK_SIZE];
static StaticTask_t TCB_tarea_LEDs;
//Declaramos los handlers para las tasks estáticas por si queremos suspenderlas desde otras tasks.
TaskHandle_t xHandle_gpios_LEDs = NULL;
//Variable para llevar la cuenta de las veces que entramos en la ISR del pulsador.
volatile int contadorInterrupcion; //Explicar el uso de volatile.
//Variable que usa el pulsador en su ISR para seleccionar los LEDs encendidos.
enum estadoLEDs {VERDE, ROJO, ENCENDIDOS, APAGADOS};
static volatile enum estadoLEDs estado_LEDs = APAGADOS;

//Función que llamamos cuando ocurra la ISR del pulsador. 
//RAM_ATTR: En RAM se ejecuta más rápido. 
static void IRAM_ATTR pulsador_isr_handler(void *arg); 

//Semáforo que controlaremos desde el botón para imprimir mensajes.
//Puntero al semáforo para referinos a él.
SemaphoreHandle_t gpiosSemPulsador = NULL;
//Memoria que guarda el estado del semáforo en RAM (asignada de manera estática, no en el heap).
static StaticSemaphore_t SemPulsador;

//Declaración de función para inicializar el hardware de los pines de los LEDs y el pulsador.
static void gpios_init_hardware(void);
//Declaración de funciones para encer y apagar los LEDs
void gpios_LED_Azul(enum estadoLED estado);
static void gpios_LED_Verde(enum estadoLED estado); //Explicar el uso de static comparando con la declaración anterior.
static void gpios_LED_Rojo(enum estadoLED estado);

//Función que asignamos a la tarea de controlar los LEDs desde el pulsador.
static void gpios_controlar_LEDs(void *pvParameters);


//Definición de funciones para encer y apagar los LEDs
void gpios_LED_Azul(enum estadoLED estado)
{
    gpio_set_level(LED_AZUL, estado);
}
static void gpios_LED_Verde(enum estadoLED estado)
{
    gpio_set_level(LED_VERDE, estado);
}
static void gpios_LED_Rojo(enum estadoLED estado)
{
    gpio_set_level(LED_ROJO, estado);
}

//Definición de función para inicializar el hardware de los pines de los LEDs y el pulsador.
static void gpios_init_hardware(void)
{
    //Configuramos el pin del led verde como salida.
    gpio_reset_pin(LED_VERDE);
    gpio_set_direction(LED_VERDE, GPIO_MODE_OUTPUT);
    gpio_pullup_dis(LED_VERDE);
    gpio_set_level(LED_VERDE, 0);

    //Configuramos el pin del led rojo como salida.
    gpio_reset_pin(LED_ROJO);
    gpio_set_direction(LED_ROJO, GPIO_MODE_OUTPUT);
    gpio_pullup_dis(LED_ROJO);
    gpio_set_level(LED_ROJO, 0);

    //Configuramos el pin del led azul como salida.
    gpio_reset_pin(LED_AZUL);
    gpio_set_direction(LED_AZUL, GPIO_MODE_OUTPUT);
    gpio_pullup_dis(LED_AZUL);  
    gpio_set_level(LED_AZUL, 0);

    //Configuramos el pin del pulsador como entrada.
    gpio_reset_pin(PULSADOR);
    gpio_set_direction(PULSADOR, GPIO_MODE_INPUT);
    //Deshabilitamos la resistencia de pull-up del pulsador y usamos una externa (se puede usar solo la interna también).
    gpio_pullup_dis(PULSADOR);
    //Configurar la interrupción del pulsador.
    gpio_set_intr_type(PULSADOR, GPIO_INTR_POSEDGE);
    //Instala el driver's GPIO ISR handler service.
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    //Asignar la función a la que se llamará cuando ocurra la ISR del pulsador.
    gpio_isr_handler_add(PULSADOR, pulsador_isr_handler, NULL);

    printf("Hardware de los LEDs y el pulsador inicializado.\n");     
}


//Se crea una tarea en memoria estática para controlar los LEDs desde el pulsador.
void gpios_crear_tarea_LEDs(void)
{
    xHandle_gpios_LEDs = xTaskCreateStatic(
    &gpios_controlar_LEDs,      //Función que se asigna a al tarea.
    "tareaLEDs",                //Nombre que le asignamos a la tarea (debug). 
    STACK_SIZE,                 //Tamaño del stack de la tarea (reservado de manera estática previamente).
    NULL,                       //Parámetros pasados a la tarea en su creación.
    tskIDLE_PRIORITY + 1,       //Prioridad de la tarea. 
    stack_tarea_LEDs,           //Array para el stack de la tarea reservado previamente. 
    &TCB_tarea_LEDs);           //Memoria reservada previamente para el TCB (task control block) de la tarea.

    //Se crea un semáforo para imprimir mensajes desde el pulsador.
    gpiosSemPulsador = xSemaphoreCreateBinaryStatic(&SemPulsador);

    printf("Tarea para controlar los LEDs desde el pulsador creada.\n");  
}

//Tarea para controlar los LEDs desde el pulsador mediante el uso de una variable global (estado_LEDs).
//Polling, comparar con la tarea de imprimir usando un semáforo en lugar de una variable global.
static void gpios_controlar_LEDs(void *pvParameters)
{
    gpios_init_hardware();
    enum estadoLEDs ultimo_estado = estado_LEDs;

    while(1)
    {
        //Se entra en el if si el estado de los LEDs ha cambiado, no entrando cada vez que se ejecuta la tarea, más eficiente.
        if(ultimo_estado != estado_LEDs)
        {
            switch (estado_LEDs)
            {
                case VERDE:
                    gpios_LED_Rojo(OFF);
                    gpios_LED_Verde(ON);
                    printf("LED Verde encendido.\n\n"); 
                    break;  
                case ROJO:
                    gpios_LED_Rojo(ON);
                    gpios_LED_Verde(OFF);
                    printf("LED Rojo encendido.\n\n"); 
                    break;  
                case ENCENDIDOS:   
                    gpios_LED_Rojo(ON);
                    gpios_LED_Verde(ON);
                    printf("LEDs Rojo y Verde encendidos.\n\n"); 
                    break;
                case APAGADOS:   
                    gpios_LED_Rojo(OFF);
                    gpios_LED_Verde(OFF);
                    printf("LEDs Rojo y Verde Apagados.\n\n"); 
                    break;
                default: break;        
            }
            ultimo_estado = estado_LEDs;
            //Por hacer: comprobar que no reentre más de una vez por pulsación.
        }
        //Esta tarea se ejecuta cada 100 ms.
        vTaskDelay(100 / portTICK_PERIOD_MS); 
    }

}

//Función del pulsador para la ISR. En RAM se ejecuta más rápido.
//ISRs usually have a limited amount of stack space allocated, 
//and usually have limited time to perform its task.
//Si se resetea el MCU al entrar en la ISR incumplimos lo anterior.
//No llamar por ejemplo a printf desde dentro de la ISR ya que nos quedamos sin stack o tiempo.
static void IRAM_ATTR pulsador_isr_handler(void *arg)
{
    switch (estado_LEDs)
    {
        case VERDE:
            estado_LEDs = ROJO;
            break;  
        case ROJO:
            estado_LEDs = ENCENDIDOS;
            break;  
        case ENCENDIDOS:   
            estado_LEDs = APAGADOS;
            break;
        case APAGADOS:   
            estado_LEDs = VERDE;
            break;
        default: break;        
    }
    //Damos el semáforo a la tarea de imprimir para que imprima un mensaje.
    xSemaphoreGiveFromISR(gpiosSemPulsador, NULL);
    //Llevamos la cuenta de las veces que se ha pulsado el pulsador desde el inicio del programa.
    contadorInterrupcion++;
}