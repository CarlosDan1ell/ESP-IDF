#include <stdio.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#define LED_1 GPIO_NUM_25 //vermelho
#define LED_2 GPIO_NUM_26 //amarelo
#define LED_3 GPIO_NUM_27 //verde
#define GPIO_OUTPUT_PIN_SEL ((1ULL<<LED_1)|(1ULL<<LED_2)|(1ULL<<LED_3))

void Task_LED(void *pvParameter){
    int cnt = 0;
    gpio_config_t io_conf; //Declara a variável descritora do drive de GPIO.
    io_conf.intr_type =  GPIO_INTR_DISABLE; //Desabilita o recurso de interrupção neste descritor
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    //io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
     
    while(1){
       
        gpio_set_level( LED_3,cnt%2);
        vTaskDelay(50/portTICK_PERIOD_MS);
        gpio_set_level( LED_2,cnt%2);
        vTaskDelay(100/portTICK_PERIOD_MS);
        gpio_set_level( LED_1,cnt%2);
        vTaskDelay(150/portTICK_PERIOD_MS);
        cnt++;

    }
}

void app_main(void){
    xTaskCreate(Task_LED,"Task_LED",2048,NULL,1,NULL);
}