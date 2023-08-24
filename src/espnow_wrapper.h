#ifndef ESPNOW_WRAPPER_H
#define ESPNOW_WRAPPER_H

#include <Arduino.h>

#define ESPNOW_MODE_BROADCAST   0
#define ESPNOW_MODE_UNICAST     1

typedef void (*EspnowReceiver)(const uint8_t *msg, size_t len);

void ESPNOW_Init(uint8_t mode, EspnowReceiver cb = nullptr);
void ESPNOW_sendMessageBroadcast(uint8_t *msg, size_t len);
void ESPNOW_Loop();
void ESPNOW_PairDevice();

#endif /* ESPNOW_WRAPPER_H */
