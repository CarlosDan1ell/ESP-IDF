/* ESPNOW Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

/*
   This example shows how to use ESPNOW.
   Prepare two device, one for sending ESPNOW data and another for receiving
   ESPNOW data.
*/
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "nvs_flash.h"
#include "esp_random.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_now.h"
#include "esp_crc.h"
#include "driver_espnow_master.h"

#define ESPNOW_MAXDELAY 512
#define MASTER 1
#define SLAVE 0

static const char *TAG = "espnow_example";
const char *msg = "ACK_FROM_MASTER";

static QueueHandle_t s_example_espnow_queue;

static uint8_t s_example_broadcast_mac[ESP_NOW_ETH_ALEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
// static uint16_t s_example_espnow_seq[EXAMPLE_ESPNOW_DATA_MAX] = {0, 0};

static void example_espnow_deinit(example_espnow_send_param_t *send_param);
char payload[16] = {0};

/* WiFi should start before using ESPNOW */
void example_wifi_init(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(ESPNOW_WIFI_MODE));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_set_channel(CONFIG_ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE));

#if CONFIG_ESPNOW_ENABLE_LONG_RANGE
    ESP_ERROR_CHECK(esp_wifi_set_protocol(ESPNOW_WIFI_IF, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N | WIFI_PROTOCOL_LR));
#endif
}

/* ESPNOW sending or receiving callback function is called in WiFi task.
 * Users should not do lengthy operations from this task. Instead, post
 * necessary data to a queue and handle it from a lower priority task. */
static void example_espnow_send_cb(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    example_espnow_event_t evt;
    example_espnow_event_send_cb_t *send_cb = &evt.info.send_cb;

    if (mac_addr == NULL)
    {
        ESP_LOGE(TAG, "Send cb arg error");
        return;
    }

    evt.id = EXAMPLE_ESPNOW_SEND_CB;
    memcpy(send_cb->mac_addr, mac_addr, ESP_NOW_ETH_ALEN);
    send_cb->status = status;
    if (xQueueSend(s_example_espnow_queue, &evt, ESPNOW_MAXDELAY) != pdTRUE)
    {
        ESP_LOGW(TAG, "Send send queue fail");
    }
}

static void example_espnow_recv_cb(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len)
{
    example_espnow_event_t evt;
    example_espnow_event_recv_cb_t *recv_cb = &evt.info.recv_cb;
    uint8_t *mac_addr = recv_info->src_addr;
    uint8_t *des_addr = recv_info->des_addr;

    ESP_LOGI(TAG, "pkt src_addr " MACSTR ",pkt des_addr: " MACSTR "", MAC2STR(recv_info->src_addr), MAC2STR(recv_info->des_addr));

    if (mac_addr == NULL || data == NULL || len <= 0)
    {
        ESP_LOGE(TAG, "Receive cb arg error");
        return;
    }

    if (IS_BROADCAST_ADDR(des_addr))
    {
        ESP_LOGD(TAG, "Receive broadcast ESPNOW data");
    }
    else
    {
        ESP_LOGD(TAG, "Receive unicast ESPNOW data");
    }

    evt.id = EXAMPLE_ESPNOW_RECV_CB;

    memcpy(recv_cb->mac_addr, mac_addr, ESP_NOW_ETH_ALEN);
    recv_cb->data = malloc(len);

    if (recv_cb->data == NULL)
    {
        ESP_LOGE(TAG, "Malloc falhou no recv_cb");
        return;
    }
    memcpy(recv_cb->data, data, len);
    recv_cb->data_len = len;

    // ESP_LOGI(TAG, "DATA %s, LEN %d", data, len);

    if (xQueueSend(s_example_espnow_queue, &evt, ESPNOW_MAXDELAY) != pdTRUE)
    {
        ESP_LOGW(TAG, "Send receive queue fail");
        free(recv_cb->data);
    }
}

int example_espnow_data_parse(uint8_t *data,uint16_t data_len,char *payload_out,uint16_t payload_max_len, uint8_t *header, uint8_t *method)
{
     if (data == NULL || data_len == 0 || payload_out == NULL)
    {
        return -1;
    }

    packet_protocol_t *buf = (packet_protocol_t *)data;
    
    if (data_len < sizeof(packet_protocol_t)) {
        ESP_LOGE(TAG, "Receive ESPNOW data too short, len:%d", data_len);
        return -1;
    }
    uint16_t  payload_len = data_len - sizeof(packet_protocol_t);
    *header = buf->header;
    *method = buf->method;
    
    // Limita cópia para não estourar buffer
    // uint16_t len = (data_len < payload_max_len - 1) ? data_len : payload_max_len - 1;

    // memcpy(payload_out, data, len);
    memcpy(payload_out,buf->payload,payload_len);
    payload_out[payload_len] = '\0'; // Garante string válida

    return 0; // Sucesso
}

void example_espnow_data_prepare(example_espnow_send_param_t *send_param)
{
    if (send_param == NULL || send_param->buffer == NULL) {
        ESP_LOGE(TAG, "send_param ou buffer NULL!");
        return;
    }
    packet_protocol_t *buf = ( packet_protocol_t *)send_param->buffer;
   
    buf->header = 0x55;
    buf->method = 0xFF;

    strcpy((char*)buf->payload,"Hello Slave");
    buf->payload_len = strlen((char*)buf->payload);
    send_param->len = sizeof(packet_protocol_t) + buf->payload_len;

    return;
    // uint8_t mac_local[6];
    // esp_read_mac(mac_local, ESP_MAC_WIFI_STA); // ou esp_wifi_get_mac conforme sua config

    // // assegura buffer mínimo
    // if (send_param->len < 6) {
    //     ESP_LOGW(TAG, "Buffer muito pequeno para MAC, ajustando len");
    //     send_param->len = 6;
    // }

    // memcpy(send_param->buffer, mac_local, 6);
    // send_param->len = 6;
}


static void example_espnow_task(void *pvParameter)
{
    example_espnow_event_t evt;
    bool is_broadcast;
    uint8_t recv_header = 0;
    uint8_t recv_method = 0;
    esp_now_peer_info_t peer = {0};

    vTaskDelay(5000 / portTICK_PERIOD_MS);

    example_espnow_send_param_t *send_param = (example_espnow_send_param_t *)pvParameter;

    ESP_LOGI(TAG, "Iniciando broadcast...");

  
    if (esp_now_send(send_param->dest_mac,
                     send_param->buffer,
                     send_param->len) != ESP_OK) {
        ESP_LOGE(TAG, "Send error");
        example_espnow_deinit(send_param);
        vTaskDelete(NULL);
    }



    while (xQueueReceive(s_example_espnow_queue, &evt, portMAX_DELAY) == pdTRUE)
    {
        switch (evt.id)
        {
            case EXAMPLE_ESPNOW_SEND_CB:
            {
                example_espnow_event_send_cb_t *send_cb = &evt.info.send_cb;

                is_broadcast = IS_BROADCAST_ADDR(send_cb->mac_addr);

                ESP_LOGI(TAG, "Enviado para " MACSTR " | status: %d",MAC2STR(send_cb->mac_addr),send_cb->status);

                break;
            }

            case EXAMPLE_ESPNOW_RECV_CB:
            {
                example_espnow_event_recv_cb_t *recv_cb = &evt.info.recv_cb;
                uint16_t payload_len = 0;
                int ret;
                ret = example_espnow_data_parse(recv_cb->data,recv_cb->data_len,payload,payload_len,&recv_header,&recv_method);

                if(ret == 0){
                    if (!esp_now_is_peer_exist(recv_cb->mac_addr)) {

                    ESP_LOGI(TAG, "SLAVE sem cadastrado: " MACSTR,MAC2STR(recv_cb->mac_addr));

                    peer.channel = CONFIG_ESPNOW_CHANNEL;
                    peer.ifidx   = ESPNOW_WIFI_IF;
                    peer.encrypt = false;
                    memcpy(peer.peer_addr, recv_cb->mac_addr, ESP_NOW_ETH_ALEN);

                    if (esp_now_add_peer(&peer) == ESP_OK) {
                        ESP_LOGI(TAG, "SLAVE cadastrado com sucesso");
                    } else {
                        ESP_LOGE(TAG, "Erro ao cadastrar SLAVE");
                    }
                    // memcpy(send_param->dest_mac, recv_cb->mac_addr,6);
                    // example_espnow_data_prepare(send_param);

                    //  esp_now_send(send_param->dest_mac,
                    //  send_param->buffer,
                    //  send_param->len);


                    // uint8_t mac_master[6];
                    // esp_read_mac(mac_master, ESP_MAC_WIFI_STA);

                    // esp_now_send(recv_cb->mac_addr, mac_master, 6);
                    // ESP_LOGI(TAG, "MAC do MASTER enviado ao SLAVE");

                    free(recv_cb->data);
                    break;
                    }

                    if (recv_header == 0x55) {
            
                        // Se o método for botão (ex: 0xFF)
                        if (recv_method == 0xFF) {

                            ESP_LOGI(TAG, "Botão do Slave " MACSTR " está: %s", MAC2STR(recv_cb->mac_addr), payload);
                              example_espnow_data_prepare(send_param);

                                esp_now_send(peer.peer_addr,
                                send_param->buffer,
                                send_param->len);

                            free(recv_cb->data);
                            break;
                        }
                    }
                }

            }
            default:
            ESP_LOGE(TAG,"Evento desconhecido: %d",evt.id);
            break;
        }
    }
}



esp_err_t example_espnow_init(void)
{
    example_espnow_send_param_t *send_param;

    s_example_espnow_queue = xQueueCreate(ESPNOW_QUEUE_SIZE, sizeof(example_espnow_event_t));
    if (s_example_espnow_queue == NULL)
    {
        ESP_LOGE(TAG, "Create mutex fail");
        return ESP_FAIL;
    }

    /* Initialize ESPNOW and register sending and receiving callback function. */
    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_register_send_cb(example_espnow_send_cb));
    ESP_ERROR_CHECK(esp_now_register_recv_cb(example_espnow_recv_cb));
#if CONFIG_ESPNOW_ENABLE_POWER_SAVE
    ESP_ERROR_CHECK(esp_now_set_wake_window(CONFIG_ESPNOW_WAKE_WINDOW));
    ESP_ERROR_CHECK(esp_wifi_connectionless_module_set_wake_interval(CONFIG_ESPNOW_WAKE_INTERVAL));
#endif
    /* Set primary master key. */
    ESP_ERROR_CHECK(esp_now_set_pmk((uint8_t *)CONFIG_ESPNOW_PMK));

    /* Add broadcast peer information to peer list. */
    esp_now_peer_info_t *peer = malloc(sizeof(esp_now_peer_info_t));
    if (peer == NULL)
    {
        ESP_LOGE(TAG, "Malloc peer information fail");
        vSemaphoreDelete(s_example_espnow_queue);
        esp_now_deinit();
        return ESP_FAIL;
    }
    memset(peer, 0, sizeof(esp_now_peer_info_t));
    peer->channel = CONFIG_ESPNOW_CHANNEL;
    peer->ifidx = ESPNOW_WIFI_IF;
    peer->encrypt = false;
    memcpy(peer->peer_addr, s_example_broadcast_mac, ESP_NOW_ETH_ALEN);
    ESP_ERROR_CHECK(esp_now_add_peer(peer));
    free(peer);

    /* Initialize sending parameters. */
    send_param = malloc(sizeof(example_espnow_send_param_t));
    if (send_param == NULL)
    {
        ESP_LOGE(TAG, "Malloc send parameter fail");
        vSemaphoreDelete(s_example_espnow_queue);
        esp_now_deinit();
        return ESP_FAIL;
    }
    memset(send_param, 0, sizeof(example_espnow_send_param_t));
    send_param->unicast = false;
    send_param->broadcast = true;
    send_param->state = 0;
    // send_param->magic = esp_random();
    send_param->magic = MASTER;
    // send_param->count = CONFIG_ESPNOW_SEND_COUNT;
    // send_param->delay = CONFIG_ESPNOW_SEND_DELAY;
    send_param->len = 250;
    send_param->buffer = malloc(send_param->len);

    if (send_param->buffer == NULL)
    {
        ESP_LOGE(TAG, "Malloc send buffer fail");
        free(send_param);
        vSemaphoreDelete(s_example_espnow_queue);
        esp_now_deinit();
        return ESP_FAIL;
    }

    memcpy(send_param->dest_mac, s_example_broadcast_mac, ESP_NOW_ETH_ALEN);
    example_espnow_data_prepare(send_param);

    if (xTaskCreate(example_espnow_task, "example_espnow_task", 2048, send_param, 4, NULL)!= pdPASS) {
    ESP_LOGE(TAG, "Erro ao criar task");
    example_espnow_deinit(send_param);
    return ESP_FAIL;
}
    return ESP_OK;
}

static void example_espnow_deinit(example_espnow_send_param_t *send_param)
{
    free(send_param->buffer);
    free(send_param);
    vSemaphoreDelete(s_example_espnow_queue);
    esp_now_deinit();
}
