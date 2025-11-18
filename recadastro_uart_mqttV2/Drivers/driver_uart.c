/* UART Echo Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "driver_defines.h"

;

/**
 * This is an example which echos any data it receives on configured UART back to the sender,
 * with hardware flow control turned off. It does not use UART driver event queue.
 *
 * - Port: configured UART
 * - Receive (Rx) buffer: on
 * - Transmit (Tx) buffer: off
 * - Flow control: off
 * - Event queue: off
 * - Pin assignment: see defines below (See Kconfig)
 */


static const char *TAG = "UART TEST";
char responseBuffer[1024];


void uart_read_task(void *arg)
{
    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = ECHO_UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    int intr_alloc_flags = 0;

#if CONFIG_UART_ISR_IN_IRAM
    intr_alloc_flags = ESP_INTR_FLAG_IRAM;
#endif

    ESP_ERROR_CHECK(uart_driver_install(ECHO_UART_PORT_NUM, BUF_SIZE * 2, 0, 0, NULL, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(ECHO_UART_PORT_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(ECHO_UART_PORT_NUM, ECHO_TEST_TXD, ECHO_TEST_RXD, ECHO_TEST_RTS, ECHO_TEST_CTS));

    // Configure a temporary buffer for the incoming data
    uint8_t *data = (uint8_t *) malloc(BUF_SIZE);

    while (1) {

        // Read data from the UART
        int len = uart_read_bytes(ECHO_UART_PORT_NUM, data, (BUF_SIZE - 1), 20 / portTICK_PERIOD_MS);

        // Write data back to the UART
        //uart_write_bytes(ECHO_UART_PORT_NUM, (const char *) data, len);
        
        // if (len) {
            
        //     data[len] = '\0'; //determina o fim dos caracteres
        //     ESP_LOGI(TAG, "O DATA COM TERMINADOR %s", data);
        //     if(receive_msg((char*)data,len,responseBuffer) == 1){
        //         vTaskDelay(pdMS_TO_TICKS(3000));
        //         uart_write_bytes(ECHO_UART_PORT_NUM, responseBuffer, strlen(responseBuffer));
        //         esp_restart();
        //     }
        //     else{
        //         uart_write_bytes(ECHO_UART_PORT_NUM, responseBuffer, strlen(responseBuffer));
        //         ESP_LOGI(TAG,"Não precisa reiniciar");
        //     }
            
        // }
        // vTaskDelay(pdMS_TO_TICKS(10));
        if (len > 0) {
            data[len] = '\0';
            ESP_LOGI(TAG, "Recebido: %s", data);

            // Passa o JSON recebido para o parser
            // process_json_message((char *)data);
            if(receive_msg((char*)data,len,responseBuffer) == 1){
                vTaskDelay(pdMS_TO_TICKS(3000));
                uart_write_bytes(ECHO_UART_PORT_NUM, responseBuffer, strlen(responseBuffer));
                esp_restart();
            }

        }
    }
}



