#include "driver_defines.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "driver/gpio.h"

#define LED_1 GPIO_NUM_2
#define GPIO_OUTPUT_PIN_SEL ((1ULL << LED_1))

static const char *TAG = "MSG_TASK";
cJSON *json_msg;

void starta_gpio(void){
    gpio_config_t io_conf = {
    .intr_type = GPIO_INTR_DISABLE,
    .mode = GPIO_MODE_OUTPUT,
    .pin_bit_mask = GPIO_OUTPUT_PIN_SEL,
   //.pull_down_en = GPIO_PULLDOWN_DISABLE,
   //.pull_up_en = GPIO_PULLUP_DISABLE,

    };

    gpio_config(&io_conf);
    ESP_LOGI(TAG,"GPIO INICIALIZADO");
}

uint8_t receive_msg(char *data, size_t len, char *response)
{
    // const cJSON *broker;
    // const cJSON *ssid;
    // const cJSON *publish;
    // const cJSON *senha;
    // const cJSON *subscribe;
    json_msg = get_json();

    uint8_t IsNeedReboot = 0;
    volatile char json_flag = 0;
    volatile char led_flag = 0;
    // gpio_config_t io_conf = {
    //     .intr_type = GPIO_INTR_DISABLE,
    //     .mode = GPIO_MODE_OUTPUT,
    //     .pin_bit_mask = GPIO_OUTPUT_PIN_SEL,
    //     //.pull_down_en = GPIO_PULLDOWN_DISABLE,
    //     //.pull_up_en = GPIO_PULLUP_DISABLE,

    // };

    // gpio_config(&io_conf);
    // Verificação de segurança
    if (len <= 0)
    {
        return IsNeedReboot;
    }
    // data[len] = '\0'; // determina o fim dos caracteres
    //ESP_LOGI(TAG, "O DATA COM TERMINADOR %s", data);
    if (data[0] == '{')
    {
        cJSON *json = cJSON_Parse(data);
        if (json == NULL)
        {
            ESP_LOGE(TAG, "Erro ao fazer parse do JSON");
            return IsNeedReboot;
        }

        if (cJSON_IsString(cJSON_GetObjectItemCaseSensitive(json, "LED")) && (cJSON_GetObjectItemCaseSensitive(json, "LED")->valuestring)){
            if(strcmp(cJSON_GetObjectItemCaseSensitive(json, "LED")->valuestring, "LIGAR")== 0){
                gpio_set_level(LED_1,1);
                ESP_LOGE(TAG,"LED LIGADO");
                strcpy(response,"LED LIGADO");
                led_flag = 1;
            }
            else if (strcmp(cJSON_GetObjectItemCaseSensitive(json, "LED")->valuestring, "DESLIGAR")== 0)
            {
                gpio_set_level(LED_1,0);
                ESP_LOGE(TAG,"LED DESLIGADO");
                strcpy(response,"LED DESLIGADO");
                led_flag = 1;
            }
            else{
                ESP_LOGE(TAG,"LED Comando invalido");
                strcpy(response,"LED Comando invalido");
                led_flag = 1;
            }
        }
        
        if (cJSON_IsString(cJSON_GetObjectItemCaseSensitive(json, "action")) && (cJSON_GetObjectItemCaseSensitive(json, "action")->valuestring))
        {
            cJSON *response_ptr = cJSON_CreateObject();
            cJSON *json_payload;
            cJSON_AddItemToObject(response_ptr, "payload", json_payload = cJSON_CreateObject());
            cJSON_AddStringToObject(response_ptr, "action", cJSON_GetObjectItemCaseSensitive(json, "action")->valuestring);
            cJSON *obj_payload = cJSON_GetObjectItemCaseSensitive(json, "payload");
            cJSON_AddStringToObject(json_payload, "type", cJSON_GetObjectItemCaseSensitive(cJSON_GetObjectItemCaseSensitive(json, "payload"), "type")->valuestring);
            cJSON_AddStringToObject(json_payload, "name", cJSON_GetObjectItemCaseSensitive(cJSON_GetObjectItemCaseSensitive(json, "payload"), "name")->valuestring);
            cJSON_AddStringToObject(json_payload, "request_id", cJSON_GetObjectItemCaseSensitive(cJSON_GetObjectItemCaseSensitive(json, "payload"), "request_id")->valuestring);

            if (strcmp(cJSON_GetObjectItemCaseSensitive(json, "action")->valuestring, "set_register") == 0)
            {
                if (cJSON_IsObject(obj_payload))
                {
                    cJSON *obj_data = cJSON_GetObjectItemCaseSensitive(obj_payload, "data"); // verifica se dentro de payload existe o objeto data
                    if (cJSON_IsObject(obj_data))
                    {
                        if (cJSON_IsString(cJSON_GetObjectItemCaseSensitive(obj_data, "broker_address")) && cJSON_GetObjectItemCaseSensitive(obj_data, "broker_address")->valuestring)
                        {
                            // ESP_LOGI(TAG, "BROKER do cJSON: %s", cJSON_GetObjectItemCaseSensitive(obj_data, "broker_address")->valuestring);
                            // if (write_nvs((char *)cJSON_GetObjectItemCaseSensitive(obj_data, "broker_address")->valuestring, KEY3) == ESP_OK)
                            // {
                            ESP_LOGI(TAG, "Novo BROKER recebido: %s", (char *)cJSON_GetObjectItemCaseSensitive(obj_data, "broker_address")->valuestring);
                            // }
                            // else
                            // {
                            //     ESP_LOGE(TAG, "Novo BROKER recebido falhou");
                            // }
                            cJSON_ReplaceItemInObject(json_msg, "broker_address", cJSON_CreateString(cJSON_GetObjectItemCaseSensitive(obj_data, "broker_address")->valuestring));
                            // char *json_print = cJSON_PrintUnformatted(json_msg);
                            // write_nvs(json_print, "json");
                            // free(json_print);
                            printa();
                        }
                        if (cJSON_IsString(cJSON_GetObjectItemCaseSensitive(obj_data, "ssid")) && cJSON_GetObjectItemCaseSensitive(obj_data, "ssid")->valuestring)
                        {

                            ESP_LOGI(TAG, "Novo SSID recebido: %s", (char *)cJSON_GetObjectItemCaseSensitive(obj_data, "ssid")->valuestring);
                          
                            cJSON_ReplaceItemInObject(json_msg, "ssid", cJSON_CreateString(cJSON_GetObjectItemCaseSensitive(obj_data, "ssid")->valuestring));
                            printa();
                        }
                        if (cJSON_IsString(cJSON_GetObjectItemCaseSensitive(obj_data, "publish_topic")) && cJSON_GetObjectItemCaseSensitive(obj_data, "publish_topic")->valuestring)
                        {
                            
                            ESP_LOGI(TAG, "Novo PUBLISH recebido: %s", (char *)cJSON_GetObjectItemCaseSensitive(obj_data, "publish_topic")->valuestring);
                            
                            cJSON_ReplaceItemInObject(json_msg, "publish_topic", cJSON_CreateString(cJSON_GetObjectItemCaseSensitive(obj_data, "publish_topic")->valuestring));
                            printa();
                        }
                        if (cJSON_IsString(cJSON_GetObjectItemCaseSensitive(obj_data, "subscribe_topic")) && cJSON_GetObjectItemCaseSensitive(obj_data, "subscribe_topic")->valuestring)
                        {
                        
                            ESP_LOGI(TAG, "Novo SUBSCRIBE recebido: %s", (char *)cJSON_GetObjectItemCaseSensitive(obj_data, "subscribe_topic")->valuestring);
                           
                            cJSON_ReplaceItemInObject(json_msg, "subscribe_topic", cJSON_CreateString(cJSON_GetObjectItemCaseSensitive(obj_data, "subscribe_topic")->valuestring));
                            printa();
                        }
                        if (cJSON_IsString(cJSON_GetObjectItemCaseSensitive(obj_data, "password")) && cJSON_GetObjectItemCaseSensitive(obj_data, "password")->valuestring)
                        {
                            
                            ESP_LOGI(TAG, "Nova SENHA recebida: %s", (char *)cJSON_GetObjectItemCaseSensitive(obj_data, "password")->valuestring);
                          
                            cJSON_ReplaceItemInObject(json_msg, "password", cJSON_CreateString(cJSON_GetObjectItemCaseSensitive(obj_data, "password")->valuestring));
                            printa();
                        }
                        if (cJSON_IsString(cJSON_GetObjectItemCaseSensitive(obj_data, "unique_id")) && cJSON_GetObjectItemCaseSensitive(obj_data, "unique_id")->valuestring)
                        {
                            
                            ESP_LOGI(TAG, "Nova unique_id recebida: %s", (char *)cJSON_GetObjectItemCaseSensitive(obj_data, "unique_id")->valuestring);
                           
                            cJSON_ReplaceItemInObject(json_msg, "unique_id", cJSON_CreateString(cJSON_GetObjectItemCaseSensitive(obj_data, "unique_id")->valuestring));
                            printa();
                        }
                    }
                    else
                    {
                        ESP_LOGE(TAG, "obj data erro !");
                    }
                }
                else
                {
                    ESP_LOGE(TAG, "obj payload erro !");
                }

            }

            else if (strcmp(cJSON_GetObjectItemCaseSensitive(json, "action")->valuestring, "get_register") == 0)
            {
                cJSON *json_data;
                cJSON_AddItemToObject(json_payload, "data", json_data = cJSON_CreateObject());
                cJSON_AddStringToObject(json_data, "unique_id", cJSON_GetObjectItemCaseSensitive(json_msg, "unique_id")->valuestring);
                cJSON_AddStringToObject(json_data, "broker_address", cJSON_GetObjectItemCaseSensitive(json_msg, "broker_address")->valuestring);
                cJSON_AddStringToObject(json_data, "ssid", cJSON_GetObjectItemCaseSensitive(json_msg, "ssid")->valuestring);
                cJSON_AddStringToObject(json_data, "publish_topic", cJSON_GetObjectItemCaseSensitive(json_msg, "publish_topic")->valuestring);
                cJSON_AddStringToObject(json_data, "subscribe_topic", cJSON_GetObjectItemCaseSensitive(json_msg, "subscribe_topic")->valuestring);
                cJSON_AddStringToObject(json_data, "password", cJSON_GetObjectItemCaseSensitive(json_msg, "password")->valuestring);

                if (cJSON_IsString(cJSON_GetObjectItemCaseSensitive(json_msg, "broker_address")) && cJSON_GetObjectItemCaseSensitive(json_msg, "broker_address")->valuestring)
                {
                    ESP_LOGI(TAG, "BROKER GET : %s", (char *)cJSON_GetObjectItemCaseSensitive(json_msg, "broker_address")->valuestring);
                    
                }
                if (cJSON_IsString(cJSON_GetObjectItemCaseSensitive(json_msg, "ssid")) && cJSON_GetObjectItemCaseSensitive(json_msg, "ssid")->valuestring)
                {

                    ESP_LOGI(TAG, "SSID GET: %s", (char *)cJSON_GetObjectItemCaseSensitive(json_msg, "ssid")->valuestring);
                 
                }
                if (cJSON_IsString(cJSON_GetObjectItemCaseSensitive(json_msg, "publish_topic")) && cJSON_GetObjectItemCaseSensitive(json_msg, "publish_topic")->valuestring)
                {
                    
                    ESP_LOGI(TAG, "PUBLISH GET: %s", (char *)cJSON_GetObjectItemCaseSensitive(json_msg, "publish_topic")->valuestring);
                    
                }
                if (cJSON_IsString(cJSON_GetObjectItemCaseSensitive(json_msg, "subscribe_topic")) && cJSON_GetObjectItemCaseSensitive(json_msg, "subscribe_topic")->valuestring)
                {
                    
                    ESP_LOGI(TAG, "SUBSCRIBE GET: %s", (char *)cJSON_GetObjectItemCaseSensitive(json_msg, "subscribe_topic")->valuestring);
                    
                }
                if (cJSON_IsString(cJSON_GetObjectItemCaseSensitive(json_msg, "password")) && cJSON_GetObjectItemCaseSensitive(json_msg, "password")->valuestring)
                {
                    
                    ESP_LOGI(TAG, "SENHA GET: %s", (char *)cJSON_GetObjectItemCaseSensitive(json_msg, "password")->valuestring);
                    
                }
                if (cJSON_IsString(cJSON_GetObjectItemCaseSensitive(json_msg, "unique_id")) && cJSON_GetObjectItemCaseSensitive(json_msg, "unique_id")->valuestring)
                {

                    ESP_LOGI(TAG, "unique_id GET: %s", (char *)cJSON_GetObjectItemCaseSensitive(json_msg, "unique_id")->valuestring);
                }
            }

            else if (strcmp(cJSON_GetObjectItemCaseSensitive(json, "action")->valuestring, "defaultsettings")== 0)
            {
                if (cJSON_IsString(cJSON_GetObjectItemCaseSensitive(json_msg, "broker_address")) && cJSON_GetObjectItemCaseSensitive(json_msg, "broker_address")->valuestring)
                {
                    defaulta("broker_address",DEFAULT_BROKER);
                }
                if (cJSON_IsString(cJSON_GetObjectItemCaseSensitive(json_msg, "ssid")) && cJSON_GetObjectItemCaseSensitive(json_msg, "ssid")->valuestring)
                {
                    defaulta("ssid",DEFAULT_SSID);
                }
                if (cJSON_IsString(cJSON_GetObjectItemCaseSensitive(json_msg, "publish_topic")) && cJSON_GetObjectItemCaseSensitive(json_msg, "publish_topic")->valuestring)
                {
                    defaulta("publish_topic",DEFAULT_PUBLISH);
                }
                if (cJSON_IsString(cJSON_GetObjectItemCaseSensitive(json_msg, "subscribe_topic")) && cJSON_GetObjectItemCaseSensitive(json_msg, "subscribe_topic")->valuestring)
                {
                    defaulta("subscribe_topic",DEFAULT_SUBSCRIBE);
                }
                if (cJSON_IsString(cJSON_GetObjectItemCaseSensitive(json_msg, "password")) && cJSON_GetObjectItemCaseSensitive(json_msg, "password")->valuestring)
                {
                    defaulta("password",DEFAULT_SENHA);
                }
            
            }
            else{

               if(!led_flag){
                strcpy(response,"ACTION NAO RECONHECIDO !");
                json_flag = 1;
               }
                

            }
            
            if (!json_flag) {

                char *json_str = cJSON_PrintUnformatted(response_ptr);
                if (json_str) {
                    strcpy(response, json_str);
                    cJSON_Delete(json);
                }
            } else {
            
                ESP_LOGE(TAG, "ACTION NAO RECONHECIDO !");
            }
        }
        else
        {
            // ESP_LOGE(TAG, "JSON INVALIDO, ACTION NAO ENCONTRADO !");
            // strcpy(response, "JSON INVALIDO, ACTION NAO ENCONTRADO !");
            // uart_write_bytes(ECHO_UART_PORT_NUM, "Comando inválido\n", strlen(response));
            if(!led_flag){
                ESP_LOGE(TAG, "ACTION NAO ENCONTRADO !");
                strcpy(response, "ACTION NAO ENCONTRADO !");
            }
        }
    }
    else
    {
        ESP_LOGE(TAG, "ERROR JSON MENSAGEM !");
        strcpy(response, "Comando invalido");
        ESP_LOGW(TAG, "Comando invalido recebido: %c", data[0]);
        // uart_write_bytes(ECHO_UART_PORT_NUM, "Comando inválido\n", strlen(response));
    }

    return IsNeedReboot;
}




//------------------------- FUNÇÕES ------------------------------------------

void defaulta(char *string1, char *macro)
{

    cJSON_ReplaceItemInObject(json_msg, string1, cJSON_CreateString(macro));
    char *json_print = cJSON_PrintUnformatted(json_msg);
    write_nvs(json_print, "json");
    free(json_print);
}

void printa(void)
{
    char *json_print = cJSON_PrintUnformatted(json_msg);
    write_nvs(json_print, "json");
    free(json_print);
}