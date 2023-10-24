#include "secrets.h"
#include <WiFiClientSecure.h>
#include <MQTTClient.h> //MQTT Library Source: https://github.com/256dpi/arduino-mqtt
#include <ArduinoJson.h> //ArduinoJson Library Source: https://github.com/bblanchon/ArduinoJson
#include "WiFi.h"

// MQTT topics for the device
#define AWS_IOT_PUBLISH_TOPIC   "esp32/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"


WiFiClientSecure wifi_client = WiFiClientSecure();
MQTTClient mqtt_client = MQTTClient(381); //256 indicates the maximum size for packets being published and received.

uint32_t t1;
const int maxStringLen=280;  //Variable para la maxima cantidad de bytes a leer
void connectAWS()
{
  //Begin WiFi in station mode
  WiFi.mode(WIFI_STA); 
  //RED con contraseña
  //WiFi.begin(ssid, password);
  //RED Poli
    WiFi.begin("Wi-Fi IPN",NULL);
  
  

  Serial.println("Connecting to Wi-Fi");

  //Wait for WiFi connection
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
 // Serial.println();

  // Configure wifi_client with the correct certificates and keys
  wifi_client.setCACert(AWS_CERT_CA);
  wifi_client.setCertificate(AWS_CERT_CRT);
  wifi_client.setPrivateKey(AWS_CERT_PRIVATE);

  //Connect to AWS IOT Broker. 8883 is the port used for MQTT
  mqtt_client.begin(AWS_IOT_ENDPOINT, 8883, wifi_client);

  //Set action to be taken on incoming messages
  mqtt_client.onMessage(incomingMessageHandler);

  Serial.print("Connecting to AWS IOT");

  //Wait for connection to AWS IoT
  while (!mqtt_client.connect(THINGNAME)) {
   // Serial.print(".");
    delay(100);
  }
  Serial.println();

  if(!mqtt_client.connected()){
    Serial.println("AWS IoT Timeout!");
    return;
  }

  //Subscribe to a topic
  mqtt_client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);

  Serial.println("AWS IoT Connected!");
}

void publishMessage()
{
  //Create a JSON document of size 200 bytes, and populate it
  //See https://arduinojson.org/
  //StaticJsonDocument<200> doc;
  //doc["elapsed_time"] = millis() - t1;
  //doc["value"] = random(1000);
  //char jsonBuffer[512];
  //serializeJson(doc, jsonBuffer); // print to mqtt_client
  
  
  String jsonSemaforo;
  char jsonSemaforo_1[512];
  StaticJsonDocument<384> doc;
  
  while(!Serial.available()){} //espera hasta que este disponible algo en el puerto serial
  
  if(Serial.available())
    jsonSemaforo=Serial.readStringUntil('\n');
  
  
  //Publish to the topic
  Serial.println(jsonSemaforo);
  
  if(mqtt_client.publish(AWS_IOT_PUBLISH_TOPIC, jsonSemaforo )){ //verificar si publico o no
    Serial.println("Sent a message");
  }else{
    Serial.print("Error al enviar mensaje");
    
  }
  
  
  
}

void incomingMessageHandler(String &topic, String &payload) {
  Serial.println("Message received!");
  Serial.println("Topic: " + topic);
  Serial.println("Payload: " + payload);
}

void setup() {
  Serial.begin(9600);
  Serial.setTimeout(500);
  connectAWS();
}

void loop() {
  publishMessage();
  mqtt_client.loop();
  //delay(4000);
}
