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
#include "esp_mac.h"
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
char Buffer1[BUF_SIZE];
char responseMQTT[BUF_SIZE];
int msg_id;


/* static char broker_uri[128]; */
// esp_mqtt_client_config_t mqtt_cfg = {0};
esp_mqtt_client_handle_t client;
extern volatile char IsBrokerReconfigure;
extern volatile char IsPublishReconfigure;
extern volatile char IsSubReconfigure;
volatile char flagMSG;

cJSON* json_mqtt;

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
       

        
        int msg_id = esp_mqtt_client_subscribe(client, cJSON_GetObjectItemCaseSensitive(json_mqtt, "subscribe_topic")->valuestring, 0);
        ESP_LOGI(TAG, "Subscribe atual: %s, msg_id=%d", cJSON_GetObjectItemCaseSensitive(json_mqtt, "subscribe_topic")->valuestring, msg_id);
        
        /* esp_mqtt_client_publish(client, get_publish_mqtt, "Inicializado com sucesso5", 0, 0, 0); */
        // esp_mqtt_client_publish(client, current_publish_topic, "Inicializado com sucesso", 0, 0, 0);
        SEND_MQTT("Inicializado com sucesso",0);
        
        ESP_LOGI(TAG, "Publish atual: %s",cJSON_GetObjectItemCaseSensitive(json_mqtt, "publish_topic")->valuestring);
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
        printf("DATA(%d)=%.*s\r\n",  event->data_len,event->data_len, event->data);
        // ESP_LOGI(TAG, "O DATA COM TERMINADOR %s", data);

        memcpy(Buffer1, event->data, event->data_len);
        Buffer1[event->data_len] = '\0';
        flagMSG = 1; 
       
        
        
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
    

    // nvs_get_str(my_handle, KEY3, get_broker_mqtt, &len3);
    json_mqtt = get_json();
    esp_mqtt_client_config_t mqtt_cfg = {

        .broker.address.uri = cJSON_GetObjectItemCaseSensitive(json_mqtt, "broker_address")->valuestring,
        .credentials.client_id = cJSON_GetObjectItemCaseSensitive(json_mqtt, "unique_id")->valuestring,

    };
    // ESP_LOGI("DEBUG", "Endereco da flag IsBrokerReconfigure: %p", &IsBrokerReconfigure);
    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
    xTaskCreate(task_tratamento, "task_tratamento", 2048, NULL, 1, NULL);
}

void SEND_MQTT(char *Buffer, size_t len){
esp_mqtt_client_publish(client, cJSON_GetObjectItemCaseSensitive(json_mqtt, "publish_topic")->valuestring, Buffer, len, 0, 0);
}

void task_tratamento(void *pvParameters)
{
    while (1)
    {
        if (flagMSG == 1) {
            flagMSG = 0;
            receive_msg(Buffer1,len, responseMQTT);
            SEND_MQTT(responseMQTT, strlen(responseMQTT));
            vTaskDelay(pdMS_TO_TICKS(3000));

            
        }
        vTaskDelay(pdMS_TO_TICKS(100)); 
    }
}
