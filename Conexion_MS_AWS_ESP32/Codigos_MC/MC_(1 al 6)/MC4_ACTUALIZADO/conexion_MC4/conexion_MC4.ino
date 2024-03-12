#include "secrets.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"

//SEMAFOROS
//Variables del programa en general
int NuevoCiclo = 120000, nuevoValor,i = 0;
unsigned long t_comprobador=0;
boolean nuevoCiclo=false;
static bool cambio=false;

//Vairables semáforo peatonal
unsigned long tiempo_1 = 0;
unsigned long tiempo_2 = 0;
boolean encender=true;
float relacionRojo_P=0;

//Orden peatonal Verde->Rojo
int SP5[2]={14,27};

//Variables para mensajes de AWS
boolean ultimoCiclo=true;
boolean continuar=true;
boolean nuevoMensaje=false;
boolean mensajeDetener=true;

/*
Pausa-->      1
Reinicio-->   2
*/

//VARIABLES PARA LA CONEXION AWS
WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);

#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/MC" //para recibir el json desde la nube
#define AWS_IOT_PUBLISH_TOPIC "MC/time" // para enviar el json a la nube

// VARAIBLE PARA OBTENER EL VALOR DEL JSON
int resultado, t, UltimoCiclo;

static bool started = false;//iniciar ciclo
TaskHandle_t Task1;


// FUNCION DE LA OPERACION EN PARALELO 
void parallelTask(void *pvParameters)  
{
  while (!started) {
    Serial.println("Esperando iniciar semaforo");
    delay(1000);
  }
  while (started) {

        t = resultado;
        nuevoValor = t*1000;

        //Codigo para que no reciba valor menor a 89000
        if (nuevoValor<89000){
          switch(nuevoValor){
            case 1000: //Pausa
              continuar=false;
              if(mensajeDetener){
                Serial.println("Luces detenidas");
                mensajeDetener=false; 
              }
            break;
        
            case 2000: //Reiniciar
              ESP.restart();
            break;
        
            default:
              nuevoValor=89000;
            break;

          }
        }

    if(nuevoValor>89000 && nuevoMensaje){
      // Mostrar el nuevo valor asignado
      Serial.print("Nuevo valor en milisegundos personalizado: ");
      Serial.print(nuevoValor);
      Serial.print(" Tiempo escrito en: ");
      Serial.println(millis()); 
      //Bandera
      cambio=true;
      nuevoMensaje=false;  
    }

    //llamado de las funciones para el comportamiento
    SP_Comportamiento(SP5,NuevoCiclo,relacionRojo_P,&tiempo_1,&tiempo_2,&encender);
    comprobador(&NuevoCiclo,&nuevoValor);
  }
}

// FUNCION PARA LA CONEXION CON AWS
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

// FUNCION PARA OBTENER EL JSON DE AWS
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
    resultado = doc["resultado"].as<int>();
    Serial.print("Resultado: ");
    Serial.print(resultado);

    cambio=true;
    nuevoMensaje=true;

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

// FUNCION PARA ENVIAR EL JSON DE AWS ULTIMO CICLO
void PublishJson()
{
  // Crear un objeto DynamicJsonDocument para almacenar el JSON a enviar
  DynamicJsonDocument doc(512);

  // Creando JSON
  doc["UltimoCiclo_MC4"] = UltimoCiclo;
  String json;
  serializeJson(doc, json);

  if(client.publish(AWS_IOT_PUBLISH_TOPIC, json.c_str())) {
    Serial.println("JSON enviado correctamente");
  } 
  else{
      Serial.println("Error al enviar JSON");
    }
}


//////////////////////////////////////////////////////////RECONEXION a INTERNET Y A AWS///////////////////////////////////////////////////

void reconnectWiFi() {  //Reconectar a internet
  // Si ya estás conectado, no hay nada que hacer
  if (WiFi.status() == WL_CONNECTED) return;

  Serial.println("Conectando a Wi-Fi...");
  
  // Intenta reconectar
  if(ssid=="Wi-Fi IPN")
    WiFi.begin(ssid);
  else
    WiFi.begin(ssid,password);

  // Espera hasta que se establezca la conexión
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n¡Conectado a Wi-Fi!");
}

void ensureMqttConnection() {  //reconectar a AWS
  if (!client.connected()) {
    Serial.println("Reconectando al servidor AWS IoT...");
    while (!client.connected()) {
      if (client.connect(THINGNAME)) {
        // Vuelve a suscribirte a los temas necesarios aquí
        client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
      } else {
        Serial.print(".");
        delay(1000);
      }
    }
    Serial.println("\n¡Reconectado a AWS IoT!");
  }
} 

//////////////////////////////////////////////////////////FUNCIONES SEMAFOROS////////////////////////////////////////////////////////////

// CAMBIO DE CICLO DENTRO DE LOS PRIMEROS 15 SEGUNDOS

void comprobador(int *NuevoCiclo,int *nuevoValor){
  //Funcion que debe verificar si es posible cambiar el nuevo valor 

    if(nuevoCiclo && cambio){
      *NuevoCiclo=*nuevoValor;
      Serial.print("Valor modificado en: ");
      Serial.println(millis());
      cambio=false;
    }
    if(millis()<=t_comprobador+15000){        
      nuevoCiclo=true;
    }else{
      nuevoCiclo=false;
      if(ultimoCiclo){
        //Serial.print("Ultimo ciclo verificado es: ");
        //Serial.println(*NuevoCiclo);
        UltimoCiclo = *NuevoCiclo;
        PublishJson();
        //Bandera
        ultimoCiclo=false;
      }
    } 
}

// COMPORTAMIENTO PARA EL SEMAFORO PEATONAL

void SP_Comportamiento(int leds[],int NuevoCiclo,float relacionRojo,unsigned long *t1,unsigned long *t2,boolean *encender){
//Enciende de acuerdo a relacionRojo, el tiempo de verde es el resto
  if(millis()>*t1+(NuevoCiclo*relacionRojo) && *encender){
  *t1 = millis();
  //print_tiempo(*t1);
  //Serial.println("Empieza luz en verde");
  digitalWrite(leds[0], LOW);
  digitalWrite(leds[1], HIGH);
  *encender=false;
  }
  //Tiempo en rojo 
  if(millis()>*t2+NuevoCiclo){
  *t2 = millis();
  //print_tiempo(*t2);
  //Serial.println("Empieza luz en rojo");
  digitalWrite(leds[0], HIGH);
  digitalWrite(leds[1], LOW);
  *encender=true;
  *t1=*t2;
  t_comprobador=*t2;

  }
}

// IMPRIME LOS MILIS EN SEGUNDOS

void print_tiempo(unsigned long tiempo_millis){
    Serial.print("Tiempo: ");
    Serial.print(tiempo_millis/1000);
    Serial.print("ms - ");
}



void setup()
{
  Serial.begin(9600);
  for (i = 0; i < 2; i++) {
    pinMode(SP5[i], OUTPUT);
  }
 
  //Inicialización SP5- Empieza en rojo
  digitalWrite(SP5[0], HIGH);
  digitalWrite(SP5[1], LOW);

  
  //Relacion Rojo SP5
  relacionRojo_P=100.0/120;

  //xTaskCreate(parallelTask, "parallelTask", 1024, NULL, 0, NULL);
  xTaskCreatePinnedToCore(
    parallelTask,   /* Task function. */
    "ligths",     /* name of task. */
    20000,       /* Stack size of task */
    NULL,        /* parameter of the task */
    0,           /* priority of the task */
    &Task1,      /* Task handle to keep track of created task */
    0);          /* pin task to core 0 */


  // Conectarse a AWS IoT
  connectAWS();
}

void loop()
{
  if (WiFi.status() != WL_CONNECTED) {
    reconnectWiFi();
  }
  if (!client.connected()) {
    ensureMqttConnection();
  }
  client.loop();
}