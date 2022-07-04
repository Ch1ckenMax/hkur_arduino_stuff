#include <DFRobot_MCP2515.h>

  DFRobot_MCP2515 CAN(10);                                  // Set CS pin

  void setup()
  {
      Serial.begin(115200);
      while( ! CAN.begin(CAN_500KBPS) ){   // init can bus : baudrate = 500k
        Serial.println("DFROBOT's CAN BUS Shield init fail");
        Serial.println("Please Init CAN BUS Shield again");
        delay(3000);
      }
      Serial.println("DFROBOT's CAN BUS Shield init ok!\n");

}
  const int inputPIN = A0;
  uint8_t throttlePercentage[1] = {5};
  void loop()
    {
        throttlePercentage[0] = analogRead(inputPIN);
        CAN.sendMsgBuf(0x06, 0, 1, throttlePercentage);
        Serial.println(throttlePercentage[0]);
        delay(1000);                       // send data per 100ms
    }
