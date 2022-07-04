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
  const int inputPIN = A0;
  INT8U throttlePercentage[1] = {5};
  void loop()
    {
        throttlePercentage[0] = analogRead(inputPIN);
        CAN.sendMsgBuf(0x06, 0, 1, throttlePercentage);
        delay(1000);                       // send data per 100ms
    }
