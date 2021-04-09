

void setup() {
   Serial.begin(9600);
   pinMode(D1, INPUT);
}

void loop() {
long state = digitalRead(D1);
    if(state == HIGH) {
      Serial.println("Motion detected!");
      delay(1000);
    }
    else {
      Serial.println("Motion absent!");
      delay(1000);
      }
    
}
