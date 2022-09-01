#include <df_can.h>
#include <SPI.h>


//Calibration algorithm (correctness not checked)
//Variable calibrationProcess: int 0, 1, 2 (0: not calibrated, 1: only calibrated lift, 2: calibrated all)
//calibrationinProgress = false
//calibrationButton = digitalRead(some pins idk)
//if 0:
  //if calibration not in progress
    //if calibrationButton is pressed
      //change calibrationinprogress to true
    //else
      //sends signal to driver that shits need calibration, press the button to start calibrating
  //else
    //send signal to dashboard dont touch shit
    //record the highest number recorded from both APPS
    //wait for 100ms
    //if already 100ms
      //calibration in progress to false
      //calibration process increment by 1
      //throttle low threshold to apps value
  //ends the loop function so throttle and stuff wont be triggered
//if 1:
  //same story as 0
//if 2:
  //good to go, no worry about anything
    

//---Variables for config

  //CAN bus config
  const uint8_t SPI_CS_PIN = 10;
  MCPCAN CAN(SPI_CS_PIN);
  #define CANBaudRate CAN_250KBPS

  // Drive mode config
  const uint8_t DRIVE_MODE_PIN = 4;
  const uint8_t REVERSE_MODE_PIN = 5;

  //Beep config
  const uint8_t BEEP_PIN = 8;
  const int BEEP_INTERVAL = 1000;

  //Throttle config
  const int MAX_TORQUE = 50; //Set the max torque of the motor here
  const uint8_t THROTTLE_PIN_A = A0;
  const uint8_t THROTTLE_PIN_B = A1;
  const unsigned int THROTTLE_LOWER_BOUND = 90;
  const unsigned int THROTTLE_UPPER_BOUND = 700;

// ---Other Variables

  //Data to CAN
  INT8U TransmittPackage[8] = {0, 0, 0, 0, 0, 0, 0, 0};

  //Throttle
  unsigned int APPS1 = 0;
  unsigned int APPS2 = 0; 
  long throttle = 0;
  uint8_t driveMode = 0; // 0 = reverse, 1 = neutral, 2 = drive
  bool inverterEnable = false;
  bool forward = true; // true = forward, false = reverse

  //Ready To Drive Sound
  unsigned long previousMillis_beep;
  uint8_t beeped = 0; // 0 = no beeping, 1 = beeping, 2 = beeping ended

  //Implausible
  bool implausibleEngineStop = false;
  bool implausibleInProgress = false;
  unsigned long previousMillis_APPS = 0;

void millisBeep() {
  unsigned long currentMillis_beep = millis();
  if (beeped == 0) {
    Serial.println("Ready to Drive Sound ON");
    digitalWrite(BEEP_PIN, HIGH);
    beeped = 1;
    previousMillis_beep = currentMillis_beep;
  }
  else if (currentMillis_beep - previousMillis_beep >= BEEP_INTERVAL) {
    digitalWrite(BEEP_PIN, LOW);
    Serial.println("Ready to Drive Sound OFF");
    beeped = 2;
    return;
  }
}

bool SCSFailure(){
  //Check range

  //Check out of range signal

  //Check data corruption

  //Check loss or delay of message

  //Check other SCS failures

  //If true, send safe or error signals?

  return false; //Placeholder
}

//Check if throttle input is implausible according to rulebook T11.8.8 and T11.8.9
//param APPS1, APPS2: raw input data
//param range: the range for pedal travel
bool implausible(){
    bool hasThrottleDeviaion = (abs(APPS1 - APPS2)/(float) THROTTLE_UPPER_BOUND - THROTTLE_LOWER_BOUND) >= 0.1 ;
    return hasThrottleDeviaion || SCSFailure();
}

//According to the rulebook T11.8.8 and T11.8.9
void millisAppsImplausibility(){ 
  if(!implausible()){
    implausibleInProgress = false;}
  else{
    if(implausibleInProgress){
      if(millis() - previousMillis_APPS > 80)
        Serial.println("STOP THE ENGINE!!!!");
        implausibleEngineStop = true; //Now in engine stopped state
    }
    else{
      implausibleInProgress = true;
      previousMillis_APPS = millis();
    }
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
  APPS1 = analogRead(THROTTLE_PIN_A) - 305;
  APPS2 = analogRead(THROTTLE_PIN_B);
  millisAppsImplausibility();
  if(implausibleEngineStop){
    //Resolve the implausible
    //Tell engine to fucking stop
    //Sends info to dashboard
    //Any conditions to make implausibleEngineStop false again?
    return; //Dont go below
  }
  Serial.print(analogRead(THROTTLE_PIN_A));
  Serial.print("  ");
  Serial.println(analogRead(THROTTLE_PIN_B));
  Serial.print(" ");
  throttle = map((APPS1 + APPS2)/2, THROTTLE_LOWER_BOUND, THROTTLE_UPPER_BOUND, 0, MAX_TORQUE);
  Serial.println(throttle);

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
    inverterEnable = throttle >= 2;
    forward = true;
  }
  else if (!digitalRead(REVERSE_MODE_PIN)) {
    driveMode = 0;
    inverterEnable = throttle >= 2;
    forward = false;
  }
  else {
    driveMode = 1;
    inverterEnable = false;
    forward = true;
    if(beeped == 1){ //if its beeping, turn it off
      digitalWrite(BEEP_PIN, LOW);
      Serial.println("Ready to Drive Sound OFF");
    }
    beeped = 0;
  }

  // ready to drive sound
  if (driveMode != 1 && beeped != 2) {
    millisBeep();
  }

  //generateDataPackage(INT8U dataArr[], int torque, int angularVelocity, bool directionForward, bool inverter, bool inverterDischarge, bool speedMode, int torqueLimit)
  generateDataPackage(TransmittPackage, throttle, 0, forward, inverterEnable, !inverterEnable, false, 0);

  //Debug message in serial
  //for (int i = 0; i < 8; i++) {
   //Serial.print(TransmittPackage[i]);
    //Serial.print("  ");
  //}
  //Serial.print("\n");

  CAN.sendMsgBuf(0x0c0, 0, 8, TransmittPackage); //send
}


  