String carsData;
void setup() {
 Serial.begin(9600);
 Serial.setTimeout(100);

}

void loop() {
  while(!Serial.available());
  carsData= Serial.readString();
  

}
/*
HardwareSerial Sender(1);
HardwareSerial Receiver(2);
String carsData; 

void setup() {
 Serial.begin(9600);
 Serial.setTimeout(100);
 Sender.begin(9600,SERIAL_8N1,17.16);
 Receiver.begin(9600,SERIAL_8N1,14,27);

}

void loop() {
  while(!Serial.available());
  carsData= Serial.readString();
  Sender.print(carsData);
  while(!Receiver.available()){
    String s = Receiver.readString();
    Serial.println(s);
  }
  //Serial.println(carsData);

}*/
