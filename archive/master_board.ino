 // demo: CAN-BUS Shield, send data
  #include <df_can.h>
  #include <SPI.h>
  //COM5

  const int SPI_CS_PIN = 10;

  MCPCAN CAN(SPI_CS_PIN);                                    // Set CS pin

  void setup()
  {
      Serial.begin(115200);
      int count = 50;                                     // the max numbers of initializint the CAN-BUS, if initialize failed first!.
      do {
          CAN.init();   //must initialize the Can interface here!
          if(CAN_OK == CAN.begin(CAN_500KBPS))                   // init can bus : baudrate = 500k
          {
              Serial.println("DFROBOT's CAN BUS Shield init ok!");
              break;
          }
          else
          {
              Serial.println("DFROBOT's CAN BUS Shield init fail");
              Serial.println("Please Init CAN BUS Shield again");

              delay(100);
              if (count <= 1)
                  Serial.println("Please give up trying!, trying is useless!");
          }

      }while(count--);

}
  const int inputPIN1 = A0;
  const int inputPIN2 = A1;
  INT8U throttlePercentage[2] = {5,5};
  void loop()
    {
        //Read raw data
        Serial.print(analogRead(inputPIN1));
        Serial.print(' ');
        Serial.print(analogRead(inputPIN2));
        Serial.print('\n');
        
        //throttlePercentage[0] = analogRead(inputPIN1);
        //throttlePercentage[1] = analogRead(inputPIN2);
        //CAN.sendMsgBuf(0x06, 0, 2, throttlePercentage);
        delay(25);                       // send data per 100ms
    }
