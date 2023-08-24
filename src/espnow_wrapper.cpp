#include "espnow_wrapper.h"

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#include <espnow.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#include <esp_now.h>
#endif


// 全0xFF的Mac地址时广播到附近所有ESPNOW设备
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
#if defined(ARDUINO_ARCH_ESP32)
esp_now_peer_info_t peerInfo = {};
#endif

static EspnowReceiver receiver = 0;
static uint8_t espnow_mode;
static uint32_t espnow_appMask;

// 数据发送回调函数
// callback when data is sent from Master to Slave
#if defined(ARDUINO_ARCH_ESP8266)
void OnDataSent(uint8_t *mac_addr, uint8_t status)
#elif defined(ARDUINO_ARCH_ESP32)
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
#endif
{
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  //  Serial.print("Last Packet Sent to: "); Serial.println(macStr);
  //  Serial.print("Last Packet Send Status: "); Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// 数据接收回调函数
// callback when data is received from Slave to Master
#if defined(ARDUINO_ARCH_ESP8266)
void OnDataRecv(uint8_t *mac_addr, uint8_t *incomingData, u8 data_len)
#elif defined(ARDUINO_ARCH_ESP32)
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *incomingData, int data_len)
#endif
{
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);

  // 解析 ESPNOW_BaseData 头
  ESPNOW_BaseData espData;
  memcpy(&espData, incomingData, sizeof(ESPNOW_BaseData));

  Serial.print("ESPNOW OnDataRecv AppId:");
  Serial.print(espData.appId, DEC);

  if (!bitRead(espnow_appMask, espData.appId))
  {
    Serial.println(" is not registered.");
    return;
  }

  // 解析剩余的内容
  Serial.print(" Data:");
  const uint8_t *pNewData = incomingData + sizeof(ESPNOW_BaseData);
  uint32_t newLen = data_len - sizeof(ESPNOW_BaseData);
  for (uint32_t i = 0; i < newLen; ++i)
  {
    Serial.print("0x");
    Serial.print(pNewData[i], HEX);
    Serial.print(",");
  }
  Serial.println();

  if (receiver != nullptr)
  {
    receiver(pNewData, newLen);
  }
}

void ESPNOW_Init(uint8_t mode)
{
  Serial.println("ESPNOW_Init");

  espnow_mode = mode;
  receiver = nullptr;
  espnow_appMask = 0;

  if (espnow_mode == ESPNOW_MODE_BROADCAST)
  {
    // 初始化 ESP-NOW
    Serial.println("Initializing ESP-NOW...");
    delay(100);
    WiFi.mode(WIFI_STA);
    Serial.println("InitESPNow");
    // This is the mac address of the Master in Station Mode
    Serial.print("STA MAC: ");
    Serial.println(WiFi.macAddress());
    WiFi.disconnect();
    if (esp_now_init() == 0)
    { // ESP_OK=0
      Serial.println("ESPNow Init Success");
    }
    else
    {
      Serial.println("ESPNow Init Failed");
      // Retry InitESPNow, add a counte and then restart?
      // InitESPNow();
      // or Simply Restart
      ESP.restart();
    }
    // 配对连接pair with another ESP-NOW device
    ESPNOW_PairDevice();
    Serial.println("ESPNOW_MODE_BROADCAST success.");
  }

  esp_now_register_recv_cb(OnDataRecv);

  Serial.print("MAC address of this node is ");
  Serial.println(WiFi.softAPmacAddress());
}

// 配对连接pair with another ESP-NOW device
void ESPNOW_PairDevice()
{
#if defined(ARDUINO_ARCH_ESP8266)
  if (!esp_now_is_peer_exist(broadcastAddress))
  {
    esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_IDLE, 0, 0, 0);
  }
#elif defined(ARDUINO_ARCH_ESP32)
  memcpy(&peerInfo.peer_addr, broadcastAddress, 6);
  if (!esp_now_is_peer_exist(broadcastAddress))
  {
    esp_now_add_peer(&peerInfo);
  }
#endif
}

// 注册AppId
void ESPNOW_RegisterApp(uint32_t appId, EspnowReceiver cb)
{
  bitSet(espnow_appMask, appId);
  receiver = cb;
}

// 发送数据 send data
void ESPNOW_SendMessageBroadcast(uint32_t appId, uint8_t *data, size_t len)
{
  ESPNOW_BaseData espData;
  espData.appId = appId;

  // 添加 ESPNOW_BaseData 头
  uint32_t size = len + sizeof(ESPNOW_BaseData);
  uint8_t msg[size];
  uint8_t *pMsg = msg;
  memcpy(pMsg, static_cast<void *>(&espData), sizeof(ESPNOW_BaseData));
  memcpy(pMsg + sizeof(ESPNOW_BaseData), data, len);

  Serial.print("ESPNOW_Broadcast AppId:");
  Serial.print(appId, DEC);
  Serial.print(" Data:");
  for (uint32_t i = 0; i < len; ++i)
  {
    Serial.print("0x");
    Serial.print(data[i], HEX);
    Serial.print(",");
  }
  Serial.println();

  esp_now_send(broadcastAddress, pMsg, len + sizeof(ESPNOW_BaseData));
}
