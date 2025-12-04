#include <stdio.h>
#include <string.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "nvs_flash.h"
#include "defines.h"
#include "esp_log.h"
#include "defines.h"

#define BOTTON_1 GPIO_NUM_25
#define GPIO_INPUT_PIN_SEL ((1ULL<<BOTTON_1))

uint8_t estadoBotao;
static const char *TAG = "main";

void init_gpio(void){
    gpio_config_t io_conf;

    //Configura o descritor de Inputs(botões)
    io_conf.intr_type =  GPIO_INTR_DISABLE; //Desabilita o recurso de interrupção neste descritor
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);
     
}

// void task_botao(void *pvParameters)
// {

//     while(1)
//     {
//         if(gpio_get_level(BOTTON_1) == 0){
//         estadoBotao = 0;
//         ESP_LOGI(TAG,"valor atual do botao %d",estadoBotao);
        
//         }
//         else{
//             // strcpy((char*)estadoBotao,"HIGH");
//             estadoBotao = 1;
//             ESP_LOGI(TAG,"valor atual do botao %d",estadoBotao);
//         }

//         vTaskDelay(pdMS_TO_TICKS(1000)); // 100 ms
//     }

// }
void task_botao(void *pvParameters)
{
    example_espnow_send_param_t *send_param = (example_espnow_send_param_t *)pvParameters;

    if (send_param == NULL || send_param->buffer == NULL) {
        ESP_LOGE(TAG, "send_param invalido ao iniciar task!");
        vTaskDelete(NULL);
        return;
    }

    uint8_t estado_anterior = 0xFF;

    while (1)
    {
        estadoBotao = gpio_get_level(BOTTON_1) ? 1 : 0;
        // ESP_LOGI(TAG, "valor atual do botao %d", estadoBotao);

        if (estadoBotao != estado_anterior)
        {
            example_espnow_data_prepare(send_param);

            esp_now_send(send_param->dest_mac,
                         send_param->buffer,
                         send_param->len);

            ESP_LOGI(TAG, "Estado enviado via ESPNOW");
            estado_anterior = estadoBotao;
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}



uint8_t get_estado(void){
   
    return estadoBotao;
}