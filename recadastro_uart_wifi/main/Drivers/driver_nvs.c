#include <stdio.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "driver_defines.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/uart.h"

/* char wifi_ssid[1048] = SSID;
char wifi_password[1048] = SENHA; */

/* size_t len = sizeof(wifi_ssid);
size_t len2 = sizeof(wifi_password); */

// static const char *TAG = "NVS"
uint8_t UartBufferSENHA[BUF_SIZE];
uint8_t UartBufferSSID[BUF_SIZE];
char get_wifi_ssid[BUF_SIZE];
char get_wifi_senha[BUF_SIZE];
size_t len = sizeof(get_wifi_ssid);
size_t len2 = sizeof(get_wifi_senha);

volatile char IsReceiveUART = 0;
volatile char IsBufferSSID = 0;
volatile char IsBufferSENHA = 0;
volatile char ssid_updated = 0;
volatile char password_updated = 0;
volatile char IsWifiReconfigure = 0;

void nvs_driver(void *pvParameters)
{

    esp_err_t ret;
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        // Verificamos se a partição NVS foi truncada e precisamos limpar
        // tentar nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    // Open
    printf("ABRINDO...");
    //nvs_handle_t my_handle;                                  // cria o handle pra endereçar no nvs
    err = nvs_open("wifi_conf", NVS_READWRITE, &my_handle); // Abrindo a "sala" para ler e escrever oq tem eu quiser

    if (err != ESP_OK)
    {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
    else
    {
        printf("its doneeee\n");

        // Read
        printf("Iniciando a leitura dos dados :\n");
        err = nvs_get_str(my_handle, KEY1, get_wifi_ssid, &len);
        ret = nvs_get_str(my_handle, KEY2, get_wifi_senha, &len2);

        switch (err)
        {
        case ESP_OK:
            /* printf("wifi_ssid = %s\n", (char *)get_wifi_ssid); */
            char *msg11 = "A SSID SALVA NA NVS É :\n";
            uart_write_bytes(ECHO_UART_PORT_NUM, msg11, strlen(msg11));
            uart_write_bytes(ECHO_UART_PORT_NUM, get_wifi_ssid, strlen((char *)get_wifi_ssid));

            break;
        case ESP_ERR_NVS_NOT_FOUND:
            printf("O valor do SSID não foi inicializado!\n");
            err = nvs_set_str(my_handle, KEY1, DEFAULT_SSID);

            if (err == ESP_OK)
            {
                err = nvs_commit(my_handle); // salva permanentemente
                printf("Valor do SSID salvo na NVS!\n");
            }

            break;
        default:
            printf("Error (%s) reading WIFI_SSID!\n", esp_err_to_name(err));
        }

        switch (ret)
        {
        case ESP_OK:
            // printf("wifi_password = %s\n", (char *)get_wifi_senha);
            char *msg11 = "A SENHA SALVA NA NVS É :\n";
            uart_write_bytes(ECHO_UART_PORT_NUM, msg11, strlen(msg11));
            uart_write_bytes(ECHO_UART_PORT_NUM, get_wifi_senha, strlen((char *)get_wifi_senha));
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            printf("O valor da PASSWORD não foi inicializado!\n");
            printf("escrevendo PASSWORD...\n");
            ret = nvs_set_str(my_handle, "wifi_password", DEFAULT_SENHA);
            if (ret == ESP_OK)
            {
                ret = nvs_commit(my_handle); // salva permanentemente
                printf("Valor da PASSWORD salva na NVS!\n");
            }
            break;
        default:
            printf("Error (%s) reading WIFI_SSID!\n", esp_err_to_name(ret));
        }

        // aguardando interrupção gerada pela uart...
        while (1)
        {
            if (IsReceiveUART == 1)
            {
                IsReceiveUART = 0;
                if (IsBufferSSID == 1)
                {
                    ret = nvs_set_str(my_handle, KEY1, (char *)UartBufferSSID);
                    if (ret == ESP_OK)
                    {
                        ret = nvs_commit(my_handle); // salva permanentemente
                                                     /*  printf("Recadastro do SSID salvo na NVS!\n"); */
                        char *msg4 = "NOVO SSID COMMITADO\n";
                        uart_write_bytes(ECHO_UART_PORT_NUM, msg4, strlen(msg4));
                        uart_write_bytes(ECHO_UART_PORT_NUM, UartBufferSSID, strlen((char*)UartBufferSSID));
                        ssid_updated = 1;
                    }
                    else
                    {
                        char *msg7 = "Entrou no Else erro SSID\n";
                        uart_write_bytes(ECHO_UART_PORT_NUM, msg7, strlen(msg7));
                    }
                    IsBufferSSID = 0;
                }
                if (IsBufferSENHA == 1)
                {
                    ret = nvs_set_str(my_handle, KEY2, (char *)UartBufferSENHA);
                    if (ret == ESP_OK)
                    {
                        ret = nvs_commit(my_handle); // salva permanentemente
                        char *msg5 = "NOVA SENHA COMMITADA\n";
                        uart_write_bytes(ECHO_UART_PORT_NUM, msg5, strlen(msg5)); 
                        uart_write_bytes(ECHO_UART_PORT_NUM, UartBufferSENHA, strlen((char*)UartBufferSENHA));
                        password_updated = 1;
                    }
                    else
                    {
                        char *msg6 = "Entrou no Else erro SENHA\n";
                        uart_write_bytes(ECHO_UART_PORT_NUM, msg6, strlen(msg6));
                    }
                    IsBufferSENHA = 0;
                }
                if(ssid_updated == 1 && password_updated == 1){
                    ssid_updated = 0;
                    password_updated = 0;
                    IsWifiReconfigure = 1;
                    char *msg7 = "SSID e PASSWORD ATUALIZADOS\n";
                    uart_write_bytes(ECHO_UART_PORT_NUM, msg7, strlen(msg7));

                }
            }
            // else
            // {
            //     char *msg9 = "AGUARDANDO FLAG\n";
            //     uart_write_bytes(ECHO_UART_PORT_NUM, msg9, strlen(msg9));
            //     vTaskDelay(100);
            // }
        }
    }
    // aguardando novos dados pela uart
}

/* void app_main(void)
{
    xTaskCreate(nvs_driver, "nvs_driver", 2048, NULL, 1, NULL);
    xTaskCreate(uart_read_task, "uart_echo_task", ECHO_TASK_STACK_SIZE, NULL, 1, NULL);
} */