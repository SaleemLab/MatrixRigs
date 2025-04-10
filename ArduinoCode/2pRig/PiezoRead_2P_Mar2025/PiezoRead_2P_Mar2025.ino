// Serial output has this format - each line has ('\t separated/): 
//     1. Piezo value
//     2. Async Pulse
//     3. Arduino time stamp
//


#define PiezoPin 2 // analog pin of lick detector
#define SyncPin 10              // sync pulse pin

// serial frequency variables
float samplingFrequency = 1000; // frequency to send new values to computer (Hz)
const long interval = 1000 / (samplingFrequency);  // sampling interval to send new values (ms)
unsigned long startMillis;  // sample timer that resets each time new data is sent
unsigned long currentMillis; // rolling timer to check if it's time to send new data

// piezo value
unsigned long PiezoVal = 0;
unsigned long lastSyncPulseTime;    // updates each time async pulse goes HIGH



///////////////////////////////////////////////////////////////////////////////////


void setup() {

  pinMode(PiezoPin, INPUT);         // piezo input
  pinMode(SyncPin, INPUT);              // sync pulse

  // interrupt for sync pulse
  attachInterrupt(digitalPinToInterrupt(SyncPin), SyncPulse_Receiver, CHANGE);
  

  Serial.begin(1000000);
  Serial.setTimeout(5);

  delay(500);
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void loop() {

	PiezoVal = analogRead(PiezoPin); //read the piezo input  pin
  
  currentMillis = millis();  //get the current "time" (the number of milliseconds since the program started)
  if (currentMillis - startMillis >= interval)  //test whether the period has elapsed
  {
    
    //PhotodiodeVal = analogRead(PhotodiodePin); // read the input pin
    Serial.print(PiezoVal);// Wheel raw input
    Serial.print("\t");
    Serial.print(lastSyncPulseTime);// Sync pulse high/low status
    Serial.print("\t");
    Serial.print(currentMillis);// Arduino timestamp
    Serial.print("\n");
    startMillis = currentMillis;  // update timer
  }
    

}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////
/// aSync pulse recording functions ///
///////////////////////////////////////
// Interrupt for when sync signal goes HIGH
void SyncPulse_Receiver() {
  lastSyncPulseTime = millis();
}
