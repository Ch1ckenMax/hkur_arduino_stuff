// demo: CAN-BUS Shield, send data
#include <df_can.h>
#include <SPI.h>
//COM5

const int SPI_CS_PIN = 10;

MCPCAN CAN(SPI_CS_PIN);                                    // Set CS pin

void printBin(byte aByte) {
  for (int8_t aBit = 7; aBit >= 0; aBit--)
    Serial.write(bitRead(aByte, aBit) ? '1' : '0');
}

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
const int inputPIN1 = A0;
const int inputPIN2 = A1;
INT8U TransmittPackage[8] = {0, 0, 0, 0, 0, 0, 0, 0};
int throttle = 0;

void loop()
{
  //Read data and map them to suitable range
  throttle = (analogRead(inputPIN1) + analogRead(inputPIN2));
  throttle = map(throttle, 475, 1090, 0, 255);
  //Serial.print("mapped throttle: ");
  //Serial.print(throttle);
  //Serial.print("  ");

  // Prevent overflow..
  if (throttle > 255) {
    throttle = 255;
  }
  else if (throttle < 0) {
    throttle = 0;
  }
  //Serial.print("limited throttle: ");
  //Serial.println(throttle);
  
  // putting values into the 8 bytes
  TransmittPackage[0] = throttle;

//  test if all 8 bytes can transmitt
//  for (int i=0; i < 8; i++) {
//    TransmittPackage[i] = throttle;
//  }

  // printing the whole transmittpackage
  for (int i=0; i < 8; i++) {
    //printBin(TransmittPackage[i]);
    Serial.print(TransmittPackage[i]);
    Serial.print("  ");
  }
  Serial.print("\n");
  
  //send
  CAN.sendMsgBuf(0x06, 0, 8, TransmittPackage);
  
  // comment out the delay in practical use
  delay(25);                       // send data per 100ms
}
