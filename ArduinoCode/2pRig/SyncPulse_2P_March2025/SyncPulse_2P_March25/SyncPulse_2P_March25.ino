
#define encoderPinOUT 8

long wait_time;

/////////////////////////////////
void setup() {
  // put your setup code here, to run once:
  pinMode(encoderPinOUT, OUTPUT);  // pin for sync pulse

  randomSeed(analogRead(0));
  
  Serial.begin(1000000);
  Serial.setTimeout(5);

  delay(500);
  
}



/////////////////////////////////
void loop() {
  // put your main code here, to run repeatedly:

  digitalWrite(encoderPinOUT, HIGH);
  delay(500);
  
  digitalWrite(encoderPinOUT, LOW);
  wait_time = random(1000,3500);
  
  Serial.print(wait_time);
  Serial.print("\n");
  
  delay(wait_time);
  
}
