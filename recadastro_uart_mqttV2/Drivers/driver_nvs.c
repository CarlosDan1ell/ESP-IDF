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
#include "esp_mac.h"

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
char get_unique_mqtt[BUF_SIZE];
char aleatorio[BUF_SIZE];
size_t len = sizeof(aleatorio);
size_t len2 = sizeof(get_wifi_senha);
size_t len3 = sizeof(get_broker_mqtt);
size_t len4 = sizeof(get_publish_mqtt);
size_t len5 = sizeof(get_sub_mqtt);
size_t len6 = sizeof(get_unique_mqtt);

cJSON* config_json;

// volatile char IsReceiveUART = 0;
// volatile char IsBufferSSID = 0;
// volatile char IsBufferSENHA = 0;
// volatile char ssid_updated = 0;
// volatile char password_updated = 0;
// volatile char IsWifiReconfigure = 0;
// volatile char IsBrokerReconfigure = 0;
// volatile char IsBufferBroker = 0;
// volatile char IsBufferSubscribe = 0;
// volatile char IsBufferPublish = 0;
// volatile char IsPublishReconfigure = 0;
// volatile char IsSubReconfigure = 0;



static const char *TAG = "NVS";

void nvs_driver(void)
{

    // esp_err_t ret;
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
    //printf("ABRINDO...");
    //nvs_handle_t my_handle;                                  // cria o handle pra endereçar no nvs

    err = nvs_open("storage", NVS_READWRITE, &my_handle); // Abrindo a "sala" para ler e escrever oq tem eu quiser

    if (err != ESP_OK)
    {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
    else
    {
       
        // Read
        // printf("Iniciando a leitura dos dados de WIFI:\n");
        err = nvs_get_str(my_handle, "json", aleatorio, &len);
        switch (err)
        {
            case ESP_OK:
                ESP_LOGI(TAG, "O JSON SALVO NA NVS É : %s", aleatorio);
                config_json = cJSON_Parse(aleatorio);

                break;
            case ESP_ERR_NVS_NOT_FOUND:

                ESP_LOGI(TAG,"Valor do JSON salvo na NVS");
                uint8_t bt_mac[6];
                esp_read_mac(bt_mac, ESP_MAC_BT); // lê o MAC Bluetooth

                char unique_id[20];
                sprintf(unique_id, "%02X:%02X:%02X:%02X:%02X:%02X",
                                    bt_mac[0], bt_mac[1], bt_mac[2], bt_mac[3], bt_mac[4], bt_mac[5]);

                ESP_LOGI("UNIQUE_ID", "ID gerado: %s", unique_id);

                cJSON *nvs_json = cJSON_CreateObject();
                cJSON_AddStringToObject(nvs_json,"ssid",DEFAULT_SSID);
                cJSON_AddStringToObject(nvs_json,"password",DEFAULT_SENHA);
                cJSON_AddStringToObject(nvs_json,"broker_address",DEFAULT_BROKER);
                cJSON_AddStringToObject(nvs_json,"publish_topic",DEFAULT_PUBLISH);
                cJSON_AddStringToObject(nvs_json,"subscribe_topic",DEFAULT_SUBSCRIBE);
                cJSON_AddStringToObject(nvs_json,"unique_id",unique_id);
                char *json_print = cJSON_PrintUnformatted(nvs_json);
                cJSON_Delete(nvs_json);


                err = nvs_set_str(my_handle,"json", json_print);

                if (err == ESP_OK)
                {
                    err = nvs_commit(my_handle); // salva permanentemente
                    ESP_LOGI(TAG,"Valor do JSON salvo na NVS : %s\n",json_print);
                }

                break;
            default:
                ESP_LOGE(TAG,"Error (%s) reading JSON!\n", esp_err_to_name(err));
        }

    }

}

esp_err_t write_nvs(const char *input, const char *KEY)
{
    esp_err_t err = nvs_set_str(my_handle, KEY, input);
    if (err == ESP_OK)
    {
        esp_err_t ret = nvs_commit(my_handle);
        if (ret == ESP_OK)
        {
            ESP_LOGI(TAG, "COMMIT !!");
        }
        else
            return ESP_FAIL;
    }
    else
        return ESP_FAIL;

    return ESP_OK;
}

cJSON* get_json(void){
    
    return config_json;
}
