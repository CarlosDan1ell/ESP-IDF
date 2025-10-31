#include <stdio.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "mqtt_client.h"
#include "mqtt.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_mac.h"
#include "WiFi.h"

#define LED_1 GPIO_NUM_21 //vermelho
#define GPIO_OUTPUT_PIN_SEL ((1ULL<<LED_1))


volatile uint8_t IsBufferMSGReady = 0;
char BufferMSG[BufferMSG_len];

static const char *TAG = "MAIN";



void initGPIOS(void){
    gpio_config_t io_conf; //Declara a variável descritora do drive de GPIO.
    io_conf.intr_type =  GPIO_INTR_DISABLE; //Desabilita o recurso de interrupção neste descritor
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    //io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);  
}

static void Task_Acionamento(void *pvParameters)
{
  while(1){
    if(IsBufferMSGReady == 1){
      ESP_LOGI(TAG,"FLAG DETECTADA");
      IsBufferMSGReady = 0;
      if(strcmp(BufferMSG, "ON") == 0){
        gpio_set_level(LED_1, 1);
      }
      else if(strcmp(BufferMSG, "OFF") == 0){
        gpio_set_level(LED_1, 0);
      }
      else{
        printf("Comando Invalido \n");
      }
    }
  }
}

void app_main(void){
  initGPIOS();
  wifi_init_sta();
  mqtt_app_start();
  ESP_LOGI(TAG,"Inicializações Concluídas. Criando a task...");

  xTaskCreate(
        Task_Acionamento,         
        "Task de Acionamento",   
        2048,                     
        NULL,                     
        5,                       
        NULL                      
    );
    
    
}