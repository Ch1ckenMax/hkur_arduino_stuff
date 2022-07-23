#include <df_can.h>
#include <SPI.h>

#define MAX_TORQUE 50 //Set the max torque of the motor here

// defining which pin to read the drivemode here
#define DRIVE_MODE_PIN 4
#define REVERSE_MODE_PIN 5

const int SPI_CS_PIN = 10;

MCPCAN CAN(SPI_CS_PIN);                                    // Set CS pin

const int inputPIN1 = A0;
const int inputPIN2 = A1;
INT8U TransmittPackage[8] = {0, 0, 0, 0, 0, 0, 0, 0};
long throttle = 0;
int driveMode = 0; // -1 = reverse, 0 = N, 1 = D
bool inverterEnable = false;
bool Forward = true; // true = forward, false = reverse

const int BEEP_INTERVAL = 1500;
#define BEEP_PIN 8
unsigned int previousMillis;
int beeped = 0;
bool beepPinState;

void millisBeep() {
  unsigned int currentMillis = millis();
  if (currentMillis - previousMillis >= BEEP_INTERVAL) {
    if (beeped == 0) {
      Serial.println("Ready to Drive Sound ON");
      digitalWrite(BEEP_PIN, HIGH);
      beeped = 1;

    }
    else {
      Serial.println("Ready to Drive Sound OFF");
      digitalWrite(BEEP_PIN, LOW);
      beeped = 2;
    }
    previousMillis = currentMillis;
  } else if (currentMillis - previousMillis <= 0) { // if millis overflow
    previousMillis = currentMillis;
  }
}

//Generates the 8 byte dataPackage to be sent to the motor controller
//Torque: in N.m. times 10, eg 30 = 3Nm
//angularVelocity: in RPM
void generateDataPackage(INT8U dataArr[], unsigned int torque, int angularVelocity, bool directionForward, bool inverter, bool inverterDischarge, bool speedMode, int torqueLimit) {
  dataArr[0] = torque % 256;
  dataArr[1] = torque / 256;
  dataArr[2] = angularVelocity % 256;
  dataArr[3] = angularVelocity / 256;
  dataArr[4] = (directionForward ? 1 : 0);
  dataArr[5] = (inverter ? 1 : 0) + 2 * (inverterDischarge ? 1 : 0) + 4 * (speedMode ? 1 : 0);
  dataArr[6] = torqueLimit % 256;
  dataArr[7] = torqueLimit / 256;
}

// print byte in binary format
void printBin(byte aByte) {
  for (int8_t aBit = 7; aBit >= 0; aBit--)
    Serial.write(bitRead(aByte, aBit) ? '1' : '0');
}

void setup()
{
  Serial.begin(115200);

  pinMode(DRIVE_MODE_PIN, INPUT_PULLUP); // setting up drive mode pins
  pinMode(REVERSE_MODE_PIN, INPUT_PULLUP); // reverse gear pin
  pinMode(BEEP_PIN, OUTPUT); // pin to trigger ready to drive sound

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

void loop()
{
  //Read data and map them to suitable range
  throttle = (analogRead(inputPIN1) + analogRead(inputPIN2));
  throttle = map(throttle, 500, 1050, 0, MAX_TORQUE);   //Max: deadzone for better stability? Shall narrow the gap between upper and lower limit?

  // Prevent overflow..
  if (throttle > MAX_TORQUE) {
    throttle = MAX_TORQUE;
  }
  else if (throttle < 0) {
    throttle = 0;
  }

  // setting up driveMode
  if (!digitalRead(4)) {
    driveMode = 1;
    inverterEnable = true;
    Forward = true;
  }
  else if (!digitalRead(5)) {
    driveMode = -1;
    inverterEnable = true;
    Forward = false;
  }
  else {
    driveMode = 0;
    inverterEnable = false;
    Forward = true;
    beeped = 0;
  }

  // ready to drive sound
  if (driveMode != 0 && beeped != 2) {
    millisBeep();
  }

  //generateDataPackage(INT8U dataArr[], int torque, int angularVelocity, bool directionForward, bool inverter, bool inverterDischarge, bool speedMode, int torqueLimit)
  generateDataPackage(TransmittPackage, throttle, 0, Forward, inverterEnable, !inverterEnable, false, 0);


  for (int i = 0; i < 8; i++) {
    //printBin(TransmittPackage[i]);
    Serial.print(TransmittPackage[i]);
    Serial.print("  ");
  }
  Serial.print("\n");

  //send
  CAN.sendMsgBuf(0x0c0, 0, 8, TransmittPackage);

  // comment out the delay in practical use
  delay(25);         // send data per 25ms
}
