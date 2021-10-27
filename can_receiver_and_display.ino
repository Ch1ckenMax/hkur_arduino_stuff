#include <MCUFRIEND_kbv.h>
#include <SPI.h>
#include "df_can.h"
#define BLACK    0x0000
#define BLUE     0x001F
#define RED      0xF800
#define GREEN    0x07E0
#define CYAN     0x07FF
#define MAGENTA  0xF81F
#define YELLOW   0xFFE0 
#define WHITE    0xFFFF

const int SPI_CS_PIN = 10;
MCPCAN CAN(SPI_CS_PIN);                                    // Set CS pin
MCUFRIEND_kbv tft;    // initialize screen

void setup()
{
    Serial.begin(115200);
    tft.reset();
    uint16_t identifier = tft.readID();
    Serial.print("ID = 0x");
    Serial.println(identifier, HEX);
    if (identifier == 0xEFEF) identifier = 0x9486;
    tft.begin(identifier);
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

void loop()
{
    unsigned char len = 0;
    unsigned char buf[8];

    // Helps refresh the screen
    tft.setRotation(1);
    tft.fillScreen(MAGENTA);
    tft.setCursor(0, 0);
    tft.setTextSize(3);

    // CAN Bus stuff
    if(CAN_MSGAVAIL == CAN.checkReceive())            // check if data coming
    {
        CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf

        for(int i = 0; i < len; i++)    // print the data
        {
            Serial.write(buf[i]);
            Serial.print("\t");
            tft.print("doggo: ");
            tft.println(buf[i]);
            tft.print("  /\\_/\\ \n ( o.o )\n  > ^ <");

            delay(1000);
        }
        Serial.println();
    }
    else {
      tft.print("NO SIGNAL");
      delay(1000);
    }
}
