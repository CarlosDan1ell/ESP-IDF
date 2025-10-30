#ifndef DRIVES_DEFINES_H
#define DRIVES_DEFINES_H

#include <string.h>
#include <stdint.h>
#include "nvs.h"


// === UART === //
#define ECHO_TEST_TXD (1)
#define ECHO_TEST_RXD (3)
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
#define DEFAULT_SSID "Cyber_Workspace"
#define DEFAULT_SENHA "HubLS3s2"

// --- Somente declarações (não crie memória aqui) ---
extern uint8_t UartBufferSENHA[BUF_SIZE];
extern uint8_t UartBufferSSID[BUF_SIZE];
extern char get_wifi_ssid[BUF_SIZE];
extern char get_wifi_senha[BUF_SIZE];
extern size_t len;
extern size_t len2;
extern size_t len_ssid;
extern size_t len_pass;
extern nvs_handle_t my_handle;





extern volatile char IsReceiveUART;
extern volatile char IsBufferSSID;
extern volatile char IsBufferSENHA;
extern volatile char ssid_updated;
extern volatile char password_updated;
extern volatile char IsWifiReconfigure;

// === Funções === //

void uart_read_task(void *arg);
void nvs_driver(void *pvParameters);
void wifi_reconfig_task(void *pvParameters);


#endif // DEFINES_H
