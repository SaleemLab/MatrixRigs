
#define SyncPin1 9
#define SyncPin2 10
#define SyncPin3 11
#define SyncPin4 12

long wait_time;

/////////////////////////////////
void setup() {
  // put your setup code here, to run once:
  pinMode(SyncPin1, OUTPUT);  // pin for sync pulse 1
  pinMode(SyncPin2, OUTPUT);  // pin for sync pulse 2
  pinMode(SyncPin3, OUTPUT);  // pin for sync pulse 3
  pinMode(SyncPin4, OUTPUT);  // pin for sync pulse 4

  randomSeed(analogRead(0));
  
  //Serial.begin(9600);
  // Serial.setTimeout(5);

  delay(500);
  
}



/////////////////////////////////
void loop() {
  // put your main code here, to run repeatedly:

  digitalWrite(SyncPin1, HIGH);
  digitalWrite(SyncPin2, HIGH);
  digitalWrite(SyncPin3, HIGH);
  digitalWrite(SyncPin4, HIGH);
  delay(500);
  
  digitalWrite(SyncPin1, LOW);
  digitalWrite(SyncPin2, LOW);
  digitalWrite(SyncPin3, LOW);
  digitalWrite(SyncPin4, LOW);
  wait_time = random(1000,3500);
  
  //Serial.println(wait_time);
  
  delay(wait_time);
  
}
