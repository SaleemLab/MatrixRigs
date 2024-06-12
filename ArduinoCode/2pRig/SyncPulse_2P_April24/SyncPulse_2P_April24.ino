
#define encoderPinOUT 5

long wait_time;

/////////////////////////////////
void setup() {
  // put your setup code here, to run once:
  pinMode(encoderPinOUT, OUTPUT);  // pin for sync pulse

  randomSeed(analogRead(0));
  
  // Serial.begin(9600);
  // Serial.setTimeout(5);

  delay(500);
  
}



/////////////////////////////////
void loop() {
  // put your main code here, to run repeatedly:

  digitalWrite(encoderPinOUT, HIGH);
  delay(500);
  
  digitalWrite(encoderPinOUT, LOW);
  wait_time = random(1000,3500);
  
  //Serial.println(wait_time);
  
  delay(wait_time);
  
}
