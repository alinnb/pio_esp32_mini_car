#ifndef ESPNOW_WRAPPER_H
#define ESPNOW_WRAPPER_H

#include <Arduino.h>

#define ESPNOW_MODE_BROADCAST   0
#define ESPNOW_MODE_UNICAST     1

#define ESPNOW_APPID_JOYSTICK   1

typedef struct {
    uint32_t appId;
} ESPNOW_BaseData;

typedef void (*EspnowReceiver)(const uint8_t *msg, size_t len);

void ESPNOW_Init(uint8_t mode);
void ESPNOW_RegisterApp(uint32_t appId, EspnowReceiver cb);
void ESPNOW_SendMessageBroadcast(uint32_t appId, uint8_t *msg, size_t len);
void ESPNOW_Loop();
void ESPNOW_PairDevice();

#endif /* ESPNOW_WRAPPER_H */
