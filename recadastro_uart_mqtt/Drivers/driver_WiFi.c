#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "driver_WiFi.h"
#include "driver_defines.h"
#include "driver/uart.h"

/* #define EXAMPLE_ESP_WIFI_SSID   "LSE"
#define EXAMPLE_ESP_WIFI_PASS   "HubLS3s2" */
#define EXAMPLE_ESP_MAXIMUM_RETRY 10

/* #if CONFIG_ESP_WPA3_SAE_PWE_HUNT_AND_PECK
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_HUNT_AND_PECK
#define EXAMPLE_H2E_IDENTIFIER ""
#elif CONFIG_ESP_WPA3_SAE_PWE_HASH_TO_ELEMENT
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_HASH_TO_ELEMENT
#define EXAMPLE_H2E_IDENTIFIER CONFIG_ESP_WIFI_PW_ID
#elif CONFIG_ESP_WPA3_SAE_PWE_BOTH
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_BOTH
#define EXAMPLE_H2E_IDENTIFIER CONFIG_ESP_WIFI_PW_ID
#endif
#if CONFIG_ESP_WIFI_AUTH_OPEN
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_OPEN
#elif CONFIG_ESP_WIFI_AUTH_WEP
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WEP
#elif CONFIG_ESP_WIFI_AUTH_WPA_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WAPI_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WAPI_PSK
#endif */

size_t len_ssid = sizeof(get_wifi_ssid);
size_t len_pass = sizeof(get_wifi_senha);
nvs_handle_t my_handle;

static EventGroupHandle_t s_wifi_event_group;

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

static const char *TAG = "wifi station";
static int s_retry_num = 0;

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();

    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {   
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY)
        {
            esp_wifi_connect();
            s_retry_num++;
            printf("\n retry to connect to the AP \n");
        }
        else
        {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_reconfig_task(void *pvParameters)
{
    wifi_config_t wifi_config = {0};
    while (1)
    {
        if (IsWifiReconfigure == 1)
        {
            // Lê novas credenciais do NVS
            size_t len_ssid = sizeof(get_wifi_ssid);
            size_t len_pass = sizeof(get_wifi_senha);
            esp_err_t err_ssid = nvs_get_str(my_handle, KEY1, get_wifi_ssid, &len_ssid);
            esp_err_t err_pass = nvs_get_str(my_handle, KEY2, get_wifi_senha, &len_pass);

            if (err_ssid == ESP_OK && err_pass == ESP_OK)
            {
                strcpy((char *)wifi_config.sta.ssid, get_wifi_ssid);
                strcpy((char *)wifi_config.sta.password, get_wifi_senha);

                esp_wifi_disconnect(); 
                vTaskDelay(pdMS_TO_TICKS(200));
                esp_wifi_stop();
                vTaskDelay(pdMS_TO_TICKS(500));
                esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
                esp_wifi_start();
                vTaskDelay(pdMS_TO_TICKS(500));
                esp_wifi_connect(); 

                char *msg = ">>> Wi-Fi reconfigurado com sucesso!\n";
                uart_write_bytes(ECHO_UART_PORT_NUM, msg, strlen(msg));
            }
            else
            {
                char *msg = ">>> Erro ao ler novas credenciais do NVS!\n";
                uart_write_bytes(ECHO_UART_PORT_NUM, msg, strlen(msg));
            }
            EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
            if (bits & WIFI_CONNECTED_BIT)
            {
                /*  ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",ssid,password); */
                char *wifi_msg2 = "Conectado ao WIFI\n";
                uart_write_bytes(ECHO_UART_PORT_NUM, wifi_msg2, strlen(wifi_msg2));
                uart_write_bytes(ECHO_UART_PORT_NUM, wifi_config.sta.ssid, strlen((char *)wifi_config.sta.ssid));
                uart_write_bytes(ECHO_UART_PORT_NUM, wifi_config.sta.password, strlen((char *)wifi_config.sta.password));
            }
            else if (bits & WIFI_FAIL_BIT)
            {
                char *wifi_msg3 = "Falha ao conectar no wifi\n";
                uart_write_bytes(ECHO_UART_PORT_NUM, wifi_msg3, strlen(wifi_msg3));
            }
            else
            {
                /*  ESP_LOGE(TAG,"UNEXPECTED EVENT"); */
                char *wifi_msg4 = "UNEXPECTED EVENT\n";
                uart_write_bytes(ECHO_UART_PORT_NUM, wifi_msg4, strlen(wifi_msg4));
            }


            IsWifiReconfigure = 0; // zera flag após reconfigurar
        }

        vTaskDelay(pdMS_TO_TICKS(1000)); // checa a flag a cada segundo
    }
}

void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip));

    wifi_config_t wifi_config = {0}; // inicializar ela com zero
    esp_err_t err_ssid = nvs_get_str(my_handle, KEY1, get_wifi_ssid, &len_ssid);
    esp_err_t err_pass = nvs_get_str(my_handle, KEY2, get_wifi_senha, &len_pass);

    if (err_ssid == ESP_OK && err_pass == ESP_OK)
    {
        strcpy((char *)wifi_config.sta.ssid, get_wifi_ssid);
        strcpy((char *)wifi_config.sta.password, get_wifi_senha);
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
        ESP_ERROR_CHECK(esp_wifi_start());
    }
    else
    {

        char *wifi_msg1 = "Erro na leitura do NVS\n";
        uart_write_bytes(ECHO_UART_PORT_NUM, wifi_msg1, strlen(wifi_msg1));
    }

    /* ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start()); */

    /* ESP_LOGI(TAG,"wifi_init_sta finished."); */
    char *wifi_msg5 = "wifi_init_sta finished.\n";
    uart_write_bytes(ECHO_UART_PORT_NUM, wifi_msg5, strlen(wifi_msg5));
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
    if (bits & WIFI_CONNECTED_BIT)
    {
        /*  ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",ssid,password); */
        char *wifi_msg2 = "Conectado ao WIFI\n";
        uart_write_bytes(ECHO_UART_PORT_NUM, wifi_msg2, strlen(wifi_msg2));
        uart_write_bytes(ECHO_UART_PORT_NUM, wifi_config.sta.ssid, strlen((char *)wifi_config.sta.ssid));
        uart_write_bytes(ECHO_UART_PORT_NUM, wifi_config.sta.password, strlen((char *)wifi_config.sta.password));
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        char *wifi_msg3 = "Falha ao conectar no wifi\n";
        uart_write_bytes(ECHO_UART_PORT_NUM, wifi_msg3, strlen(wifi_msg3));
    }
    else
    {
        /*  ESP_LOGE(TAG,"UNEXPECTED EVENT"); */
        char *wifi_msg4 = "UNEXPECTED EVENT\n";
        uart_write_bytes(ECHO_UART_PORT_NUM, wifi_msg4, strlen(wifi_msg4));
    }

    xTaskCreate(wifi_reconfig_task, "wifi_reconfig_task", 2048, NULL, 2, NULL);
}
