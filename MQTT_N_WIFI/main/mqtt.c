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
#include "protocol_examples_common.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "mqtt_client.h"
#include "WiFi.h"
#include "mqtt.h"

#define ALO "entrou"
#define HELLO "aqui"
extern volatile uint8_t IsBufferMSGReady;
extern char BufferMSG[BufferMSG_len];
static const char *TAG = "mqtt_example";


static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0)
    {
        ESP_LOGE(TAG,"Last error %s : 0x%x", message, error_code);
    }
}
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{

    ESP_LOGD(TAG,"Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event -> client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id){
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        //msg_id = esp_mqtt_client_publish(client, "topic/qos1", "data_3", 0, 1, 0); // função que faz com que o cliente publique no topico o dado com o tamanho autocalculado (0) somente 1 vez e que nao retenha a mensagem (0)
        //ESP_LOGI(TAG, "sent publish sucessful, msg_id=%d", msg_id);
        msg_id = esp_mqtt_client_subscribe(client, "/Leds/Controle", 0);
        ESP_LOGI(TAG, "Subscribed to %d", msg_id);

        /* msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
        ESP_LOGI(TAG, "sent subscribe sucessful, msg_id=%d", msg_id); */

        //msg_id = esp_mqtt_client_unsubscribe(client, "/topic1/qos1");
        //ESP_LOGI(TAG, "sent unsubscribe sucessful, msg_id=%d",msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
       /*  msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id); */
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG,"MQTT_EVENT_PUBLISHED, msg_id%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG,"MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event ->data_len, event -> data);

        if(event->data_len < BufferMSG_len) //Vou verificar se a mensagem é maior q minha memória 
        {
            snprintf(BufferMSG,BufferMSG_len, "%.*s", event->data_len, event->data);
            IsBufferMSGReady = 1;
            ESP_LOGI(TAG, "Sucesso! Dados copiados para o buffer: '%s'", BufferMSG);
        }
        else{
            ESP_LOGW(TAG, "A mensagem recebida é muito grande para o buffer");
            
        }
        
        //mqtt_mensagem(event->topic, event->data, event->data_len);
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG,"MQTT_EVENT_ERROR");
        if(event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
        {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;
    default:
        ESP_LOGI(TAG,"Other event id:%d", event->event_id);
        break;
    }
}

void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtt://192.168.0.243:1883",
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client,ESP_EVENT_ANY_ID, mqtt_event_handler,NULL);
    esp_mqtt_client_start(client);
}
/* void app_main1(void)
{
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("mqtt_client", ESP_LOG_VERBOSE);
    esp_log_level_set("mqtt_example", ESP_LOG_VERBOSE);
    esp_log_level_set("transport_base", ESP_LOG_VERBOSE);
    esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);
    esp_log_level_set("transport", ESP_LOG_VERBOSE);
    esp_log_level_set("outbox", ESP_LOG_VERBOSE);

    //ESP_ERROR_CHECK(nvs_flash_init());
    //ESP_ERROR_CHECK(esp_event_loop_create_default());
    
}
 */