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

        // para data[0] = 'x', vamos alocar ela como SENHA
        // para data[0] = 'y', vamos alocar ela como SSID
        if (len) {
            data[len] = '\0'; //determina o fim dos caracteres
            if(data[0] == 'x'){
                //uint8_t UartBufferSENHA[BUF_SIZE] = data;
                strcpy((char*)UartBufferSENHA,(char*)data + 1); //mudar para strlen
                /* char *msg1 = "Nova SENHA recebida\n";
                uart_write_bytes(ECHO_UART_PORT_NUM, msg1, strlen(msg1)); */
                ESP_LOGI(TAG,"SENHA NOVA RECEBIDA = %s",UartBufferSENHA);
                IsBufferSENHA = 1;
                
            }
            else if(data[0] == 'y') {
                /* uint8_t UartBufferSSID[BUF_SIZE] = data; */
                strcpy((char*)UartBufferSSID,(char*)data + 1);
                char *msg = "Novo SSID recebido\n";
                uart_write_bytes(ECHO_UART_PORT_NUM, msg, strlen(msg));
                IsBufferSSID = 1;
            }
            else if(data[0] == 'b'){
                strcpy((char*)UartBufferBROKER,(char*)data + 1);
                printf("Novo BROKER recebido\n");
                /* char *msgB = "Novo BROKER recebido\n";
                uart_write_bytes(ECHO_UART_PORT_NUM, msgB, strlen(msgB)); */
                IsBufferBroker = 1;
                printf("o IsBufferBroker vale : %d\n", IsBufferBroker);
                ESP_LOGI("DEBUG", "Endereco da flag IsBrokerReconfigure NVS: %p", &IsBrokerReconfigure);

            }
            else if(data[0] == 'p'){
                strcpy((char*)UartBufferPUBLISH,(char*)data + 1);
                char *msgP = "Novo PUBLISH recebido\n";
                uart_write_bytes(ECHO_UART_PORT_NUM, msgP, strlen(msgP));
                IsBufferPublish = 1;

            }
            else if(data[0] == 's'){
                strcpy((char*)UartBufferSUBSCRIBE,(char*)data + 1);
                char *msgS = "Novo PUBLISH recebido\n";
                uart_write_bytes(ECHO_UART_PORT_NUM, msgS, strlen(msgS));
                IsBufferSubscribe = 1;

            }
            else{
                /* printf("credenciais recebidas são Inválidas"); */
                char *msg2 = "credenciais recebidas sao Invalidas\n";
                uart_write_bytes(ECHO_UART_PORT_NUM, msg2, strlen(msg2));
            }
            IsReceiveUART = 1;
        }
        vTaskDelay(1);
    }
}



