#ifndef DRIVES_DEFINES_H
#define DRIVES_DEFINES_H

#include <string.h>
#include <stdint.h>
#include "nvs.h"
#include "mqtt_client.h"


// === UART === //
#define ECHO_TEST_TXD (17)
#define ECHO_TEST_RXD (16)
#define ECHO_TEST_RTS (UART_PIN_NO_CHANGE)
#define ECHO_TEST_CTS (UART_PIN_NO_CHANGE)

#define ECHO_UART_PORT_NUM      (2)
#define ECHO_UART_BAUD_RATE     (115200)
#define ECHO_TASK_STACK_SIZE    (2048)
#define BUF_SIZE (1024)

// === NVS === //
#define NAMESPACE "wifi_conf"
#define KEY1 "wifi_ssid"
#define KEY2 "wifi_password"
#define KEY3 "broker_mqtt"
#define KEY4 "publish_mqtt"
#define KEY5 "subscribe_mqtt"
#define DEFAULT_SSID "Cyber_Workspace"
#define DEFAULT_SENHA "HubLS3s2"
#define DEFAULT_BROKER "mqtt://broker.hivemq.com:1883"
#define DEFAULT_PUBLISH "Oi, meu nome eh carlos"
#define DEFAULT_SUBSCRIBE "Saudacao"



// --- Somente declarações (não crie memória aqui) ---
extern uint8_t UartBufferSENHA[BUF_SIZE];
extern uint8_t UartBufferSSID[BUF_SIZE];
extern uint8_t UartBufferBROKER[BUF_SIZE];
extern uint8_t UartBufferPUBLISH[BUF_SIZE];
extern uint8_t UartBufferSUBSCRIBE[BUF_SIZE];


extern char get_wifi_ssid[BUF_SIZE];
extern char get_wifi_senha[BUF_SIZE];
extern char get_broker_mqtt[BUF_SIZE];
extern char get_publish_mqtt[BUF_SIZE];
extern char get_sub_mqtt[BUF_SIZE];
extern char current_publish_topic[BUF_SIZE];
extern char current_subscribe_topic[BUF_SIZE];
extern size_t len;
extern size_t len2;
extern size_t len3;
extern size_t len4;
extern size_t len5;
extern size_t len_ssid;
extern size_t len_pass;
extern nvs_handle_t my_handle;
extern esp_mqtt_client_handle_t client;





extern volatile char IsReceiveUART;
extern volatile char IsBufferSSID;
extern volatile char IsBufferSENHA;
extern volatile char ssid_updated;
extern volatile char password_updated;
extern volatile char IsWifiReconfigure;
extern volatile char IsBrokerReconfigure;
extern volatile char IsBufferBroker;
extern volatile char IsBufferSubscribe;
extern volatile char IsBufferPublish;
extern volatile char IsPublishReconfigure;
extern volatile char IsSubReconfigure;



// === Funções === //

void uart_read_task(void *arg);
void nvs_driver(void *pvParameters);
void wifi_reconfig_task(void *pvParameters);
void mqtt_app_start(void);
void mqtt_reconfig_task(void *pvParameters);


#endif // DEFINES_H
