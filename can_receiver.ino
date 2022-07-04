#include <SPI.h>
#include "df_can.h"
//COM4

const int SPI_CS_PIN = 10;
MCPCAN CAN(SPI_CS_PIN);                                    // Set CS pin

void setup()
{
  Serial.begin(115200);
  int count = 50;                                     // the max numbers of initializint the CAN-BUS, if initialize failed first!.
  do {
    CAN.init();   //must initialize the Can interface here!
    if (CAN_OK == CAN.begin(CAN_250KBPS))                  // init can bus : baudrate = 500k
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

  } while (count--);

}

INT8U buf[8] = {0,0,0,0,0,0,0,0};
INT8U len = 8;

void loop()
{
  // CAN Bus stuff
  if (CAN_MSGAVAIL == CAN.checkReceive())           // check if data coming
  {
    CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf
    for (int i = 0; i < len; i++)   // print the data
    {
      Serial.print((int) buf[i]);
      Serial.print("\t");

      //delay(250);
    }
    Serial.println();
  }

}
