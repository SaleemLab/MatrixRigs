// constants won't change:
const int ledPin =  3;// the number of the LED pin

// serial
const byte numChars = 15;
char receivedChars[numChars];   // an array to store the received data
bool newData = false;

float FlickerFreq;
float FlickerDur;
String FirstChar;

void setup() {
  // set the digital pin as output:
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
 // FlickerLED(1, 10000);

  Serial.begin (9600);
  Serial.setTimeout(5);

  delay(500);

}

void loop() {
  GetSerialInput();
}

void GetSerialInput() { // part of code taken from http://forum.arduino.cc/index.php?topic=396450.0
  static byte ndx = 0;
  char endMarker = '\r';
  char rc;

  char delimiters[] = ",";
  char *token;

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
      
      uint8_t  idx = 0;
       #define MAX_VALS    3
       char *pszValues[MAX_VALS];
      token = strtok( receivedChars, "," );
        
        while( token != NULL ) 
        {
        //Serial.println( token );
        if( idx < MAX_VALS )
            pszValues[idx++] = token;
        token = strtok( NULL, "," );
        }
        FirstChar = pszValues[0];
        if (FirstChar == "f")
        {
        
        FlickerFreq = atof(pszValues[1]);
        FlickerDur = atof(pszValues[2]);

        Serial.println(FirstChar);
        Serial.println(FlickerFreq);
        Serial.println(FlickerDur);
        FlickerLED(FlickerFreq, FlickerDur);
        }
        else
        {
          Serial.println("not f");
        }
        memset(0, receivedChars, sizeof(receivedChars));
      }

  }      
}






// Stimuli
void FlickerLED(float FlickerFreq, float Duration)
{
  const long interval = 1000/(FlickerFreq*2);           // interval at which to blink (milliseconds)
  unsigned long previousMillis = 0;        // will store last time LED was updated
  unsigned long TimerStart = millis();        // will store last time LED was updated
  volatile byte ledState = LOW;

  bool runFunction = HIGH;
  while (runFunction)
  {
      unsigned long currentMillis = millis();
      if (currentMillis - previousMillis >= interval) {
      // save the last time you blinked the LED
      previousMillis = currentMillis;

    // if the LED is off turn it on and vice-versa:
    ledState = !ledState;

    digitalWrite(ledPin, ledState);

    unsigned long TimerMillis = millis();
    if (TimerMillis-TimerStart >= Duration)
    {
      runFunction = LOW;
      digitalWrite(ledPin, LOW); // turn led off once done
      Serial.println("Stim done");
    }
  }
}

}
