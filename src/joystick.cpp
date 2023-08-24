#include "joystick.h"
#include "espnow_wrapper.h"

#include <EEPROM.h>
#define EEPROM_SIZE 9

// TRIGGER BUTTONS
#define LB 19
#define RB 25

// LEFT-JOYSTICK
#define LH 34
#define LV 35
#define LS 5

// RIGHT-JOYSTICK
#define RH 36
#define RV 32
#define RS 4

#define R1 2
#define R2 13

// BATTERY VOLTAGE
// #define ADC 39  //电池ADC

// RGB
// #define DATA_PIN 17

// OTHERS
#define BK 23 // 拨动开关

int buttons[7] = {R1, R2, LS, RS, LB, RB, BK};

const int numberOfPotSamples = 5;  // Number of pot samples to take (to smooth the values)
const int delayBetweenSamples = 2; // Delay in milliseconds between pot samples

int LX_read = 0;
int LY_read = 0;
int RX_read = 0;
int RY_read = 0;

byte LX_zero = 127;
byte LY_zero = 127;
byte RX_zero = 127;
byte RY_zero = 127;

int LX_to_send = 0;
int LY_to_send = 0;
int RX_to_send = 0;
int RY_to_send = 0;

bool LY_inverted = false;
bool LX_inverted = false;
bool RY_inverted = false;
bool RX_inverted = false;

int counter = 0;
int invert_counter = 0;

float voltage = 3.00;
int percentage = 0;

// 初始化按键gpio
void pinmode_pullup()
{
  for (int i = 0; i < 7; i++)
  {
    pinMode(buttons[i], INPUT_PULLUP);
  }
}

// 把val从一个区间映射到新的区间
int map_normal(int val, int lower, int middle, int upper, bool reverse)
{
  val = constrain(val, lower, upper);
  if (val < middle)
    val = map(val, lower, middle, 0, 127);
  else
    val = map(val, middle, upper, 127, 255);
  return (reverse ? 255 - val : val);
}

// 读取摇杆状态
void read_joydata()
{
  int potValues[numberOfPotSamples];
  // 左摇杆，LEFT-JOYSTICK
  for (int i = 0; i < numberOfPotSamples; i++)
  {
    potValues[i] = analogRead(LH);
    delay(delayBetweenSamples);
  }
  int potValue = 0;
  for (int i = 0; i < numberOfPotSamples; i++)
  {
    potValue += potValues[i];
  }
  potValue = potValue / numberOfPotSamples;
  LX_read = map(potValue, 0, 4095, 255, 0);

  int potValues2[numberOfPotSamples];
  for (int i = 0; i < numberOfPotSamples; i++)
  {
    potValues2[i] = analogRead(LV);
    delay(delayBetweenSamples);
  }
  int potValue2 = 0;
  for (int i = 0; i < numberOfPotSamples; i++)
  {
    potValue2 += potValues2[i];
  }
  potValue2 = potValue2 / numberOfPotSamples;
  LY_read = map(potValue2, 0, 4095, 255, 0);

  // 右摇杆，RIGHT-JOYSTICK
  int potValues3[numberOfPotSamples];
  for (int i = 0; i < numberOfPotSamples; i++)
  {
    potValues3[i] = analogRead(RH);
    delay(delayBetweenSamples);
  }
  int potValue3 = 0;
  for (int i = 0; i < numberOfPotSamples; i++)
  {
    potValue3 += potValues3[i];
  }
  potValue3 = potValue3 / numberOfPotSamples;
  RX_read = map(potValue3, 0, 4095, 255, 0);

  int potValues4[numberOfPotSamples];
  for (int i = 0; i < numberOfPotSamples; i++)
  {
    potValues4[i] = analogRead(RV);
    delay(delayBetweenSamples);
  }
  int potValue4 = 0;
  for (int i = 0; i < numberOfPotSamples; i++)
  {
    potValue4 += potValues4[i];
  }
  potValue4 = potValue4 / numberOfPotSamples;
  RY_read = map(potValue4, 0, 4095, 255, 0);
}

// 摇杆原点纠偏程序
void zero_test()
{
  Serial.println(" joy_zero_testing... ");
  read_joydata();
  delay(300);

  LX_to_send = map_normal(LX_read, 0, 127, 255, 0);
  LY_to_send = map_normal(LY_read, 0, 127, 255, 0);
  RX_to_send = map_normal(RX_read, 0, 127, 255, 0);
  RY_to_send = map_normal(RY_read, 0, 127, 255, 0);

  LX_to_send = constrain(LX_to_send, 0, 255);
  LY_to_send = constrain(LY_to_send, 0, 255);
  RX_to_send = constrain(RX_to_send, 0, 255);
  RY_to_send = constrain(RY_to_send, 0, 255);

  LX_zero = LX_to_send;
  LY_zero = LY_to_send;
  RX_zero = RX_to_send;
  RY_zero = RY_to_send;

  Serial.println(" Writing in EEPROM... ");
  delay(300);

  // 把纠偏的值写入E2PROM
  if (EEPROM.read(1) != LX_zero)
    EEPROM.write(1, LX_zero);
  if (EEPROM.read(2) != LY_zero)
    EEPROM.write(2, LY_zero);
  if (EEPROM.read(3) != RX_zero)
    EEPROM.write(3, RX_zero);
  if (EEPROM.read(4) != RY_zero)
    EEPROM.write(4, RY_zero);
  EEPROM.commit();

  Serial.println(" Done... ");
  Serial.print(" LX_zero: ");
  Serial.print(EEPROM.read(1));
  Serial.print(" LY_zero: ");
  Serial.print(EEPROM.read(2));
  Serial.print(" RX_zero: ");
  Serial.print(EEPROM.read(3));
  Serial.print(" RY_zero: ");
  Serial.println(EEPROM.read(4));
}

// 初始化摇杆的初值并存储到e2prom
void eeprom_ini()
{
  Serial.print(" eeprom_ini() ");
  EEPROM.begin(EEPROM_SIZE);

  if (EEPROM.read(0) != 55)
  { // 判断是否首次使用（新的所有地址貌似是255的值）
    zero_test();
    EEPROM.write(0, 55);
    EEPROM.commit();
  }

  LX_zero = EEPROM.read(1);
  LY_zero = EEPROM.read(2);
  RX_zero = EEPROM.read(3);
  RY_zero = EEPROM.read(4);
}

//======================================
Data_Package data;    // Create a variable with the above structure
Data_Package predata; // the last state

void copyData()
{
  memcpy((uint8_t *)&predata, (uint8_t *)&data, sizeof(data));
}

int compareData()
{
  if (abs(predata.j1PotX - data.j1PotX) > 5 || abs(predata.j1PotY - data.j1PotY) > 5 || abs(predata.j2PotX - data.j2PotX) > 5 || abs(predata.j2PotY - data.j2PotY) > 5)
  {
    return 1;
  }

  if (predata.buttonLB != data.buttonLB || predata.buttonRB != data.buttonRB || predata.buttonR1 != data.buttonR1 || predata.buttonR2 != data.buttonR2 || predata.tSwitch1 != data.tSwitch1)
  {
    return 1;
  }

  return 0;
}

void JoyStick_Reset()
{ // 数据重置
  // Reset the values when there is no radio connection - Set initial default values
  data.j1PotX = 127;
  data.j1PotY = 127;
  data.j2PotX = 127;
  data.j2PotY = 127;
  data.buttonR1 = 1;
  data.buttonR2 = 1;
  data.j1Button = 1;
  data.j2Button = 1;
  data.buttonLB = 1;
  data.buttonRB = 1;
  data.tSwitch1 = 1;
  data.roll = 127;
  data.pitch = 127;
  data.buttonR3 = 1;

  copyData();
}

void JoyStick_PrintState()
{
  Serial.print("JoyStick Data Size: ");
  Serial.print(sizeof(data));
  Serial.println(" Bytes");
  Serial.print(" LX: ");
  Serial.print(data.j1PotX);
  Serial.print(" LY: ");
  Serial.print(data.j1PotY);
  Serial.print(" RX: ");
  Serial.print(data.j2PotX);
  Serial.print(" RY: ");
  Serial.print(data.j2PotY);
  Serial.print(" R1: ");
  Serial.print(data.buttonR1);
  Serial.print(" R2: ");
  Serial.print(data.buttonR2);
  Serial.print(" J1: ");
  Serial.print(data.j1Button);
  Serial.print(" J2: ");
  Serial.print(data.j2Button);
  Serial.print(" LB: ");
  Serial.print(data.buttonLB);
  Serial.print(" RB: ");
  Serial.print(data.buttonRB);
  Serial.print(" T1: ");
  Serial.println(data.tSwitch1);
}

// 数据接收回调函数
// callback when data is received from Slave to Master
void JoyStick_OnDataRecv(const uint8_t *msg, size_t len)
{
  memcpy(&data, msg, len);
  Serial.print("JoyStick_OnDataRecv");
  Serial.print("j1PotX: ");
  Serial.print(data.j1PotX);
  Serial.print("; j1PotY: ");
  Serial.print(data.j1PotY);
  Serial.print("; j2PotX: ");
  Serial.print(data.j2PotX);
  Serial.print("; j2PotY: ");
  Serial.println(data.j2PotY);
}

void JoyStick_Init()
{
  pinmode_pullup();
  JoyStick_Reset();
  ESPNOW_Init(ESPNOW_MODE_BROADCAST);
  ESPNOW_RegisterApp(ESPNOW_APPID_JOYSTICK, JoyStick_OnDataRecv);

  eeprom_ini(); // EEPROM初始化
  Serial.print(" LX_zero: ");
  Serial.print(EEPROM.read(1));
  Serial.print(" LY_zero: ");
  Serial.print(EEPROM.read(2));
  Serial.print(" RX_zero: ");
  Serial.print(EEPROM.read(3));
  Serial.print(" RY_zero: ");
  Serial.println(EEPROM.read(4));
}

// 发送数据 send data
void JoyStick_SendData()
{
  ESPNOW_SendMessageBroadcast(ESPNOW_APPID_JOYSTICK, (uint8_t *)&data, sizeof(data));
}

void JoyStick_Loop()
{
  read_joydata();

  LX_to_send = map_normal(LX_read, 0, LX_zero, 255, LX_inverted);
  LY_to_send = map_normal(LY_read, 0, LY_zero, 255, LY_inverted);
  RX_to_send = map_normal(RX_read, 0, RX_zero, 255, RX_inverted);
  RY_to_send = map_normal(RY_read, 0, RY_zero, 255, RY_inverted);

  LX_to_send = constrain(LX_to_send, 0, 255);
  LY_to_send = constrain(LY_to_send, 0, 255);
  RX_to_send = constrain(RX_to_send, 0, 255);
  RY_to_send = constrain(RY_to_send, 0, 255);

  data.j1PotX = LX_to_send;
  data.j1PotY = LY_to_send;
  data.j2PotX = RX_to_send;
  data.j2PotY = RY_to_send;
  data.buttonR1 = digitalRead(R1);
  data.buttonR2 = digitalRead(R2);
  data.j1Button = digitalRead(LS);
  data.j2Button = digitalRead(RS);
  data.buttonLB = digitalRead(LB);
  data.buttonRB = digitalRead(RB);
  data.tSwitch1 = digitalRead(BK);

  /************************************************/
  // 同时按下四键及拨动开关3秒后进行摇杆原点纠偏，此时不要碰摇杆
  if (!data.buttonR1 && !data.buttonR2 && !data.tSwitch1)
  {
    delay(3000);
    if (!data.buttonR1 && !data.buttonR2 && !data.tSwitch1)
    {
      zero_test();
    }
  }

  // 如果摇杆或按键有变化，则发送新的数据
  if (compareData() != 0)
  {
    JoyStick_PrintState();
    JoyStick_SendData();
  }

  // 备份摇杆新状态
  copyData();

  delay(100);
}
