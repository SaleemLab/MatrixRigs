// define the pins that hardware are connected to
#define LickPinL 2        // digital pin of lick detector
#define LickPinR 3        // digital pin of lick detector
#define encoder0PinA 18   // sensor A of rotary encoder
#define encoder0PinB 19   // sensor B of rotary encoder
#define SValvePinL 11     // digital pin controlling the left solenoid valve
#define SValvePinR 12     // digital pin controlling the right solenoid valve
#define SyncPin 10        // sync pulse pin
#define PhotodiodePin 17  // photoiode analog input

// Serial communication variables
float samplingFrequency = 100; // frequency to send new values to computer
const long interval = 1000 / (samplingFrequency);  // sampling interval to send new values
unsigned long startMillis;  // sample timer that resets each time new data is sent
unsigned long currentMillis; // rolling timer to check if it's time to send new data

// variables for rotary encoder
volatile unsigned int encoder0Pos = 0;  // variable for counting ticks of rotary encoder
bool A_set;
bool B_set;

// event time variables (async pulse, licks)
unsigned long lastSyncPulseTime;    // updates each time async pulse goes HIGH
//unsigned long lastLeftLickTime;  // [not currently used] updates each time left lick detector goes LOW
//unsigned long lastRightLickTime;   // updates each time right lick detector goes LOW
volatile unsigned int LickCountL = 0;
volatile unsigned int LickCountR = 0;

// variables for pinch valve of reward system
const byte numChars = 10;
char receivedChars[numChars];  // an array to store the received data

// left reward vars
char rewardTimeL[6];  // an array to store the received data
bool newRewardL = false; // bool flag for if a new reward is requested on this valve
int TimeONL = 0; // requested valve open time received from computer
bool ValveOpenL = false; // bool flag for whether valve is currently open
uint32_t StartTimeL = 0;  // start timer for when valve first opens
bool TimerFinishedL = false; // bool flag for whether the valve open time has ended

// right reward vars
char rewardTimeR[6];  // an array to store the received data
boolean newRewardR = false;
int TimeONR = 0; 
bool ValveOpenR = false;
uint32_t StartTimeR = 0;  
bool TimerFinishedR = false;

// variable for photodiode value
int PhotodiodeVal;

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

void setup() {

  pinMode(encoder0PinA, INPUT);  // rotary encoder sensor A
  pinMode(encoder0PinB, INPUT);  // rotary encoder sensor B

  pinMode(LickPinL, INPUT_PULLUP);  // lick detector for left port
  pinMode(LickPinR, INPUT_PULLUP);  // lick detector for right port

  pinMode(SValvePinL, OUTPUT);    // solenoid valve for left port
  pinMode(SValvePinR, OUTPUT);    // solenoid valve for right port
  pinMode(SyncPin, INPUT);        // sync pulse
  pinMode(PhotodiodePin, INPUT);  // PhotoDiode

  // interrupts for rotary encoder
  attachInterrupt(digitalPinToInterrupt(encoder0PinA), doEncoderA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoder0PinB), doEncoderB, CHANGE);

  // interrupt for lick detector
  attachInterrupt(digitalPinToInterrupt(LickPinL), Lick_CounterL, FALLING);
  attachInterrupt(digitalPinToInterrupt(LickPinR), Lick_CounterR, FALLING);

  // interrupt for sync pulse
  attachInterrupt(digitalPinToInterrupt(SyncPin), SyncPulse_Receiver, RISING);

  Serial.begin(1000000);
  Serial.setTimeout(5);

  delay(500);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void loop() {
  //Check for change in position and send to Serial buffer

  currentMillis = millis();                   //get the current "time" (actually the number of milliseconds since the program started)
  if (currentMillis - startMillis >= interval)  //test whether the period has elapsed
  {
    Serial.print(encoder0Pos);  // Wheel raw input
    Serial.print(",");
    Serial.print(LickCountL);  // timestamp of last left lick was detected
    Serial.print(",");
    Serial.print(LickCountR);  // last timestmap a right lick was detected
    Serial.print(",");
    Serial.print(lastSyncPulseTime);  // last time a HIGH async pulse value was detected
    Serial.print(",");
    Serial.print(PhotodiodeVal); // most recent photodiode value (we probably want to use a separate arduino for this, to sample at 1kHz+)
    Serial.print(",");
    Serial.print(currentMillis);  // Arduino timestamp for these values (mainly relevant for wheel)
    Serial.print("\n");

    startMillis = currentMillis;  // reset timer
  }


  GetSerialInput();
  ActivatePVL();
  ActivatePVR();
  PhotodiodeVal = analogRead(PhotodiodePin);  // read the input pin
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// TO do: use memset to 'empty' the received chars
// Read inputs from Matlab. It is an integer specifing the amount of time in ms the pinch valve stays open
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
    } else {
      receivedChars[ndx] = '\0';  // terminate the string
      ndx = 0;
    }

    if (receivedChars[0] == 'r') { // if first char is 'r', we want to open the right valve
      for (int i = 0; i < 5; i = i + 1) {
        rewardTimeR[i] = receivedChars[i + 1];
      }
      newRewardR = true;
    }

    if (receivedChars[0] == 'l') { // if first char is 'l', we want to open the left valve
      for (int i = 0; i < 5; i = i + 1) {
        rewardTimeL[i] = receivedChars[i + 1];
      }
      newRewardL = true;
    }
  }
}


/////////////////////////////////////
/// Pinch valve control functions ///
/////////////////////////////////////

void ActivatePVL() {
  if (newRewardL)  // if there's a new reward instruction
  {
    TimeONL = atoi(rewardTimeL);  // get valve duration by convert arraying of chars to integers
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
    if ((millis() - StartTimeL) >= (uint32_t)TimeONL)  // if valve open timer finished
    {
      digitalWrite(SValvePinL, LOW);  // turn valve off
      ValveOpenL = false;             // set valve open bool to false
    }
  }
}

void ActivatePVR() {
  if (newRewardR)  // if there's a new reward instruction
  {
    TimeONR = atoi(rewardTimeR);  // get valve duration by convert arraying of chars to integers
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
    if ((millis() - StartTimeR) >= (uint32_t)TimeONR)  // if valve open timer finished
    {
      digitalWrite(SValvePinR, LOW);  // turn valve off
      ValveOpenR = false;             // set valve open bool to false
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
// Interrupt for when IR beam detection goes LOW (lick breaks beam)
void Lick_CounterL() {
  lastLeftLickTime = millis();
  LickCountL = LickCountL + 1;
}
void Lick_CounterR() {
  lastRightLickTime = millis();
  LickCountR = LickCountR + 1;
}

///////////////////////////////////////
/// aSync pulse recording functions ///
///////////////////////////////////////
// Interrupt for when sync signal goes HIGH
void SyncPulse_Receiver() {
  lastSyncPulseTime = millis();
}
