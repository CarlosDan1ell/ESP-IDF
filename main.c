#include <stdio.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

void blink_task(void *pvParameter){
    
    gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);
    printf("Blinking LED on GPIO 2\n");
    int cnt = 0;
    while (1)
    {
        gpio_set_level(GPIO_NUM_2,cnt%2);
        cnt++;
        vTaskDelay(100/portTICK_PERIOD_MS);
    }
    

}
void app_main(void)
{
    xTaskCreate(blink_task,"blink_task",1024,NULL,5,NULL);
    printf("Blynk task started\n");

}