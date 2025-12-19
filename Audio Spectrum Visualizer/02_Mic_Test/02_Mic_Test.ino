#define MIC_PIN 34

void setup() {
  Serial.begin(115200);
}

void loop() {
  int val = analogRead(MIC_PIN);
  Serial.println(val);
  delay(5);
}
