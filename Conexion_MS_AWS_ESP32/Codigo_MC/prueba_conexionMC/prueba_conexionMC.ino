#include "secrets.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"

//SEMAFOROS
#define D13 13
int i, s1[2] = {15, 4}, s2[2] = {14, 27};


WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);

#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/MC"

// Variables para almacenar valores del JSON
int spValues[8] = {0}; // Suponiendo que los valores de SP son enteros
int svValues[4] = {0}; // Suponiendo que los valores de SV son enteros

static bool started = false;//iniciar ciclo
TaskHandle_t Task1;

// Función de la operación en paralelo
void parallelTask(void *pvParameters)  //modificar funcion para que
{
  while (!started) {
    Serial.println("Esperando iniciar semaforo");
    delay(1000);
  }
  while (started) {
    digitalWrite(s1[0], LOW);
    delay(3000);
    digitalWrite(s1[0], HIGH);


    digitalWrite(s1[1], LOW);
    delay(3000);
    digitalWrite(s1[1], HIGH);


    digitalWrite(s2[0], LOW);
    delay(3000);
    digitalWrite(s2[0], HIGH);


    digitalWrite(s2[1], LOW);
    delay(3000);
    digitalWrite(s2[1], HIGH);


    digitalWrite(D13, LOW);
    delay(3000);
    digitalWrite(D13, HIGH);


    digitalWrite(s1[0], LOW);
    digitalWrite(s1[1], LOW);
    digitalWrite(s2[0], LOW);
    digitalWrite(s2[1], LOW);
    digitalWrite(D13, LOW);
    delay(3000);
    digitalWrite(s1[0], HIGH);
    digitalWrite(s1[1], HIGH);
    digitalWrite(s2[0], HIGH);
    digitalWrite(s2[1], HIGH);
    digitalWrite(D13, HIGH);
  }
}


void connectAWS()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.println("Conectando a Wi-Fi");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  // Configure WiFiClientSecure para usar las credenciales del dispositivo AWS IoT
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  // Conectar al broker MQTT en el endpoint de AWS IoT
  client.setServer(AWS_IOT_ENDPOINT, 8883);

  // Create a message handler
  client.setCallback(messageHandler);

  Serial.println("Conectando a AWS IoT");

  while (!client.connect(THINGNAME))
  {
    Serial.print(".");
    delay(100);
  }

  if (!client.connected())
  {
    Serial.println("¡AWS IoT Timeout!");
    return;
  }

  // Suscribirse al tema
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);

  Serial.println("¡AWS IoT Conectado!");
}

void messageHandler(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Mensaje recibido en el tema: ");
  Serial.println(topic);

  // Procesar el mensaje JSON recibido
  DynamicJsonDocument doc(512);
  DeserializationError error = deserializeJson(doc, payload, length);

  if (!error)
  {
    // Extraer valores del JSON y almacenarlos en variables
    JsonObject spObject = doc["SP"];
    for (int i = 0; i < 8; i++)
    {
      spValues[i] = spObject[String(i + 1)].as<int>();
      Serial.print("SP");
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.println(spValues[i]);
    }

    JsonObject svObject = doc["SV"];
    for (int i = 0; i < 4; i++)
    {
      svValues[i] = svObject[String(i + 1)].as<int>();
      Serial.print("SV");
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.println(svValues[i]);
    }
    if (!started)
    {
      started = true;
      // INICIA la tarea en paralelo

    }
  }
  else
  {
    Serial.print("Error al analizar el mensaje JSON: ");
    Serial.println(error.c_str());
  }
}

void setup()
{
  Serial.begin(9600);
  Serial.setTimeout(500);

  for (i = 0; i <= 2; i++) {
    pinMode(s1[i], OUTPUT);
  }
  for (i = 0; i <= 2; i++) {
    pinMode(s2[i], OUTPUT);
  }
  pinMode(D13, OUTPUT);
  //xTaskCreate(parallelTask, "parallelTask", 1024, NULL, 0, NULL);

  xTaskCreatePinnedToCore(
    parallelTask,   /* Task function. */
    "ligths",     /* name of task. */
    20000,       /* Stack size of task */
    NULL,        /* parameter of the task */
    4,           /* priority of the task */
    &Task1,      /* Task handle to keep track of created task */
    0);          /* pin task to core 0 */


  // Conectarse a AWS IoT
  connectAWS();
}

void loop()
{
  client.loop();

}
