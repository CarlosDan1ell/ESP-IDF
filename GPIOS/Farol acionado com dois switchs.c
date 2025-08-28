#include <stdio.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#define LED_1 GPIO_NUM_25 //vermelho
#define LED_2 GPIO_NUM_26 //amarelo
#define LED_3 GPIO_NUM_27 //verde
#define BOTTON_1 GPIO_NUM_16
#define BOTTON_2 GPIO_NUM_22
#define GPIO_OUTPUT_PIN_SEL ((1ULL<<LED_1)|(1ULL<<LED_2)|(1ULL<<LED_3))
#define GPIO_INPUT_PIN_SEL ((1ULL<<BOTTON_1)|(1ULL<<BOTTON_2))

void Task_LED(void *pvParameter){
    int cnt = 0;
    //Configura o descritor de Outputs(LED)
    gpio_config_t io_conf; //Declara a variável descritora do drive de GPIO.

    io_conf.intr_type =  GPIO_INTR_DISABLE; //Desabilita o recurso de interrupção neste descritor
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    //io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);

    //Configura o descritor de Inputs(botões)
    io_conf.intr_type =  GPIO_INTR_DISABLE; //Desabilita o recurso de interrupção neste descritor
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);
     
    while(1){
       
        if ((gpio_get_level(BOTTON_1)==0)||(gpio_get_level(BOTTON_2)==0))
        {
            
            gpio_set_level( LED_3,cnt%2);
            vTaskDelay(50/portTICK_PERIOD_MS);
            gpio_set_level( LED_2,cnt%2);
            vTaskDelay(100/portTICK_PERIOD_MS);
            gpio_set_level( LED_1,cnt%2);
            vTaskDelay(150/portTICK_PERIOD_MS);
            cnt++;
            
        }

    }
}

void app_main(void){
    xTaskCreate(Task_LED,"Task_LED",2048,NULL,1,NULL);
}
