#define SyncPin 6

unsigned long currentMillis;
unsigned long lastSyncPulseTime;
unsigned long startMillis;
float samplingFrequency = 120;
const long interval = 1000 / (samplingFrequency);

void setup() {
  // put your setup code here, to run once:
pinMode(SyncPin, INPUT);

attachInterrupt(digitalPinToInterrupt(SyncPin), SyncPulse_Receiver, CHANGE);

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
  Serial.print(currentMillis);
  //Serial.print(",");
  Serial.println("\n");

  startMillis = currentMillis;  
}}


void SyncPulse_Receiver(){
  lastSyncPulseTime = millis();
}