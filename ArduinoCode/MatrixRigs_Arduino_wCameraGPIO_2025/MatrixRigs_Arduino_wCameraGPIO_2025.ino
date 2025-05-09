// define the pins that hardware are connected to
#define LickPinL 2        // digital pin of lick detector
#define LickPinR 3        // digital pin of lick detector
#define WheelPin1 18      // sensor A of rotary encoder (GREEN)
#define WheelPin2 19      // sensor B of rotary encoder (GREY)
#define SValvePinL 11     // digital pin controlling the left solenoid valve
#define SValvePinR 12     // digital pin controlling the right solenoid valve
#define SyncPin 6        // sync pulse pin
#define FaceCamPin 8    // frame gpio for face cam
#define BodyCamPin 9    // frame gpio for body cam
#define PhotodiodePin A0  // photoiode analog input

// Serial communication variables
float samplingFrequency = 120;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                ; // frequency to send new values to computer
const long interval = 1000 / (samplingFrequency);  // sampling interval to send new values
const byte numChars = 30;
char receivedChars[numChars];  // an array to store the received data
bool newData = false;
String FirstChar;
unsigned long startMillis;  // sample timer that resets each time new data is sent
unsigned long currentMillis; // rolling timer to check if it's time to send new data
unsigned long previousMillis; // sample timer that resets every time sync pulse changes

// variables for rotary encoder
volatile long encoderCount = 0;    // Stores the count from the encoder
volatile uint8_t lastEncoded = 0;  // Holds the previous state of the encoder


// event time variables (async pulse, camera frame times)
volatile unsigned long lastSyncPulseTime;    // updates each time async pulse goes HIGH
volatile unsigned long lastBodyCamFrameTime;
volatile unsigned long lastFaceCamFrameTime;
//unsigned long lastLeftLickTime;  // [not currently used] updates each time left lick detector goes LOW
//unsigned long lastRightLickTime;   // updates each time right lick detector goes LOW



// lick counter variables
volatile unsigned int LickCountL = 0;
volatile unsigned int LickCountR = 0;

// left reward vars
int rewardTimeL; // an array to store the received data
bool newRewardL = false; // bool flag for if a new reward is requested on this valve
bool ValveOpenL = false; // bool flag for whether valve is currently open
uint32_t StartTimeL = 0;  // start timer for when valve first opens
bool TimerFinishedL = false; // bool flag for whether the valve open time has ended

// right reward vars
int rewardTimeR;  // an array to store the received data
boolean newRewardR = false;
bool ValveOpenR = false;
uint32_t StartTimeR = 0;  
bool TimerFinishedR = false;

// variable for photodiode value
int PhotodiodeVal;

// sync pulse vars
unsigned long waitTime = 0;
bool isSyncHigh = false;
int syncPulseOnTime = 500;
int syncPulseLowerBound = 1000;
int syncPulseUpperBound = 3500;

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

void setup() {

  pinMode(WheelPin1, INPUT_PULLUP);  // rotary encoder sensor A
  pinMode(WheelPin2, INPUT_PULLUP);  // rotary encoder sensor B

  pinMode(LickPinL, INPUT_PULLUP);  // lick detector for left port
  pinMode(LickPinR, INPUT_PULLUP);  // lick detector for right port

  pinMode(FaceCamPin, INPUT_PULLUP);  // lick detector for left port
  pinMode(BodyCamPin, INPUT_PULLUP);  // lick detector for right port

  pinMode(SValvePinL, OUTPUT);    // solenoid valve for left port
  pinMode(SValvePinR, OUTPUT);    // solenoid valve for right port

  pinMode(SyncPin, OUTPUT);        // sync pulse

  pinMode(PhotodiodePin, INPUT);  // Photodiode

    // Read the initial state of the encoder pins
  uint8_t aState = digitalRead(WheelPin1);
  uint8_t bState = digitalRead(WheelPin2);
  lastEncoded = (aState << 1) | bState;
  
  // Attach interrupts to both encoder channels
  attachInterrupt(digitalPinToInterrupt(WheelPin1), updateEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(WheelPin2), updateEncoder, CHANGE);

  // interrupt for lick detector
  attachInterrupt(digitalPinToInterrupt(LickPinL), Lick_CounterL, FALLING);
  attachInterrupt(digitalPinToInterrupt(LickPinR), Lick_CounterR, FALLING);

  attachInterrupt(digitalPinToInterrupt(FaceCamPin), FaceCam_Receiver, RISING);
  attachInterrupt(digitalPinToInterrupt(BodyCamPin), BodyCam_Receiver, RISING);

  // interrupt for sync pulse - no longer used
  //attachInterrupt(digitalPinToInterrupt(SyncPin), SyncPulse_Receiver, RISING);

  Serial.begin(1000000);
  Serial.setTimeout(5);

  delay(500);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void loop() {
  //Check for change in position and send to Serial buffer

  currentMillis = millis();     //get the current "time" (actually the number of milliseconds since the program started)
  

  // update sync pulse
  if (!isSyncHigh && currentMillis - previousMillis >= waitTime) {
    // Turn pins HIGH
    lastSyncPulseTime = currentMillis;
    digitalWrite(SyncPin, HIGH);

    previousMillis = currentMillis;
    waitTime = syncPulseOnTime; // turn sync pulse on for 500ms
    isSyncHigh = true;
  }
  else if (isSyncHigh && currentMillis - previousMillis >= waitTime) {
    // Turn pins LOW
    digitalWrite(SyncPin, LOW);

    previousMillis = currentMillis;
    waitTime = random(syncPulseLowerBound, syncPulseUpperBound); // get next random interval
    isSyncHigh = false;
  }
  
  if (currentMillis - startMillis >= interval)  // check whether time to send new serial message
  {
    Serial.print(encoderCount);  // Wheel raw input
    Serial.print(",");
    Serial.print(lastFaceCamFrameTime);  // timestamp of last left lick was detected
    Serial.print(",");
    Serial.print(lastBodyCamFrameTime);  // last timestmap a right lick was detected
    Serial.print(",");
    Serial.print(lastSyncPulseTime);  // last time a HIGH async pulse value was detected
    Serial.print(",");
    //Serial.print(PhotodiodeVal); // most recent photodiode value (we probably want to use a separate arduino for this, to sample at 1kHz+)
    //Serial.print(",");
    Serial.print(currentMillis);  // Arduino timestamp for these values (mainly relevant for wheel)
    Serial.print("\n");

    startMillis = currentMillis;  // reset timer
  }
  
  GetSerialInput(); // get any serial input

  if (newData) { // action any new serial data
    newData = false;
    ActionSerial();
  }

  ActivatePVL(); // activate or deactivate valves
  ActivatePVR();

  PhotodiodeVal = analogRead(PhotodiodePin);  // read the photodiode input pin

  // get the latest rotary encoder value
  long count;
  count = encoderCount;

}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////// HANDLE SERIAL INPUT //////////////////////////////////////
void GetSerialInput() {  // part of code taken from http://forum.arduino.cc/index.php?topic=396450.0
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
    } else {                      // serial message finished
      receivedChars[ndx] = '\0';  // terminate the string
      ndx = 0;
      newData = true;
    }
  }
}


void ActionSerial() {  // Actions serial data by choosing appropriate stimulation
  
    char delimiters[] = ",";
  char *token;
  uint8_t idx = 0;
  #define MAX_VALS 5  // max required? freq, duration, contrast, carrier freq?
  char *serialVals[MAX_VALS];
  token = strtok(receivedChars, ",");


  while (token != NULL) {
    //Serial.println( token );
    if (idx < MAX_VALS)
      serialVals[idx++] = token;
    token = strtok(NULL, ",");
  }

  FirstChar = serialVals[0];

  if (FirstChar == "l")  // left reward
  {
    newRewardL = true;
    rewardTimeL = atoi(serialVals[1]);

  } 
  else if (FirstChar == "r")
  {
    newRewardR = true;
    rewardTimeR = atoi(serialVals[1]);
  }
}
   
/////////////////////////////////////
/// Pinch valve control functions ///
/////////////////////////////////////

void ActivatePVL() {
  if (newRewardL)  // if there's a new reward instruction
  {
    if (!ValveOpenL)              // if the valve isn't open, open it
    {
      StartTimeL = millis();           // get time that valve opened
      ValveOpenL = true;               // set bool flag for open valve
      digitalWrite(SValvePinL, HIGH);  // open the valve
    }
    newRewardL = false;  // we've opened the valve, so newReward is false.
  }
  if (ValveOpenL)  // if the valve is open, check to see if it's time to close it, irrespective of what happens to Serial.read()
  {
    if ((millis() - StartTimeL) >= rewardTimeL)  // if valve open timer finished
    {
      digitalWrite(SValvePinL, LOW);  // turn valve off
      ValveOpenL = false;             // set valve open bool to false
    }
  }
}

void ActivatePVR() {
  if (newRewardR)  // if there's a new reward instruction
  {
    if (!ValveOpenR)              // if the valve isn't open, open it
    {
      StartTimeR = millis();           // get time that valve opened
      ValveOpenR = true;               // set bool flag for open valve
      digitalWrite(SValvePinR, HIGH);  // open the valve
    }
    newRewardR = false;  // we've opened the valve, so newReward is false.
  }
  if (ValveOpenR)  // if the valve is open, check to see if it's time to close it, irrespective of what happens to Serial.read()
  {
    if ((millis() - StartTimeR) >= rewardTimeR)  // if valve open timer finished
    {
      digitalWrite(SValvePinR, LOW);  // turn valve off
      ValveOpenR = false;             // set valve open bool to false

    }
  }
}

/////////////////////////////////////
/// Wheel pos recording functions ///
/////////////////////////////////////

void updateEncoder() {
  //Serial.println("Do");
  // Read current state of encoder pins
  uint8_t aState = digitalRead(WheelPin1);
  uint8_t bState = digitalRead(WheelPin2);
  uint8_t encoded = (aState << 1) | bState;
  
  // Combine previous and current state to determine direction
  int8_t sum = (lastEncoded << 2) | encoded;
  
  // These patterns represent the valid transitions.
  // Depending on the direction of rotation, we update the count:
  if(sum == 0b0001 || sum == 0b0111 || sum == 0b1110 || sum == 0b1000)
    encoderCount++;
  else if(sum == 0b0010 || sum == 0b0100 || sum == 0b1101 || sum == 0b1011)
    encoderCount--;
  
  lastEncoded = encoded;  // Save the current state for the next update
}


/////////////////////////////////////
/// Lick time recording functions ///
/////////////////////////////////////
// Interrupt for when IR beam detection goes LOW (lick breaks beam)
void Lick_CounterL() {
  //lastLeftLickTime = millis(); // [not currently used]
  LickCountL = LickCountL + 1;
}
void Lick_CounterR() {
  //lastRightLickTime = millis();
  LickCountR = LickCountR + 1;
}

/////////////////////////////////////
/// Camera frame time recording functions ///
/////////////////////////////////////
void FaceCam_Receiver() {
  //Serial.println("trigger");
  lastFaceCamFrameTime = millis();
}

void BodyCam_Receiver() {
  //Serial.println("trigger");
  lastBodyCamFrameTime = millis();
}


///////////////////////////////////////
/// aSync pulse recording functions ///
///////////////////////////////////////
// Interrupt for when sync signal goes HIGH
//void SyncPulse_Receiver() {
//  lastSyncPulseTime = millis();
//}
