#include <df_can.h>
#include <SPI.h>

//CAN bus config
const uint8_t SPI_CS_PIN = 10;
MCPCAN CAN(SPI_CS_PIN);
#define CANBaudRate CAN_250KBPS

// Drive mode config
const uint8_t DRIVE_MODE_PIN = 4;
const uint8_t REVERSE_MODE_PIN = 5;

//Beep config
const uint8_t BEEP_PIN = 8;
const int BEEP_INTERVAL = 1500;

//Throttle config
const int MAX_TORQUE = 50; //Set the max torque of the motor here
const uint8_t THROTTLE_PIN_A = A0;
const uint8_t THROTTLE_PIN_B = A1;

//Other Variables
INT8U TransmittPackage[8] = {0, 0, 0, 0, 0, 0, 0, 0};
long throttle = 0;
uint8_t driveMode = 0; // 0 = reverse, 1 = neutral, 2 = drive
bool inverterEnable = false;
bool forward = true; // true = forward, false = reverse
unsigned long previousMillis;
uint8_t beeped = 0; // 0 = not beeped yet, 1 = beeping, 2 = beeping ended

void millisBeep() {
  unsigned long currentMillis = millis();
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

void setup()
{
  //PinModes
  pinMode(DRIVE_MODE_PIN, INPUT_PULLUP); // setting up drive mode pins
  pinMode(REVERSE_MODE_PIN, INPUT_PULLUP); // reverse gear pin
  pinMode(BEEP_PIN, OUTPUT); // pin to trigger ready to drive sound

  //CAN bus connection initalization
  Serial.begin(115200);
  uint8_t count = 50;                                     // the max numbers of initializint the CAN-BUS, if initialize failed first!.
  do {
    CAN.init();   //must initialize the Can interface here!
    if (CAN_OK == CAN.begin(CANBaudRate))                  // init can bus : baudrate = 500k
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
  throttle = map(analogRead(THROTTLE_PIN_A) + analogRead(THROTTLE_PIN_B), 500, 1050, 0, MAX_TORQUE);

  // Prevent overflow..
  if (throttle > MAX_TORQUE) {
    throttle = MAX_TORQUE;
  }
  else if (throttle < 0) {
    throttle = 0;
  }

  // setting up driveMode
  if (!digitalRead(DRIVE_MODE_PIN)) {
    driveMode = 2;
    inverterEnable = true;
    forward = true;
  }
  else if (!digitalRead(REVERSE_MODE_PIN)) {
    driveMode = 0;
    inverterEnable = true;
    forward = false;
  }
  else {
    driveMode = 1;
    inverterEnable = false;
    forward = true;
    beeped = 0;
  }

  // ready to drive sound
  if (driveMode != 1 && beeped != 2) {
    millisBeep();
  }

  //generateDataPackage(INT8U dataArr[], int torque, int angularVelocity, bool directionForward, bool inverter, bool inverterDischarge, bool speedMode, int torqueLimit)
  generateDataPackage(TransmittPackage, throttle, 0, forward, inverterEnable, !inverterEnable, false, 0);

  //Debug message in serial
  for (int i = 0; i < 8; i++) {
    Serial.print(TransmittPackage[i]);
    Serial.print("  ");
  }
  Serial.print("\n");

  CAN.sendMsgBuf(0x0c0, 0, 8, TransmittPackage); //send
}
