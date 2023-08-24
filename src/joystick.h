#ifndef ESPNOW_H
#define ESPNOW_H

#include <Arduino.h>

// 设置数据结构体
typedef struct
{
  byte j1PotX; // 左右摇杆
  byte j1PotY;
  byte j2PotX;
  byte j2PotY;
  bool buttonR1; // 左右按键
  bool buttonR2;
  bool j1Button; // 左右摇杆按键
  bool j2Button;
  bool buttonLB; // 左右扳机按键
  bool buttonRB;
  bool tSwitch1; // 拨动开关
  byte roll;     // 用于M5StackCore2或未来增加的陀螺仪功能
  byte pitch;
  bool buttonR3;
} Data_Package;

void JoyStick_Init();
void JoyStick_Reset();
void JoyStick_SendData();
void JoyStick_PrintState();
void JoyStick_Loop();

#endif
