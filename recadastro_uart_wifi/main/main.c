#include <stdio.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "Drivers/driver_defines.h"
#include "Drivers/driver_WiFi.h"
#include "nvs_flash.h"


void app_main(void)
{   
    xTaskCreate(nvs_driver,"uart_read_task",2048,NULL,1,NULL);
    xTaskCreate(uart_read_task,"uart_read_task",2048,NULL,1,NULL);
    wifi_init_sta();
    while(1) vTaskDelay(1);
}