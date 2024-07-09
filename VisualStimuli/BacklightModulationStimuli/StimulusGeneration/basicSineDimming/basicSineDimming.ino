uint32_t period = 5000L;   // the L forces Long ints 32 bit

void setup() 
{
  pinMode(3, OUTPUT);

}

void loop()
{
  int time = millis() % period;              // returns a value between 0 and period;
  float angle = (PI * time) / period;        // mapping to degrees
  int y = 100 * sin(angle);

  analogWrite(3, y);           // PWM outout
  Serial.println(y);

 
  // other code here
}