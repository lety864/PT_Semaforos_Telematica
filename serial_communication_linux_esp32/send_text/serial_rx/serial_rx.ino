String x;
void setup() {
 Serial.begin(9600);
 Serial.setTimeout(1000);

}

void loop() {
  while(!Serial.available());
  x= Serial.readString();
  Serial.println(x);

}
