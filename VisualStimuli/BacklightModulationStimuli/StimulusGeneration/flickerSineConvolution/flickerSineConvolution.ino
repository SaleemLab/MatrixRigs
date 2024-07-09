// constants won't change:
const int ledPin = 3;  // the number of the LED pin
const unsigned long flickerFreq = 20;
const long interval = 1000 / (flickerFreq * 2);  // interval at which to blink (milliseconds)

const unsigned long sineFreq = 0.5;
uint32_t period = 10000L;  // the L forces Long ints 32 bit

volatile byte ledState = LOW;


// Variables will change:
unsigned long previousMillis = 0;  // will store last time LED was updated

void setup() {
  pinMode(3, OUTPUT);
  //Serial.begin(9600);
}

void loop() {

  // generate sine wave between 0 and 1
  int time = millis() % period;        // returns a value between 0 and period;
  float angle = (PI * time) / period;  // mapping to degrees
  float y = sin(angle);

  // get low and high luminance levels for sine wave-based contrast 
  float lowValue = 127 - y * 127;
  float highValue = 127 + y * 127;

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    ledState = !ledState;
    //Serial.println(ledState);

    if (ledState == 0) {
      analogWrite(ledPin, lowValue);
      //Serial.println(lowValue);

    } else {
      analogWrite(ledPin, highValue);
      //Serial.println(highValue);
    }
  }
}