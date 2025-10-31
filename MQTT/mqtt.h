#ifndef MQTTT_H
#define MQTTT_H

#define BufferMSG_len 4
void  mqtt_app_start(void);
void mqtt_mensagem(const char *topic, const char *data, int len);


#endif 