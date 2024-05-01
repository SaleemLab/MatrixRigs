//#include <Event.h>
//#include <Timer.h>

#define photodiodePin 0        // PhotoDiode pin - analogue
#define syncPin 10             // digital pin of sync

// Serial communication variables
float samplingFrequency = 1000; // frequency to send new values to computer
const long interval = 1000000 / (samplingFrequency);  // sampling interval to send new values
unsigned long startMicros;  // sample timer that resets each time new data is sent
unsigned long currentMicros; // rolling timer to check if it's time to send new data

/*
int photodiodePin = 0;    
int syncPin = 0;
*/

int photodiodeVal;
int syncVal;

void setup()

{
  pinMode(photodiodePin, INPUT);   // PhotoDiode
  pinMode(syncPin, INPUT);   // digital pin of sync
  
  Serial.begin(250000);          //  setup serial
  delay(500);
}



void loop()

{

  photodiodeVal = analogRead(photodiodePin);    // read the input pin
  syncVal = digitalRead(syncPin);
  currentMicros = micros();                   //get the current "time" (actually the number of milliseconds since the program started)
  if (currentMicros - startMicros >= interval)  //test whether the period has elapsed
  {
  Serial.print(photodiodeVal);
  Serial.print(",");
  Serial.println(syncVal);
  //delay(0);
      startMicros = currentMicros;  // reset timer
  }
  
}
