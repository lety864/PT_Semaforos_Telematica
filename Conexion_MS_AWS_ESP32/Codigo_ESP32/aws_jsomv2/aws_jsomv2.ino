//#include "secrets.h" //modulo1
#include "secretsv2.h" //modulo2 

#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"

#define AWS_IOT_PUBLISH_TOPIC   "esp32/pub"   //modulo1
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"

WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);

void connectAWS()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  //RED Poli
  //WiFi.begin("Wi-Fi IPN",NULL);
 
  Serial.println("Connecting to Wi-Fi");
 
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
 
  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);
 
  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.setServer(AWS_IOT_ENDPOINT, 8883);
 
  // Create a message handler
  client.setCallback(messageHandler);
 
  Serial.println("Connecting to AWS IOT");
 
  while (!client.connect(THINGNAME))
  {
    Serial.print(".");
    delay(100);
  }
 
  if (!client.connected())
  {
    Serial.println("AWS IoT Timeout!");
    return;
  }
 
  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
 
  Serial.println("AWS IoT Connected!");
}

void publishMessage()
{
    
  String jsonSemaforo;
  char jsonSemaforo_1[512];
  StaticJsonDocument<384> doc;
  while(!Serial.available()){} //espera hasta que este disponible algo en el puerto serial
  if(Serial.available())
    jsonSemaforo=Serial.readStringUntil('\n'); 
  //Publish to the topic
  Serial.println(jsonSemaforo);
  const char* payload = jsonSemaforo.c_str(); // Convert String to const char*
  
  if(client.publish(AWS_IOT_PUBLISH_TOPIC, payload )){ //verificar si publico o no
    Serial.println("Sent a message");
  }else{
    Serial.print("Error al enviar mensaje");    
  }

}

void messageHandler(char* topic, byte* payload, unsigned int length)
{
  Serial.print("incoming: ");
  Serial.println(topic);
 
  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);
  const char* message = doc["message"];
  Serial.println(message);
}

void setup() {
  Serial.begin(9600);
  Serial.setTimeout(500);
  connectAWS();
}

void loop() {
  // put your main code here, to run repeatedly:
  publishMessage();
  client.loop();
}
