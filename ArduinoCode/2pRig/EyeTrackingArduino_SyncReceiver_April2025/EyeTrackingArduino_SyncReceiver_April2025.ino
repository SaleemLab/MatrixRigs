// Typically on COM10 [06.12.2025]
#define SyncPin 6
//#define FrameTrig 9   


unsigned long lastSyncPulseTime; // async pluse interrupt time 

unsigned long currentMillis; // rolling timer to check if its time to send the new data 
unsigned long startMillis; // sample timer that resets everytime new data is sent
float samplingFrequency = 120;
const long interval = 1000 / (samplingFrequency);

// variables for frame trigger and line trigger counters 
//unsigned long last2pFrameTime;   // updates each time 2p frame goes LOW

void setup() {
  // put your setup code here, to run once:
pinMode(SyncPin, INPUT);
//pinMode(FrameTrig, INPUT);              // frame trigger 

// interrupts async pulse 
attachInterrupt(digitalPinToInterrupt(SyncPin), SyncPulse_Receiver, CHANGE);
// interrupt for frame and line scanner trigger 
//attachInterrupt(digitalPinToInterrupt(FrameTrig), Frame_Counter, FALLING);

Serial.begin(1000000);
startMillis = millis();
}

void loop() {
  // put your main code here, to run repeatedly:
currentMillis = millis();
if (currentMillis - startMillis >= interval)
{
  Serial.print(lastSyncPulseTime);
  Serial.print("\t");
  //Serial.print(last2pFrameTime);
  //Serial.print("\t");
  Serial.print(currentMillis);
  //Serial.print(",");
  Serial.println("\n");

  startMillis = currentMillis;  
}}


void SyncPulse_Receiver(){
  lastSyncPulseTime = millis();
}

/////////////////////////////////////////
/// 2p frame time recording functions ///
/////////////////////////////////////////

//void Frame_Counter() { // Interrupt for when frame trigger goes down and line trigger goes LOW
//  last2pFrameTime = millis();
//
