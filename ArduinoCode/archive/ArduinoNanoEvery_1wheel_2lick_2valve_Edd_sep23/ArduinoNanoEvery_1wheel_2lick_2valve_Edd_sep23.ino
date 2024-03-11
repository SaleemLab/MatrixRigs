#include <Event.h>
#include <Timer.h>


#define LickPinL 2 // digital pin of lick detector
#define LickPinR 3 // 

#define encoder0PinA 18          // sensor A of rotary encoder
#define encoder0PinB 19          // sensor B of rotary encoder

#define SValvePinL 11             // digital pin controlling the solenoid valve
#define SValvePinR 12


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

void setup() {

  pinMode(encoder0PinA, INPUT);         // rotary encoder sensor A
  pinMode(encoder0PinB, INPUT);         // rotary encoder sensor B
  
  pinMode(LickPinL, INPUT_PULLUP);              // lick detector for left port
  pinMode(LickPinR, INPUT_PULLUP);              // lick detector for right port
  
  pinMode(SValvePinL, OUTPUT);           // solenoid valve for left port
  pinMode(SValvePinR, OUTPUT);          // solenoid valve for right port
  
  
  // interrupts for rotary encoder
  attachInterrupt(digitalPinToInterrupt(encoder0PinA), doEncoderA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoder0PinB), doEncoderB, CHANGE);
  
  // interrupt for lick detector
  attachInterrupt(digitalPinToInterrupt(LickPinL), Lick_CounterL, FALLING);
  attachInterrupt(digitalPinToInterrupt(LickPinR), Lick_CounterR, FALLING);


  Serial.begin (1000000);
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
    Serial.print("\n");

    tmp_Pos = encoder0Pos;
    tmp_LickCountL = LickCountL;
    tmp_LickCountR = LickCountR;
  }
  else {
    Serial.print(tmp_Pos);//
    Serial.print("\t");
    Serial.print(tmp_LickCountL);//
    Serial.print("\t");
    Serial.print(tmp_LickCountR);//
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
