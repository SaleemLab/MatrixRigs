// constants won't change:

const int pin0 = 8;
const int pin50 = 9;//
const int ledPin =  10;// the number of the LED pin
const unsigned long flickerFreq = 5;
const long interval = 1000/(flickerFreq*2);           // interval at which to blink (milliseconds)

// Variables will change:
int ledState = LOW;             // ledState used to set the LED

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;        // will store last time LED was updated


void setup() {
  // set the digital pin as output:
  pinMode(ledPin, OUTPUT);
  pinMode(pin0, OUTPUT);
  pinMode(pin50, OUTPUT);

  analogWrite(pin0, 0);
  analogWrite(pin50, 50);
}

void loop() {

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;

    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }

    // set the LED with the ledState of the variable:
    digitalWrite(ledPin, ledState);
  }
}
