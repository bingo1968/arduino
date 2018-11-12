int pinInterrupt = 2;

void onChange(){
  if (digitalRead(pinInterrupt)== LOW) Serial.println("Key Down");
  else Serial.println("Key Up");
  }
  
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(pinInterrupt,INPUT);
  attachInterrupt(digitalPinToInterrupt(pinInterrupt), onChange, CHANGE);
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(10);
}
