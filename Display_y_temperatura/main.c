/*
Programa para la segunda sesión del taller de FreeRTOS de Maker Levante, donde explicamos:
- Como dividir un programa en ficheros .c y .h.
- El uso de volatile, extern y static con funciones y variables.
- Configuración y uso de una interrupción en el ESP32.
- Uso de colas para intercambiar información entre tareas.
- Como utilizar un semáforo para sincronizar dos tareas.
- Uso de un software timer.
- Instalación de librerías (llamadas componentes) en ISP-IDF.
- Ejemplo de configuración y uso del ADC del ESP32.

El programa nos permite visualizar las lecturas de un sensor de temperatura por el puerto serie y en 
un display. También nos permite controlar los LEDs mediante una interrupción asociada a un pulsador.

Asignación de pines del ESP32:
LED Rojo: pin 2 ESP32.
LED Verde: pin 1 ESP32.
LED Azul: pin 0 ESP32.
Pulsador:  pin 4 ESP32.
SDA Display: pin 9 ESP32.
SCL Display: pin 8 ESP32.
Sensor de temperatura: pin 3 ESP32.
*/

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "colas.h"
#include "display.h"
#include "gpios.h"
#include "sensor_temp.h"
#include "temporizador.h"
#include "imprimir.h"

void app_main(void)
{
    //Desde la función main llamamos a las distintas tareas que van a estar corriendo en el ESP.

    //Creamos la cola que almacena las medidas de temperaturas y su semáforo. 
    //Si no creamos la cola antes de usarla por las tareas el esp se reinicia.
    colas_crear_cola_temp();
    //Creamos la tarea que controlar los LEDs desde el pulsador y un semáforo 
    //controlado por el pulsador que usará la tarea de imprimir.
    gpios_crear_tarea_LEDs();
    //Creamos una tarea estática para imprimer mensajes por el puerto serie.
    imprimir_crear_tarea_info();
    //Creamos un timer de repetición por software que se dispara cada 500 ms, parpadea el led azul.
    temporizador_crear_timer();
    //Creamos la tarea para leer el sensor de temperatura usando el ADC.
    sensor_temperatura_crear_tarea();
    //Creamos una tarea que escriba en el display por bus I2C.
    display_crear_task();

    //Esta función no requiere de un while(1), solo se ejecuta una vez. 
}






