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
uint8_t UartBufferBROKER[BUF_SIZE];
uint8_t UartBufferPUBLISH[BUF_SIZE];
uint8_t UartBufferSUBSCRIBE[BUF_SIZE];
char get_wifi_ssid[BUF_SIZE];
char get_wifi_senha[BUF_SIZE];
char get_broker_mqtt[BUF_SIZE];
char get_publish_mqtt[BUF_SIZE];
char get_sub_mqtt[BUF_SIZE];
size_t len = sizeof(get_wifi_ssid);
size_t len2 = sizeof(get_wifi_senha);
size_t len3 = sizeof(get_broker_mqtt);
size_t len4 = sizeof(get_publish_mqtt);
size_t len5 = sizeof(get_sub_mqtt);

volatile char IsReceiveUART = 0;
volatile char IsBufferSSID = 0;
volatile char IsBufferSENHA = 0;
volatile char ssid_updated = 0;
volatile char password_updated = 0;
volatile char IsWifiReconfigure = 0;
volatile char IsBrokerReconfigure = 0;
volatile char IsBufferBroker = 0;
volatile char IsBufferSubscribe = 0;
volatile char IsBufferPublish = 0;
volatile char IsPublishReconfigure = 0;
volatile char IsSubReconfigure = 0;

static const char *TAG = "NVS";

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
    err = nvs_open("Network_confg", NVS_READWRITE, &my_handle); // Abrindo a "sala" para ler e escrever oq tem eu quiser

    if (err != ESP_OK)
    {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
    else
    {
        printf("its doneeee\n");

        // Read
        printf("Iniciando a leitura dos dados de WIFI:\n");
        err = nvs_get_str(my_handle, KEY1, get_wifi_ssid, &len);
        ret = nvs_get_str(my_handle, KEY2, get_wifi_senha, &len2);
        esp_err_t ret2 = nvs_get_str(my_handle, KEY3, get_broker_mqtt, &len3);
        esp_err_t ret3 = nvs_get_str(my_handle,KEY4,get_publish_mqtt,&len4);
        esp_err_t ret4 = nvs_get_str(my_handle,KEY5,get_sub_mqtt,&len5);
        // esp_err_t ret3 = nvs_get_str(my_handle, KEY3, get_publish_mqtt, &len4);

        switch (err)
        {
        case ESP_OK:
            /* printf("wifi_ssid = %s\n", (char *)get_wifi_ssid); */
            ESP_LOGI(TAG,"O SSID SALVO NA NVS É : %s",get_wifi_ssid);
            /* char *msg11 = "A SSID SALVA NA NVS É :\n";
            uart_write_bytes(ECHO_UART_PORT_NUM, msg11, strlen(msg11));
            uart_write_bytes(ECHO_UART_PORT_NUM, get_wifi_ssid, strlen((char *)get_wifi_ssid)); */

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
            ESP_LOGI(TAG,"A SENHA SALVA NA NVS É : %s",get_wifi_senha);
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
          
        
        switch (ret2) // verificação do Broker
        {
        case ESP_OK:
            ESP_LOGI(TAG,"O BROKER SALVO NA NVS É : %s",get_broker_mqtt);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            printf("O valor do BROKER não foi inicializado!\n");
            printf("escrevendo BROKER...\n");
            ret2 = nvs_set_str(my_handle, KEY3, DEFAULT_BROKER);
            if (ret2 == ESP_OK)
            {
                ret2 = nvs_commit(my_handle); // salva permanentemente
                printf("Valor do BROKER salvo na NVS!\n");
            }
            break;
        default:
            printf("Error (%s) reading BROKER_MQTT!\n", esp_err_to_name(ret2));
        }
        

        switch (ret3) // verificação do PUB
        {
        case ESP_OK:
            ESP_LOGI(TAG,"O PUBLISH SALVO NA NVS É : %s",get_publish_mqtt);
        
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            printf("O valor do publish não foi inicializado!\n");
            printf("escrevendo publish...\n");
            ret2 = nvs_set_str(my_handle, KEY4, DEFAULT_PUBLISH);
            if (ret2 == ESP_OK)
            {
                ret2 = nvs_commit(my_handle); // salva permanentemente
                printf("Valor do publish salvo na NVS!\n");
            }
            break;
        default:
            printf("Error (%s) reading PUBLISH_MQTT!\n", esp_err_to_name(ret3));
        }

        switch (ret4) // verificação do SUB
        {
        case ESP_OK:

            ESP_LOGI(TAG,"O SUB SALVO NA NVS É : %s",get_sub_mqtt);
            
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            printf("O valor do SUB não foi inicializado!\n");
            printf("escrevendo SUB...\n");
            ret4 = nvs_set_str(my_handle, KEY5, DEFAULT_SUBSCRIBE);
            if (ret4 == ESP_OK)
            {
                ret4 = nvs_commit(my_handle); // salva permanentemente
                printf("Valor do sub salvo na NVS!\n");
            }
            break;
        default:
            printf("Error (%s) reading SUB_MQTT!\n", esp_err_to_name(ret4));
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
                if (IsBufferBroker == 1){
                    ret = nvs_set_str(my_handle, KEY3, (char *)UartBufferBROKER);
                    ESP_LOGI("DEBUG", "Novo broker setado: %s", UartBufferBROKER);
                    if (ret == ESP_OK)
                    {
                        
                        ret = nvs_commit(my_handle); // salva permanentemente
                        /* char *msg55 = "NOVA SENHA COMMITADA\n";
                        uart_write_bytes(ECHO_UART_PORT_NUM, msg5, strlen(msg5)); 
                        uart_write_bytes(ECHO_UART_PORT_NUM, UartBufferSENHA, strlen((char*)UartBufferSENHA)); */
                        err = nvs_get_str(my_handle,KEY3,get_broker_mqtt,&len3);
                        if(err == ESP_OK){
                            ESP_LOGW("DEBUG NVS", "Novo broker salvo: %s", get_broker_mqtt);
                        }
                        /* ESP_LOGW("DEBUG NVS", "Novo broker salvo: %s", get_broker_mqtt); */
                        IsBrokerReconfigure = 1;
                        // printf("o IsBrokerReconfigure vale : %d\n", IsBrokerReconfigure);
                        ESP_LOGW("DEBUG NVS", "o IsBrokerReconfigure vale : %d", IsBrokerReconfigure);
                        // nvs_close(my_handle);
                    }
                    else
                    {
                       printf("Novo broker recebido e não foi commitado");
                    }
                    IsBufferBroker = 0;
                }
                if (IsBufferPublish == 1){
                    ret = nvs_set_str(my_handle, KEY4, (char *)UartBufferPUBLISH);
                    if (ret == ESP_OK)
                    {
                        
                        ret = nvs_commit(my_handle); // salva permanentemente
                        /* char *msg55 = "NOVA SENHA COMMITADA\n";
                        uart_write_bytes(ECHO_UART_PORT_NUM, msg5, strlen(msg5)); 
                        uart_write_bytes(ECHO_UART_PORT_NUM, UartBufferSENHA, strlen((char*)UartBufferSENHA)); */
                        printf("Novo Publish recebido e commitado\n");
                        /* printf("o IsBrokerReconfigure vale : %d\n", IsBrokerReconfigure); */
                        IsPublishReconfigure = 1;
                    }
                    else
                    {
                       printf("Novo pub recebido e não foi commitado");
                    }
                    IsBufferPublish = 0;
                }
                if (IsBufferSubscribe == 1){
                    ret = nvs_set_str(my_handle, KEY5, (char *)UartBufferSUBSCRIBE);
                    if (ret == ESP_OK)
                    {
                        
                        ret = nvs_commit(my_handle); // salva permanentemente
                        /* char *msg55 = "NOVA SENHA COMMITADA\n";
                        uart_write_bytes(ECHO_UART_PORT_NUM, msg5, strlen(msg5)); 
                        uart_write_bytes(ECHO_UART_PORT_NUM, UartBufferSENHA, strlen((char*)UartBufferSENHA)); */
                        printf("Novo sub recebido e commitado\n");
                        /* printf("o IsBrokerReconfigure vale : %d\n", IsBrokerReconfigure); */
                        IsSubReconfigure = 1;
                    }
                    else
                    {
                       printf("Novo sub recebido e não foi commitado");
                    }
                    IsBufferSubscribe = 0;
                }
                
            }
            vTaskDelay(10);
            
        }
        
    }
    // aguardando novos dados pela uart
}
