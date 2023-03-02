



/*
  Tomaso Muzzu - UCL - 25 May 2017
  Script to communicate with the following devices from Matlab:
  - rotary encoder with quadrature encoding of position. Model Kubler 05.2400.1122.1024 (READ)
  - pintch valve for water reward. Model NResearch 225P011-21 (WRITE)
  - lick detector based on IR beam breaking circuit. Model OP550 and IR26-21C-L110-TR8 (READ)
*/
/* 22 Jan 2018
  read out of the sync pulse for syncing VR with ePhys
  sync pulse signal is read via an interrupt and I store the time in ms
  timestamp of the last 0->1 signal transition is sent to Matlab
*/
/* ***** NOTDUE 2 lick port VERSION -> Serial -> SErial. 2 lick counters and svalves******* */

#include <Event.h>
#include <Timer.h>

// IMPORTANT: because we are using interrupts, it's important to assign the lickpins and encoder pins as follows. 
// Unfortunately 0 and 1 don't work on arduino during serial, so we can only get one encoder input (forwards only!, sucks!)
#define LickPinL 2 // digital pin of lick detector
#define LickPinR 3 // digital pin of lick detector
#define encoder0PinA 7          // sensor A of rotary encoder
#define encoder0PinB 3          // sensor B of rotary encoder
#define SValvePinL 11             // digital pin controlling the solenoid valve
#define SValvePinR 12
//#define SyncPin 10               // sync pulse pin
//#define EyeCameraTrPin_IN 12    // trigger pin to start sending pulse to the eye camera
//#define EyeCameraTrPin_OUT 13   // trigger to eye tracking camera
//#define RecCameraTrPin_IN 1     // register recording camera frame times
//#define RecCameraTrPin_OUT 8    // trigger pin to start/stop recording camera

// variables for rotary encoder
volatile unsigned int encoder0Pos = 0;    // variable for counting ticks of rotary encoder
unsigned int tmp_Pos = 1;                 // variable for counting ticks of rotary encoder
boolean A_set;
boolean B_set;

// variables for lick counters
volatile unsigned int LickCountL = 0;      // variable for counting licks
unsigned int tmp_LickCountL = 0;           // temporary variable for counting licks
volatile unsigned int LickCountR = 0;      // variable for counting licks
unsigned int tmp_LickCountR = 0;           // temporary variable for counting licks

// variables for pinch valve of reward system
const byte numChars = 10;
char receivedChars[numChars];   // an array to store the received data
boolean newData = false;
// left reward
char rewardTimeL[6];   // an array to store the received data
boolean newRewardL = false;
int TimeONL = 0;             // new for this version
int TempVarL = 0;
boolean TimerFinishedL = false;
uint32_t StartTimeL = 0;      // variable to store temporary timestamps of previous iteration of the while loop

// right reward
char rewardTimeR[6];   // an array to store the received data
boolean newRewardR = false;
int TimeONR = 0;             // new for this version
int TempVarR = 0;
boolean TimerFinishedR = false;
uint32_t StartTimeR = 0;      // variable to store temporary timestamps of previous iteration of the while loop


// variable for sync pulse
volatile unsigned int PinStatus = 0;      // variable for transitions
uint32_t TempTime = 0;           // variable to store time when sync pulse goes up
volatile unsigned Delta_t = 0;            // variable to store intervals between 0->1 transitions

// variables for eye tracking camera trigger
boolean cameraState = false;
int VFR = 50; // frame rate of eye tracking camera
int VFR_T = 1000/VFR; // inverse of frame rate in ms
int EyeTr_dur_ON = 5; // duration in ms of the ttl pulse for the eye camera
char frameRate[6];
// variable for recording camera trigger recording
volatile unsigned int FrameCount = 0;    // variable for counting frames
uint32_t FrameTime = 0;            // variable to store intervals between 0->1 transitions
unsigned int tmp_FrameCount = 0;           // temporary variable for counting Frames

// delete
////volatile unsigned int RecCamPinStatus = 0;      // variable for transitions
////uint32_t RecCamTempTime = 0;           // variable to store time when sync pulse goes up
////volatile unsigned RecCamDelta_t = 0;            // variable to store intervals between transitions
//// unsigned int tmp_FrameCount = 0;           // temporary variable for counting licks

void setup() {

  pinMode(encoder0PinA, INPUT);         // rotary encoder sensor A
  pinMode(encoder0PinB, INPUT);         // rotary encoder sensor B
  
  pinMode(LickPinL, INPUT_PULLUP);              // lick detector for left port
  pinMode(LickPinR, INPUT_PULLUP);              // lick detector for right port
  
  pinMode(SValvePinL, OUTPUT);           // solenoid valve for left port
  pinMode(SValvePinR, OUTPUT);          // solenoid valve for right port
  
  //pinMode(SyncPin, INPUT);              // sync pulse
//  pinMode(EyeCameraTrPin_IN, INPUT);    // trigger in for camera
 // pinMode(EyeCameraTrPin_OUT, OUTPUT);  // eye camera trigger
 // pinMode(RecCameraTrPin_OUT, OUTPUT);  // trigger to start/stop camera
 // pinMode(RecCameraTrPin_IN, INPUT);    // eye camera register frame time (trigger)
  
  // interrupts for rotary encoder
  attachInterrupt(digitalPinToInterrupt(encoder0PinA), doEncoderA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoder0PinB), doEncoderB, CHANGE);
  
  // interrupt for lick detector
  attachInterrupt(digitalPinToInterrupt(LickPinL), Lick_CounterL, FALLING);
  attachInterrupt(digitalPinToInterrupt(LickPinR), Lick_CounterR, FALLING);

  // interrupt for sync pulse
  //attachInterrupt(digitalPinToInterrupt(SyncPin), SyncPulse_Receiver, CHANGE);
  // interrupt for recording camera pulse counter
  //attachInterrupt(digitalPinToInterrupt(RecCameraTrPin_IN), RecCameraPulse_Receiver, CHANGE);
  
  Serial.begin (250000);
  Serial.setTimeout(5);

  delay(500);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void loop() {
  //Check for change in position and send to Serial buffer

  if (tmp_Pos != encoder0Pos || (tmp_LickCountL != LickCountL) || (tmp_LickCountR != LickCountR)) { // || (tmp_FrameCount != FrameCount)) { //should I add a FrameCount condition and create a temp_FrameCount as well?
    Serial.print(encoder0Pos);//
    Serial.print("\t");
    Serial.print(LickCountL);//
    Serial.print("\t");
    Serial.print(LickCountR);//
    //Serial.print("\t");
    //Serial.print(PinStatus);
    //Serial.print("\t");
    //Serial.print(FrameCount);
    //Serial.print("\t");
    //Serial.print(FrameTime);
    //Serial.print("\t");
    //Serial.print(receivedChars);
    Serial.print("\n");
    tmp_Pos = encoder0Pos;
    tmp_LickCountL = LickCountL;
    tmp_LickCountR = LickCountR;
    //tmp_FrameCount = FrameCount;
  }
  else {
    Serial.print(tmp_Pos);//
    Serial.print("\t");
    Serial.print(tmp_LickCountL);//
    Serial.print("\t");
    Serial.print(tmp_LickCountR);//
    //Serial.print("\t");
    //Serial.print(PinStatus);
    //Serial.print("\t");
    //Serial.print(tmp_FrameCount);
    //Serial.print("\t");
    //Serial.print(FrameTime);
    //Serial.print("\t");
    //Serial.print(receivedChars);
    Serial.print("\n");
  }

  GetSerialInput();
  ActivatePVL();
  ActivatePVR();
  //TriggerCamera();

  //delay(1);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Read inputs from Matlab. It is an integer specifing the amount of time in ms the pintch valve stays open
void GetSerialInput() { // part of code taken from http://forum.arduino.cc/index.php?topic=396450.0
  static byte ndx = 0;
  char endMarker = '\r';
  char rc;

  if (Serial.available() > 0) {
    rc = Serial.read();

    if (rc != endMarker) {
      receivedChars[ndx] = rc;
      ndx++;
      if (ndx >= numChars) {
        ndx = numChars - 1;
      }
    }
    else {
      receivedChars[ndx] = '\0'; // terminate the string
      ndx = 0;
      newData = true;
    }
    
    if (receivedChars[0] == 'r'){
      for(int i=0;i<5; i=i+1){
        rewardTimeR[i] = receivedChars[i+1];
      }
        newRewardR = true;
      }

    if (receivedChars[0] == 'l'){
      for(int i=0;i<5; i=i+1){
        rewardTimeL[i] = receivedChars[i+1];
      }
        newRewardL = true;  
      }

//    if (receivedChars[0] == 'c'){
//      toggleCameraState(); // either 0 or 1
//    }
//    if (receivedChars[0] == 'f'){
//     for(int i=0;i<5; i=i+1){
//        frameRate[i] = receivedChars[i+1];
//      }
//     VFR = atoi(frameRate);
//     VFR_T = 1000/VFR; // inverse of frame rate in ms
//    }
  }
}

//void toggleCameraState() {
//  if (receivedChars[1] == '1') {  // strcmp(cameraStateInput,"1")
//    cameraState = true;
//    digitalWrite(RecCameraTrPin_OUT, HIGH);
//  }
//  if (receivedChars[1] == '0') { // strcmp(cameraStateInput,"0")
//    cameraState = false;
//    digitalWrite(RecCameraTrPin_OUT, LOW);
//  }
//}
  
/////////////////////////////////////////////////////////////////////////////
// Timer for the output signal to the pintch valve to stay high

void ActivatePVL() {
  if (newRewardL == true) {
    TimeONL = 0;             // zero previous time
    TimeONL = atoi(rewardTimeL);   // convert array of chars to integers
    if (TempVarL == 0) {
      StartTimeL = millis();
      TempVarL = 1;
    }
    newData = false;
    newRewardL = false;
  }
  if (TempVarL == 1) {
    // start checking the time and keep the valve open as long as you wish irrespective of what happens to Serial.read()
    if ((millis() - StartTimeL) <= (uint32_t)TimeONL) {
      digitalWrite(SValvePinL, HIGH); // open valve
      TimerFinishedL = false;
      TempVarL = 1;
    }
    else {
      digitalWrite(SValvePinL, LOW); // close valve
      TimerFinishedL = true;
      TempVarL = 0;
    }
  }
}

void ActivatePVR() {
  if (newRewardR == true) {
    TimeONR = 0;             // zero previous time
    TimeONR = atoi(rewardTimeR);   // convert array of chars to integers
    if (TempVarR == 0) {
      StartTimeR = millis();
      TempVarR = 1;
    }
    newData = false;
    newRewardR = false;
  }
  if (TempVarR == 1) {
    // start checking the time and keep the valve open as long as you wish irrespective of what happens to Serial.read()
    if ((millis() - StartTimeR) <= (uint32_t)TimeONR) {
      digitalWrite(SValvePinR, HIGH); // open valve
      TimerFinishedR = false;
      TempVarR = 1;
    }
    else {
      digitalWrite(SValvePinR, LOW); // close valve
      TimerFinishedR = true;
      TempVarR = 0;
    }
  }
}

/////////////////////////////////////////////////////////////////////////////
// trigger signal to acquire frames through eye camera. Get Start/Stop via hardware from other Arduino
//void TriggerCamera() {
//  if (cameraState == true) {
//  // if (tmp_Pos>5000) {
//    if (millis()%VFR_T <= EyeTr_dur_ON){
//      digitalWrite(EyeCameraTrPin_OUT, HIGH); // trigger a frame exposure to the camera 
//    }
//    else {
//      digitalWrite(EyeCameraTrPin_OUT, LOW); // make sure the camera does not acquire anything
//    }
//  }
//  else {
//    digitalWrite(EyeCameraTrPin_OUT, LOW); // make sure the camera does not acquire anything
//  }
//}

/////////////////////////////////////////////////////////////////////////////
// Interrupt on A changing state
void doEncoderA() {
  // Low to High transition?
  if (digitalRead(encoder0PinA) == HIGH) {
    A_set = true;
    if (!B_set) {
      encoder0Pos = encoder0Pos + 1;
    }
  }
  // High-to-low transition?
  if (digitalRead(encoder0PinA) == LOW) {
    A_set = false;
  }
}
// Interrupt on B changing state
void doEncoderB() {
  // Low-to-high transition?
  if (digitalRead(encoder0PinB) == HIGH) {
    B_set = true;
    if (!A_set) {
      encoder0Pos = encoder0Pos - 1;
    }
  }
  // High-to-low transition?
  if (digitalRead(encoder0PinB) == LOW) {
    B_set = false;
  }
}

/////////////////////////////////////////////////////////////////////////////
// Interrupt for when IR beam breaking circuit goes down
void Lick_CounterL() {
  // High-to-low transition?
  if (digitalRead(LickPinL) == LOW) {LickCountL = LickCountL + 1;}
}

void Lick_CounterR() {
  // High-to-low transition?
  if (digitalRead(LickPinR) == LOW) {LickCountR = LickCountR + 1;}
}


/////////////////////////////////////////////////////////////////////////////
// Interrupt for when sync signal goes up
//void SyncPulse_Receiver() {
//  // low-to-high transition?
//  if (digitalRead(SyncPin) == HIGH) {
//    PinStatus = 1;
//    // Delta_t = millis()- TempTime;
//    // TempTime = millis();
//  }
//  else if (digitalRead(SyncPin) == LOW) {
//    PinStatus = 0;
//  }
//}

/////////////////////////////////////////////////////////////////////////////
// Interrupt for when sync signal goes up
// void RecCameraPulse_Receiver() {
//  // CHANGE transition?
//   if (digitalRead(RecCameraTrPin_IN) == CHANGE) {
//     FrameCount = FrameCount + 1;
//     FrameTime = millis();
//   }
//
//}

// EOF
