/*
No utilizamos la API de ESP-IDF si no que instalamos un componentes más sencillo (SSD1306) para este ejemplo
mediante  ESP-IDF: Show ESP Component Registry
https://components.espressif.com/
https://www.phippselectronics.com/how-to-add-a-component-in-esp-idf-in-vscode/
https://www.freertos.org/Documentation/02-Kernel/04-API-references/06-Queues/09-xQueueReceive
https://www.freertos.org/Documentation/02-Kernel/04-API-references/01-Task-creation/02-xTaskCreateStatic
*/

//Hablar sobre donde poner estos #includes, en el .h o aquí?
#include "display.h"
#include "ssd1306.h"

//Pines del ESP32-C3 a los que conectamos el I2C del display..
#define I2C_MASTER_SCL_IO 8         // gpio number for I2C master clock 
#define I2C_MASTER_SDA_IO 9         // gpio number for I2C master data  
#define I2C_MASTER_NUM I2C_NUM_0    //I2C port number for master dev 
#define I2C_MASTER_FREQ_HZ 100000   //I2C master clock frequency 

//Usado por las funciones que configurar el display.
static ssd1306_handle_t ssd1306_dev = NULL;

//Tamaño del stack individual de las tareas.
#define STACK_SIZE 2048

//Reserva en RAM del stack and task control block (TCB) para la tarea creada estática del display.
static StackType_t stackTaskDisplay[STACK_SIZE];
static StaticTask_t TCBtaskDisplay;

//Declaramos el handle (un puntero) para la task que creamos por si queremos suspenderla desde otras tasks.
TaskHandle_t xHandleDisplay = NULL;


//Función asignada a la tare del display (lo que va a hacer la tarea del display creada).
//Comentar el uso de static en la declaración de una función.
static void display_tarea_escribir(void *pvParameters); 

//Creamos la tarea estática para controlar el display.
void display_crear_task(void)
{
    xHandleDisplay = xTaskCreateStatic(
    &display_tarea_escribir,    //Función que se asigna a al tarea.
    "tareaDisplay",             //Nombre que le asignamos a la tarea (debug). 
    STACK_SIZE,                 //Tamaño del stack de la tarea (reservado de manera estática previamente).
    NULL,                       //Parámetros pasados a la tarea en su creación.
    tskIDLE_PRIORITY + 1,       //Prioridad de la tarea. 
    stackTaskDisplay,           //Array para el stack de la tarea reservado previamente. 
    &TCBtaskDisplay);           //Memoria reservada previamente para el TCB (task control block) de la tarea.

    printf("Tareas para escribie en el display creada.\n"); 
}

/*
Esta función lee la cola de temperatura y escribe el valor leído en el display.
Está lectura puede ocurrir de manera periódica cada 1 segundo que configuramos, o mediante el uso de un semáforo.
Para usar el semáforo añadir #define SEMAFORO, para que se ejecute de manera periódica comentar el define anterior.
Ejemplo para ver la sincronización de tareas con un semáforo.
*/
static void display_tarea_escribir(void *pvParameters)
{
    //Configuramos el periférico de I2C del ESP32. Se define una estructura que contiene la configuración del periférico.
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = (gpio_num_t)I2C_MASTER_SDA_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = (gpio_num_t)I2C_MASTER_SCL_IO;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    conf.clk_flags = I2C_SCLK_SRC_FLAG_FOR_NOMAL;

    //Funciones para configurar e inicializar el periférico.
    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);

    //Configuramos el display que vamos a usar según los ejemplos del driver que hemos instalado.
    ssd1306_dev = ssd1306_create(I2C_MASTER_NUM, SSD1306_I2C_ADDRESS);

    //Escribimos en mensaje "Temperatura" en el display.
    ssd1306_refresh_gram(ssd1306_dev);
    ssd1306_clear_screen(ssd1306_dev, 0x00);
    char data_str[] = "Temperatura";
    ssd1306_draw_string(ssd1306_dev, 16, 0, (const uint8_t *)data_str, 16, 1);   
    ssd1306_refresh_gram(ssd1306_dev);
    
    float temperatura = 0;

#define SEMAFORO //Comentamos esta línea para cambiar el funcionamiento del ejemplo.
#ifdef SEMAFORO
    while(1)
    {
        //Esperamos a que el semaforo de la cola de temperatura esté libre, si no lo está la tarea se bloquea
        //el tiempo que especifiquemos hasta que el semáforo esté libre.
        //portMAX_DELAY es el retardo máximo (días de duración).
        if(xSemaphoreTake(colasTempSemaforoHandle, portMAX_DELAY))
        {
            //Si hay elementos en la cola de temperaturas se entra en el if, y se lee un elemento de la cola en &temperatura.
            if(xQueueReceive(colasTempHandle,&temperatura,(TickType_t)0) == pdPASS)
            {
                printf("Elemento leido de la cola de temperatura: %.1f por la tarea del display.\n\n", temperatura); 
            }
            //Si la cola de temperaturas está vacía entramos en el else.
            else
            {
                printf("Cola de temperatura vacia.\n");
            }
            //Escribimos el valor de temperatura leído de la cola en el LCD (en el ejemplo solo contemplamos valores enteros positivos).
            ssd1306_draw_num(ssd1306_dev, 40, 40, (uint32_t)temperatura, 3, 16);
            ssd1306_refresh_gram(ssd1306_dev);
        }
    }
#else
    //En este ejemplo no usamos el semáforo para leer la cola de temperatura, si no que lo hacemos una vez por segundo.
    //Vamos a leer la cola de temperatura más rápido de lo que la tarea del sensor escribe en dicha cola.
    while(1)
    {
        //Si hay elementos en la cola de temperaturas se entra en el if, y se lee un elemento de la cola en &temperatura.
        if(xQueueReceive(colasTempHandle,&temperatura,(TickType_t)0) == pdPASS )
        {
            printf("Elemeno leido de la cola de temperatura: %.1f.\n\n", temperatura); 
        }
        //Si la cola de temperaturas está vacía entramos en el else.
        else
        {
            printf("Cola de temperatura vacia.\n");
        }
        //Escribimos el valor de temperatura leído de la cola en el LCD (en el ejemplo solo contemplamos valores enteros positivos).
        ssd1306_draw_num(ssd1306_dev, 40, 40, (uint32_t)temperatura, 3, 16);
        ssd1306_refresh_gram(ssd1306_dev);
        //Está tarea se va a ejecutar una vez por segundo.
        vTaskDelay(1000/ portTICK_PERIOD_MS);
    }     
#endif
}