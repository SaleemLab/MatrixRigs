// To do:
// 1) Set vars to 0 once stimulus has run to prevent accidental leak to subsequent stimuli
// 2) check valid vars (mainly nonzero) before running stimuli. Otherwise return a serial error message

// constants won't change:
const int ledPin =  3;// the number of the LED pin

// serial
const byte numChars = 30;
char receivedChars[numChars];   // an array to store the received data
bool newData = false;

// stimulus selection char
String FirstChar;

// generic stimulus duration var
float stimulusDuration;

// constant brightness value
float pwmVal;

// vars for flicker and sinusoidal dimming
float pwmSin[48];
float frequency;

// vars for temporal contrast switching
float contrastSwitchTime;
float ledUpdateTime;

// vars for flicker-sine convolution
float envelopeFrequency;

// vars for flicker sweep
float startFrequency;
float frequencyMultiplier;
float frequencyUpdateTime;



void setup() {
  // set the digital pin as output:

  TCB1_CTRLA = 0b00000011; // change pin3 pwm freq to 32.25kh

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  Serial.begin(115200);
  Serial.setTimeout(5);

  // create LUT for 48 step sine wave
  for (int i = 0; i <= 48; i++) {
    pwmSin[i] = (127.5 * (1 - (sin(PI + (2 * PI * i / 48))))) ;
    //pwmSin[i] = 127.5 * sin(2 * PI * i / 48)));

    Serial.println(pwmSin[i]);
  }

  SetPWM(127.5);

  delay(500);

}

void loop() {
  GetSerialInput();
  if (newData)
  {
    newData = false;
    ActionSerial();
  }
}

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
    else { // serial message finished
      receivedChars[ndx] = '\0'; // terminate the string
      ndx = 0;
      newData = true;
      }
  }      
}

void ActionSerial() { // Actions serial data by choosing appropriate stimulation
  Serial.print("rc: ");
  Serial.println(receivedChars);
  char delimiters[] = ",";
  char *token;
  uint8_t  idx = 0;
  #define MAX_VALS    5 // max required? freq, duration, contrast, carrier freq?
  char *serialVals[MAX_VALS];
  token = strtok( receivedChars, "," );
  
  
  while( token != NULL ) 
  {
    //Serial.println( token );
    if( idx < MAX_VALS )
    serialVals[idx++] = token;
    token = strtok( NULL, "," );
  }

        FirstChar = serialVals[0];

        if (FirstChar == "f") // flicker
        {
          stimulusDuration = atof(serialVals[1]);
          frequency = atof(serialVals[2]);
          Serial.println("Stim: Flicker");
          Serial.print("Stim duration: ");
          Serial.println(stimulusDuration);
          Serial.print("Frequency: ");
          Serial.println(frequency);

          FlickerLED(frequency, stimulusDuration);
        }
        else if (FirstChar == "s") // sinusoidal dimming
        {
          stimulusDuration = atof(serialVals[1]);
          frequency = atof(serialVals[2]);
          Serial.println("Stim: Sinusoidal dimming");
          Serial.print("Stim duration: ");
          Serial.println(stimulusDuration);
          Serial.print("Frequency: ");
          Serial.println(frequency);

          SineLED(frequency, stimulusDuration);
        }

        else if (FirstChar == "cs") // contrast switching
        {
          stimulusDuration = atof(serialVals[1]);
          contrastSwitchTime = atof(serialVals[2]);
          ledUpdateTime = atof(serialVals[3]);
          Serial.println("Stim: ContrastSwitching");
          Serial.print("Stim duration: ");
          Serial.println(stimulusDuration);
          Serial.print("Contrast Switching Time: ");
          Serial.println(contrastSwitchTime);
          Serial.print("LED Update Time: ");
          Serial.println(ledUpdateTime);

          
          contrastSwitching(contrastSwitchTime, stimulusDuration, ledUpdateTime);
        }
        else if (FirstChar == "fsc") // flicker-sine convolution
        {
          stimulusDuration = atof(serialVals[1]);
          frequency = atof(serialVals[2]); // flicker frequency
          envelopeFrequency = atof(serialVals[3]);
          Serial.println("Stim: Flicker-sine convolution");
          Serial.print("Stim duration: ");
          Serial.println(stimulusDuration);
          Serial.print("Flicker Frequency: ");
          
          Serial.println(frequency);
          Serial.print("Envelope Frequency: ");
          Serial.println(envelopeFrequency);
          
          SineContrastConv(stimulusDuration, frequency, envelopeFrequency);
        }
        else if (FirstChar == "fs") // Flicker-sweep stimulus
        {
          stimulusDuration = atof(serialVals[1]);
          startFrequency = atof(serialVals[2]); // flicker frequency
          frequencyMultiplier = atof(serialVals[3]);
          frequencyUpdateTime = atof(serialVals[4]);
          
          //startFrequency = 1;
          //frequencyMultiplier = 1.05; // update frequency
          //frequencyUpdateTime = 200; // msstimulusDuration = 20000;
          //stimulusDuration = 20000; // ms

          //FlickerSweepLED(5, stimulusDuration);
          FlickerSweepLED(startFrequency, frequencyMultiplier, frequencyUpdateTime, stimulusDuration);

        }
        else if (FirstChar == "sb") // Set Brightness
        {
          pwmVal = atof(serialVals[1]);
          
          SetPWM(pwmVal);

        }
        else // not valid stimulus code
        {
          Serial.print(FirstChar);
          Serial.println(" is an invalid stimulus code");
        }
        //memset('\0', receivedChars, sizeof(receivedChars));
        memset(receivedChars, '\0', sizeof(receivedChars));
        //Serial.print("rc: ");
        //Serial.println(receivedChars);
        

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
    }

    unsigned long TimerMillis = millis();
    if (TimerMillis-TimerStart >= Duration)
    {
      runFunction = LOW;
      //digitalWrite(ledPin, LOW); // turn led off once done
      SetPWM(0);
      //Serial.println("99");
    }
  }
}

void SineLED(float SineFreq, float Duration)
{
  const long interval = 1000000/(SineFreq*48);           // interval at which to blink (milliseconds)
  unsigned long previousMillis = 0;        // will store last time LED was updated

  unsigned long TimerStart = millis();        // will store last time LED was updated
  unsigned long previousMicros = 0;        // will store last time LED was updated
  volatile byte ledState = LOW;

  bool runFunction = HIGH;
  int i=0;

  while (runFunction)
  {
    unsigned long currentMicros = micros();
    if (currentMicros - previousMicros >= interval) { // check if time to update PWM duty cycle
      previousMicros = currentMicros;
      analogWrite(ledPin, pwmSin[i]);
      //Serial.println(pwmSin[i]);
      i++;
      if (i>47) {i=0;}
       }

    unsigned long TimerMillis = millis(); // Check for end of stim duration
    if (TimerMillis-TimerStart >= Duration)
    {
      runFunction = LOW;
      //digitalWrite(ledPin, LOW); // turn led off once done
      SetPWM(127.5); // turn LED to mean luminance once done
      //Serial.println("99");
    }
  }
}

void contrastSwitching(float contrastSwitchTime, float Duration, float updateTime)
{
  //const unsigned long contrastSwitchTime = 3000; // in ms
  //const unsigned long updateTime = 100; // in ms

  int randNumber;      // the variable which is supposed to hold the random number
  volatile byte randNumberByte;
  volatile byte contrast = LOW; // this will switch to start at HIGH

  unsigned long previousMillis = 0;        // will store last time LED was updated
  unsigned long switchPreviousMillis = 0;        // will store last time LED was updated

  unsigned long TimerStart = millis();        // will store last time LED was updated
  volatile byte ledState = LOW;

  unsigned long t = 0;

  bool runFunction = HIGH;
  while (runFunction)
  {
    unsigned long switchCurrentMillis = millis();
    if (switchCurrentMillis - switchPreviousMillis >= contrastSwitchTime) {
    switchPreviousMillis = switchCurrentMillis;

        // if the LED is off turn it on and vice-versa:
    contrast=!contrast;
    }
  
  // check if time to update LED and update
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= updateTime) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;

  if (contrast==HIGH) 
  {
    randNumber = random(0, 255);
    analogWrite(ledPin, randNumber);
    randNumberByte = randNumber;
    Serial.println(byte(randNumber));

  } 
  else
  {
    randNumber = random(64, 191);
    analogWrite(ledPin, randNumber);  
    randNumberByte = randNumber;
    Serial.println(byte(randNumber));
  }

  }
  unsigned long TimerMillis = millis();
  if (TimerMillis-TimerStart >= Duration)
    {
      runFunction = LOW;
      digitalWrite(ledPin, LOW); // turn led off once done
      Serial.println("-1");
    }
}
}

void FlickerSweepLED(float startFrequency, float frequencyMultiplier, float frequencyUpdateTime, float Duration)
{
  //startFrequency = 1;
  float currentFrequency = startFrequency;
  //float frequencyMultiplier = 1.05; // update frequency
  //frequencyUpdateTime = 200; // ms

  long interval = 1000/(currentFrequency*2);           // interval at which to blink (milliseconds)
  unsigned long previousMillis = 0;        // will store last time LED was updated
  unsigned long TimerStart = millis();        // will store last time LED was updated
  unsigned long previousFreqUpdateMillis = millis();
  volatile byte ledState = LOW;

  bool runFunction = HIGH;
  bool checkForReverse = HIGH;
  while (runFunction)
  {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    // if the LED is off turn it on and vice-versa:
    ledState = !ledState;
    digitalWrite(ledPin, ledState);
    }

    unsigned long FreqUpdateMillis = millis();
        if (FreqUpdateMillis - previousFreqUpdateMillis >= frequencyUpdateTime) {
          // save last frequency update time
          previousFreqUpdateMillis = FreqUpdateMillis;
          currentFrequency = currentFrequency*frequencyMultiplier;
          interval = 1000/(currentFrequency*2);
        }

        unsigned long TimerMillis = millis();

        if (checkForReverse)
        {
          if (TimerMillis-TimerStart >= Duration/2)
          {
          checkForReverse = LOW;
          frequencyMultiplier = 1/frequencyMultiplier;
          }
        }
        
        if (TimerMillis-TimerStart >= Duration)
        {
          runFunction = LOW;
          digitalWrite(ledPin, LOW); // turn led off once done
          Serial.println("-1");
        }
    }
}




void SineContrastConv(float Duration, float SineFreq, float EnvelopeFreq)
{
  const long interval = 1000000/(SineFreq*48);           // interval at which to blink (milliseconds)
  const long intervalEnv = 1000000/(EnvelopeFreq*48);   // interval to change envelope contrast val
  unsigned long previousMillis = 0;        // will store last time LED was updated

  unsigned long TimerStart = millis();        // will store last time LED was updated
  unsigned long previousMicros = 0;        // will store last time LED was updated
  unsigned long previousMicrosEnv = 0;        // will store last time contrast was updated
  volatile byte ledState = LOW;

  bool runFunction = HIGH;
  int i=0;
  int j=0;
  float contrastMult = 1;

  while (runFunction)
  {

    unsigned long currentMicros = micros();

    if (currentMicros - previousMicrosEnv >= intervalEnv) { // check if time to update contrast envelope
      previousMicrosEnv = currentMicros;
      contrastMult = pwmSin[j]/255;
      //Serial.println(contrastMult);
      //Serial.println(pwmSin[i]);
      j++;
      if (j>47) {j=0;}
       }

    if (currentMicros - previousMicros >= interval) { // check if time to update PWM duty cycle
      previousMicros = currentMicros;
      analogWrite(ledPin, 127.5+((pwmSin[i]-127.5)*contrastMult));
      //Serial.println(pwmSin[i]);
      i++;
      if (i>47) {i=0;}
       }

       

    unsigned long TimerMillis = millis(); // Check for end of stim duration
    if (TimerMillis-TimerStart >= Duration)
    {
      runFunction = LOW;
      //digitalWrite(ledPin, LOW); // turn led off once done
      SetPWM(127.5); // turn LED to mean luminance once done
      //Serial.println("99");
    }
  }
}


void SetPWM(float pwmVal)
{
  analogWrite(ledPin, pwmVal);
  Serial.println("-1");
}



