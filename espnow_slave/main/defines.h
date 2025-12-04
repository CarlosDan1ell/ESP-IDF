#ifndef DEFINES_H
#define DEFINES_H
#include "driver_espnow_slave.h"
#include "esp_now.h"

void example_wifi_init(void);
esp_err_t example_espnow_init(void);
void init_gpio(void);
uint8_t get_estado(void);
void task_botao(void *pvParameters);
void example_espnow_data_prepare(example_espnow_send_param_t *send_param);


#endif 