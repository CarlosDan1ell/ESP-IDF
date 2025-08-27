#include <stdio.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#define LED GPIO_NUM_25
#define BUTTON GPIO_NUM_16


void Task_LED(void *pvParameter){
    int cnt = 0;
    gpio_set_direction(BUTTON,GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON,GPIO_PULLUP_ONLY);
    gpio_set_direction(LED,GPIO_MODE_OUTPUT);
    printf("Piscando o led com o pino 25\n");

    while(1){
        if (gpio_get_level(BUTTON)==0)
        {
            printf("%d\n", cnt%2);
            gpio_set_level(LED,cnt%2);
            cnt++;
        }
        
        vTaskDelay(300/portTICK_PERIOD_MS);

    }
}

void app_main(void){
    xTaskCreate(Task_LED,"Task_LED",2048,NULL,1,NULL);
}
