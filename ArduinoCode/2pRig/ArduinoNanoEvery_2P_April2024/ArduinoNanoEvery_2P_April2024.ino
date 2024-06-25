// Serial output has this format - each line has ('\t separated/): 
//     1. Wheel counter
//     2. Lick Left 
//     3. Lick Right
//     4. Async Pulse
//     5. Photodiode Value
//     6. 2p frame count
//     7. 2p line count
//     8. Arduino time stamp
//
// Last edit April 2024
// Aman and Sonali 

// did not update new valve open condition from Edd's recent code - AS 29/04/2024

#include <Event.h>
#include <Timer.h>

#define LickPinL 2 // digital pin of lick detector
#define LickPinR 3 // digital pin of lick detector
#define encoder0PinA 18          // sensor A of rotary encoder
#define encoder0PinB 19          // sensor B of rotary encoder
#define SValvePinL 11             // digital pin controlling the left solenoid valve
#define SValvePinR 12            // digital pin controlling the right solenoid valve
#define SyncPin 10               // sync pulse pin
#define PhotodiodePin 17         // photoiode analog input 
#define FrameTrig 9         // frame trigger digital input 
#define LineTrig 8         // line trigger digital input 

// serial frequency variables
float samplingFrequency = 100; // frequency to send new values to computer
const long interval = 1000 / (samplingFrequency);  // sampling interval to send new values
unsigned long startMillis;  // sample timer that resets each time new data is sent
unsigned long currentMillis; // rolling timer to check if it's time to send new data

// variables for rotary encoder
volatile unsigned int encoder0Pos = 0;    // variable for counting ticks of rotary encoder
boolean A_set;
boolean B_set;

// event time variables (async pulse, licks)
unsigned long lastSyncPulseTime;    // updates each time async pulse goes HIGH
unsigned long lastLeftLickTime;  // updates each time left lick detector goes LOW
unsigned long lastRightLickTime;   // updates each time right lick detector goes LOW

// variables for frame trigger and line trigger counters 
unsigned long last2pFrameTime;   // updates each time 2p frame goes LOW
unsigned long last2pLineTime;   // updates each time async pulse goes HIGH

// variables for pinch valve of reward system
const byte numChars = 10;
char receivedChars[numChars];   // an array to store the received data
boolean newData = false;

// left reward
char rewardTimeL[6];   // an array to store the received data
bool newRewardL = false;
int TimeONL = 0;             // new for this version
int TempVarL = 0;
bool TimerFinishedL = false;
uint32_t StartTimeL = 0;      // variable to store temporary timestamps of previous iteration of the while loop

// right reward
char rewardTimeR[6];   // an array to store the received data
bool newRewardR = false;
int TimeONR = 0;             // new for this version
int TempVarR = 0;
bool TimerFinishedR = false;
uint32_t StartTimeR = 0;      // variable to store temporary timestamps of previous iteration of the while loop

// variable for photodiode
int PhotodiodeVal;

//volatile unsigned int FrameCount = 0;      // variable for counting 2p frames
//unsigned int tmp_FrameCount = 0;           // temporary variable for counting 2p frames
//volatile unsigned int LineCount = 0;      // variable for counting line scanner
//unsigned int tmp_LineCount = 0;           // temporary variable for counting line scanner

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////


void setup() {

  pinMode(encoder0PinA, INPUT);         // rotary encoder sensor A
  pinMode(encoder0PinB, INPUT);         // rotary encoder sensor B
  
  pinMode(LickPinL, INPUT_PULLUP);              // lick detector for left port
  pinMode(LickPinR, INPUT_PULLUP);              // lick detector for right port
  
  pinMode(FrameTrig, INPUT);              // frame trigger 
  pinMode(LineTrig, INPUT);              // line trigger

  pinMode(SValvePinL, OUTPUT);           // solenoid valve for left port
  pinMode(SValvePinR, OUTPUT);          // solenoid valve for right port
  
  pinMode(SyncPin, INPUT);              // sync pulse
  pinMode(PhotodiodePin, INPUT);        // PhotoDiode

  // interrupts for rotary encoder
  attachInterrupt(digitalPinToInterrupt(encoder0PinA), doEncoderA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoder0PinB), doEncoderB, CHANGE);
  
  // interrupt for lick detector
  attachInterrupt(digitalPinToInterrupt(LickPinL), Lick_CounterL, FALLING);
  attachInterrupt(digitalPinToInterrupt(LickPinR), Lick_CounterR, FALLING);

  // interrupt for frame and line scanner trigger 
  attachInterrupt(digitalPinToInterrupt(FrameTrig), Frame_Counter, FALLING);
  attachInterrupt(digitalPinToInterrupt(LineTrig), Line_Counter, RISING);

  // interrupt for sync pulse
  attachInterrupt(digitalPinToInterrupt(SyncPin), SyncPulse_Receiver, CHANGE);
  
  Serial.begin (1000000);
  Serial.setTimeout(5);

  delay(500);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void loop() {
  //Check for change in position and send to Serial buffer

    currentMillis = millis();  //get the current "time" (actually the number of milliseconds since the program started)
  if (currentMillis - startMillis >= interval)  //test whether the period has elapsed
  {
    Serial.print(encoder0Pos);// Wheel raw input
    Serial.print("\t");
    Serial.print(lastLeftLickTime);// Left lick count
    Serial.print("\t");
    Serial.print(lastRightLickTime);// Right lick count
    Serial.print("\t");
    Serial.print(lastSyncPulseTime);// Sync pulse high/low status
    Serial.print("\t");
    Serial.print(PhotodiodeVal);
    Serial.print("\t");
    Serial.print(last2pFrameTime);// 2p frame count
    Serial.print("\t");
    Serial.print(last2pLineTime);// line scanner count
    Serial.print("\t");
    Serial.print(currentMillis);// Arduino timestamp
    Serial.print("\n");
    startMillis = currentMillis;  // update timer
  }

    

  GetSerialInput();
  ActivatePVL();
  ActivatePVR();
  PhotodiodeVal = analogRead(PhotodiodePin);    // read the input pin


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


  }
}


  
/////////////////////////////////////////////////////////////////////////////
// Timer for the output signal to the pinch valve to stay high

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

/////////////////////////////////////
/// Wheel pos recording functions ///
/////////////////////////////////////

void doEncoderA() { // Interrupt on A changing state
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

void doEncoderB() { // Interrupt on B changing state
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

/////////////////////////////////////
/// Lick time recording functions ///
/////////////////////////////////////

void Lick_CounterL() { // Interrupt for when IR beam breaking circuit goes LOW
    lastLeftLickTime = millis();
}

void Lick_CounterR() {
    lastRightLickTime = millis();
}

/////////////////////////////////////////
/// 2p frame time recording functions ///
/////////////////////////////////////////

void Frame_Counter() { // Interrupt for when frame trigger goes down and line trigger goes LOW
  last2pFrameTime = millis();
}

void Line_Counter() { // Interrupt for when frame trigger goes down and line trigger goes HIGH
  last2pLineTime = millis();
}

///////////////////////////////////////
/// aSync pulse recording functions ///
///////////////////////////////////////
// Interrupt for when sync signal goes HIGH
void SyncPulse_Receiver() {
  lastSyncPulseTime = millis();
}


