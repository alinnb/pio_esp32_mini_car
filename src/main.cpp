#include "joystick.h"

//**********************Setup*************************
void setup()
{
  Serial.begin(115200);
  delay(100);

  JoyStick_Init();
  JoyStick_PrintState();
}

void loop()
{
  JoyStick_Loop();
}
