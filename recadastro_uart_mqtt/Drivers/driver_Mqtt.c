#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "nvs_flash.h"
/* #include "protocol_examples_common.h" */

#include "lwip/err.h"
#include "lwip/sys.h"

#include "mqtt_client.h"
#include "driver_WiFi.h"
#include "driver_defines.h"

#define ALO "entrou"
#define HELLO "aqui"
static const char *TAG = "mqtt_example";
char current_publish_topic[BUF_SIZE];
char current_subscribe_topic[BUF_SIZE];
int msg_id;
/* static char broker_uri[128]; */
esp_mqtt_client_config_t mqtt_cfg = {0};
esp_mqtt_client_handle_t client = NULL;
extern volatile char IsBrokerReconfigure;
extern volatile char IsPublishReconfigure;
extern volatile char IsSubReconfigure;


static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0)
    {
        ESP_LOGE(TAG, "Last error %s : 0x%x", message, error_code);
    }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{

    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;

    /* int msg_id; */
    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
        // ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        // msg_id = esp_mqtt_client_publish(client, get_publish_mqtt, "data_3", 0, 1, 0); // função que faz com que o cliente publique no topico o dado com o tamanho autocalculado (0) somente 1 vez e que nao retenha a mensagem (0)

        // ESP_LOGI(TAG, "sent publish sucessful, msg_id=%d", msg_id);

        // msg_id = esp_mqtt_client_subscribe(client, get_sub_mqtt, 0);
        // ESP_LOGI(TAG, "sent subscribe sucessful, msg id=%d", msg_id);

        //  msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
        // ESP_LOGI(TAG, "sent subscribe sucessful, msg_id=%d", msg_id);

        // msg_id = esp_mqtt_client_unsubscribe(client, "/topic1/qos1");
        // ESP_LOGI(TAG, "sent unsubscribe sucessful, msg_id=%d",msg_id);
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        nvs_get_str(my_handle, KEY5, get_sub_mqtt, &len5);
        strcpy(current_subscribe_topic, get_sub_mqtt);
        nvs_get_str(my_handle, KEY4, get_publish_mqtt, &len4);
        strcpy(current_publish_topic,get_publish_mqtt);

        
        int msg_id = esp_mqtt_client_subscribe(client, current_subscribe_topic, 0);
        ESP_LOGI(TAG, "Subscribe atual: %s, msg_id=%d", current_subscribe_topic, msg_id);
        
        /* esp_mqtt_client_publish(client, get_publish_mqtt, "Inicializado com sucesso5", 0, 0, 0); */
        esp_mqtt_client_publish(client, current_publish_topic, "Inicializado com sucesso5", 0, 0, 0);
        ESP_LOGI(TAG, "Publish atual: %s",current_publish_topic);
        break;
       
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;
    case MQTT_EVENT_SUBSCRIBED:
        /* ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, topic%.*s\r\n", event->topic_len, event->topic); */
        // msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
       /*  msg_id = esp_mqtt_client_subscribe(client, current_subscribe_topic, 0);
        ESP_LOGI(TAG, "Inscrição confirmada no tópico: %s, msg_id=%d", current_subscribe_topic, event->msg_id); */
        // ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, topic %.*s\r\n", event->topic_len, event->topic);
        /* ESP_LOGI(TAG, "Inscrição confirmada no tópico: %.*s\r\n", event->topic_len, event->topic); */
        
        // ESP_LOGI(TAG, "Inscrição confirmada no tópico: %s, msg_id=%d", current_subscribe_topic, msg_id);
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id%d", event->msg_id);
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, topic %.*s\r\n", event->topic_len, event->topic);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
        {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

void mqtt_app_start(void)
{
    nvs_get_str(my_handle, KEY3, get_broker_mqtt, &len3);
    esp_mqtt_client_config_t mqtt_cfg = {

        .broker.address.uri = get_broker_mqtt,

    };
    // ESP_LOGI("DEBUG", "Endereco da flag IsBrokerReconfigure: %p", &IsBrokerReconfigure);
    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
    xTaskCreate(mqtt_reconfig_task, "mqtt_reconfig_task", 2048, NULL, 1, NULL);
}
void mqtt_reconfig_task(void *pvParameters)
{
    esp_mqtt_client_config_t mqtt_cfg = {0};
    while (1)
    {
        // ESP_LOGI("DEBUG", "Loop reconfig, flag = %d", IsBrokerReconfigure);
        if (IsBrokerReconfigure == 1)
        {
            // lê novas credenciais do NVS
            ESP_LOGW("DEBUG", ">>> Entrou no if reconfigure!");
            IsBrokerReconfigure = 0;
            /* nvs_open("Network_confg", NVS_READWRITE, &my_handle); */
            esp_err_t err_Broker = nvs_get_str(my_handle, KEY3, get_broker_mqtt, &len3);
            ESP_LOGI("DEBUG RECONFIGURE", "URI: %s", get_broker_mqtt);
            // nvs_close(my_handle);
            

            if (err_Broker == ESP_OK)
            {

                if (client != NULL)
                {
                    ESP_LOGI(TAG, "Parando cliente MQTT atual...\n");
                    esp_mqtt_client_stop(client);
                    vTaskDelay(pdMS_TO_TICKS(500));
                    esp_mqtt_client_destroy(client);
                    client = NULL;
                }
                mqtt_cfg.broker.address.uri = get_broker_mqtt;
                ESP_LOGI("DEBUG", "URI ATUALIZADO: %s", mqtt_cfg.broker.address.uri);
                client = esp_mqtt_client_init(&mqtt_cfg);
                esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
                esp_err_t err = esp_mqtt_client_start(client);

                if (client == NULL)
                {
                    ESP_LOGE(TAG, "ERRO AO INICIALIZAR O BROKER ATUALIZADO");
                }
                if (err != ESP_OK)
                {
                    ESP_LOGE(TAG, "Erro ao iniciar MQTT: %s\n", esp_err_to_name(err));
                }
            }
        }
        if (IsPublishReconfigure == 1)
        {
            ESP_LOGW("DEBUG", ">>> Reconfigurando TOPICO DE PUBLISH!");
            IsPublishReconfigure = 0;

            esp_err_t err_pub = nvs_get_str(my_handle, KEY4, get_publish_mqtt, &len4);
            if (err_pub == ESP_OK)
            {
                strcpy(current_publish_topic,get_publish_mqtt);
                ESP_LOGI(TAG, "Novo tópico de publish: %s", current_publish_topic);
                msg_id = esp_mqtt_client_publish(client, current_publish_topic, "FINALMENTE MEU DADO FOI", 0, 0, 0);
            }
            else{
                ESP_LOGE(TAG,"erro para novo publish");
            }
        }
        if (IsSubReconfigure == 1)
        {
            ESP_LOGW("DEBUG", ">>> Reconfigurando TOPICO DE SUBSCRIBE!");
            IsSubReconfigure = 0;
            msg_id = esp_mqtt_client_unsubscribe(client, current_subscribe_topic);
            ESP_LOGI(TAG, "sent unsubscribe sucessful, msg_id=%d",msg_id);

            esp_err_t err_sub = nvs_get_str(my_handle, KEY5, get_sub_mqtt, &len5);
            if (err_sub == ESP_OK)
            {
                strcpy(current_subscribe_topic, get_sub_mqtt);
                ESP_LOGI(TAG, "Novo tópico de subscribe: %s", current_subscribe_topic);
                

                if (client != NULL)
                {
                    esp_mqtt_client_subscribe(client, current_subscribe_topic, 0);
                }
            }
            else{
                ESP_LOGE(TAG,"erro para atualizar novo sub");
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1000)); // checa a flag a cada segundo
    }
}
