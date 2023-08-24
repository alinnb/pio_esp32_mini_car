#include "espnow_wrapper.h"

#include <WiFi.h>
#include <esp_now.h>

// 全0xFF的Mac地址时广播到附近所有ESPNOW设备
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
esp_now_peer_info_t peerInfo = {};

static EspnowReceiver receiver = 0;
static uint8_t espnow_mode;

// 数据发送回调函数
// callback when data is sent from Master to Slave
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  //  Serial.print("Last Packet Sent to: "); Serial.println(macStr);
  //  Serial.print("Last Packet Send Status: "); Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// 数据接收回调函数
// callback when data is received from Slave to Master
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *incomingData, int data_len)
{
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  // memcpy(&data, incomingData, sizeof(data));
  //  Serial.print("j1PotX: ");
  //  Serial.print(data.j1PotX);
  //  Serial.print("; j1PotY: ");
  //  Serial.print(data.j1PotY);
  //  Serial.print("; j2PotX: ");
  //  Serial.print(data.j2PotX);
  //  Serial.print("; j2PotY: ");
  //  Serial.println(data.j2PotY);
  if (receiver != nullptr)
  {
    receiver(incomingData, data_len);
  }
}

void ESPNOW_Init(uint8_t mode, EspnowReceiver cb)
{
  Serial.println("ESPNOW_Init");
  espnow_mode = mode;

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

  if (cb != nullptr)
  {
    receiver = cb;
  }

  Serial.print("MAC address of this node is ");
  Serial.println(WiFi.softAPmacAddress());
}

// 配对连接pair with another ESP-NOW device
void ESPNOW_PairDevice()
{
  memcpy(&peerInfo.peer_addr, broadcastAddress, 6);
  if (!esp_now_is_peer_exist(broadcastAddress))
  {
    esp_now_add_peer(&peerInfo);
  }
}

// 发送数据 send data
void ESPNOW_sendMessageBroadcast(uint8_t *data, size_t len)
{
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&data, sizeof(data));
}
