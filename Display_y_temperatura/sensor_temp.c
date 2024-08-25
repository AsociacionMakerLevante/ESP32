/*Documentación:
https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/adc_oneshot.html
https://docs.espressif.com/projects/esp-idf/en/latest/esp32c3/api-reference/peripherals/adc_calibration.html
https://www.freertos.org/Documentation/02-Kernel/04-API-references/10-Semaphore-and-Mutexes/00-Semaphores
https://www.freertos.org/Documentation/02-Kernel/04-API-references/01-Task-creation/02-xTaskCreateStatic
ADC conectado al PIN 3 = ADC1_CH3.
Datasheet del sensor: https://www.analog.com/media/en/technical-documentation/data-sheets/tmp35_36_37.pdf
*/
#include "sensor_temp.h"

//Tamaño del stack individual de las tareas.
#define STACK_SIZE_SENTEMP 2048
//Reserva en RAM del stack and task control block (TCB) para las tareas creadas estáticas del sensor de temperatura.
static StackType_t stackTaskSenTemp[STACK_SIZE_SENTEMP];
static StaticTask_t TCBtaskSenTemp;

//Declaramos los handlers para las tasks estáticas por si queremos suspenderlas desde otras tasks.
TaskHandle_t xHandleSenTemp = NULL;

//Función asignada a la tare del sensor de temperatura.
static void sensor_temp_leer_adc(void *pvParameters);

//Creamos una tarea estática para leer el sensor de temperatura.
void sensor_temperatura_crear_tarea(void)
{
    xHandleSenTemp = xTaskCreateStatic(
    &sensor_temp_leer_adc,      //Función que se asigna a al tarea.
    "tareaSensorT",             //Nombre que le asignamos a la tarea (debug). 
    STACK_SIZE_SENTEMP,         //Tamaño del stack de la tarea (reservado de manera estática previamente).
    NULL,                       //Parámetros pasados a la tarea en su creación.
    tskIDLE_PRIORITY + 1,       //Prioridad de la tarea. 
    stackTaskSenTemp,           //Array para el stack de la tarea reservado previamente. 
    &TCBtaskSenTemp);           //Memoria reservada previamente para el TCB (task control block) de la tarea.
    printf("Tarea para leer el sensor de temperatura creada.\n"); 
}

//Función asignada a la tarea que creamos para leer sensor de temperatura.
//Lee el ADC cada x tiempo y manda la lectura a una cola que la usará el display.
static void sensor_temp_leer_adc(void *pvParameters)
{
    int resultado_adc = 0;
    int conversion_resultado_adc = 0;
    float temperatura = 0;

    //Configuramos el ADC siguiendo la API del ESP32.
    //Crea un ADC Unit Handle en modo de un solo disparo.
    adc_oneshot_unit_handle_t adc1_handle;
    adc_oneshot_unit_init_cfg_t init_config1 = {
    .unit_id = ADC_UNIT_1,
    .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    //Configura el ADC para medir la señal analógica. 
    adc_oneshot_chan_cfg_t config = {
    .bitwidth = ADC_BITWIDTH_DEFAULT, //12 bits
    .atten = ADC_ATTEN_DB_2_5,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_3, &config));
    //ADC configurado, para leerlo usar adc_oneshot_read(); no usar dentro de una ISR.

    //Crear el esquema de calibración del ADC.
    adc_cali_handle_t adc1_cali_chan3_handle = NULL;
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .atten = ADC_ATTEN_DB_2_5,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_cali_create_scheme_curve_fitting(&cali_config, &adc1_cali_chan3_handle));
    //Llamar adc_cali_raw_to_voltage() para convertir la lectura del ADC a mV.

    while(1)
    {
        //Leemos el ADC.
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL_3, &resultado_adc));
        //Convertimos la lectura del ADC a mV (calibración).
        ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_chan3_handle, resultado_adc, &conversion_resultado_adc));
        //Calculamos la temperatura según la fórmula que tenemos en el datasheet del sensor de temperatura.
        temperatura = (conversion_resultado_adc - 500)/10.0f;
        //Mandamos la temperatura leída a la cola, si está llena se imprime el mensaje de cola llena.
        if(xQueueSend(colasTempHandle, (void *)&temperatura, (TickType_t)0) == pdPASS) 
        {
            printf("Elemento enviado a la cola de temperatura: %.1f\ por la tarea del sensor.\n", temperatura); 
        }
        else
        {
            printf("Cola de temperaturas llena.\n"); 
        }
        //Damos el semáforo de la cola de temperatura para que el display pueda cogerlo y leerla.
        xSemaphoreGive(colasTempSemaforoHandle);
        //Esta tarea se repite cada 5 segundos.
        vTaskDelay(5000 / portTICK_PERIOD_MS); 
    }
}