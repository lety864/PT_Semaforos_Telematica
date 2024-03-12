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


//Variables semáforo peatonal
unsigned long tiempo_1 = 0;
unsigned long tiempo_2 = 0;
boolean encender=true;
float relacionRojo_P=0;
//Orden peatonal Verde->Rojo
int SP8[2]={4,15};

//Variables semaforo vehicular
unsigned long t_luces[5]={0,0,0,0,0};
boolean encender_V[4]={true,false,true,false};

//Relaciones depende del numero de comportamientos que hay
//En este caso son 4 sin contar el de parpadeo
//Primer rojo, verde,amarillo,segundo rojo
float relaciones_V[4]={0,0,0,0};
int contador_parpadeo=0;
int stuck_contador=9;
//Orden Vehicular Verde->Amarillo->Rojo
int SV4[3]={27,14,13};
float relacionParpadeo_V=0;

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
    SP_Comportamiento(SP8,NuevoCiclo,relacionRojo_P,&tiempo_1,&tiempo_2,&encender);
    SV_Comportamiento(SV4,NuevoCiclo,t_luces,relaciones_V,encender_V);
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
  doc["UltimoCiclo_MC6"] = UltimoCiclo;
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

// COMPORTAMIENTO PARA EL SEMAFORO VEHICULAR

void SV_Comportamiento(int leds[],int NuevoCiclo,unsigned long *t1,float *relaciones_V,boolean *encender){
  //Funcion de comportamiento de Semáforo vehicular de 3 luces 
  
  
  if(millis()>*t1+((*relaciones_V)*NuevoCiclo) && *encender){
    *t1=millis();
    print_tiempo(*t1);
    Serial.println("Empieza luz verde");
    digitalWrite(leds[2], HIGH);
    digitalWrite(leds[0],LOW);
    *encender=false;
    *(encender+1)=true;
    contador_parpadeo=0;
  } 
  

  if(contador_parpadeo<9){
    if(millis()>=*(t1+1)+(NuevoCiclo*(*(relaciones_V)))+(NuevoCiclo*(*(relaciones_V+1)))-(NuevoCiclo*(relacionParpadeo_V/9)*stuck_contador) && millis()<=*(t1+1)+(NuevoCiclo*(*(relaciones_V)))+(NuevoCiclo*(*(relaciones_V+1))) && *(encender+2) ){
      //Serial.print("Empieza parpadeo en: ");
      //Serial.println(millis());
      digitalWrite(leds[0],!digitalRead(leds[0]));
      contador_parpadeo++;
      stuck_contador--;
    }
  }    

  
  if(millis()>*(t1+2)+(NuevoCiclo*(*(relaciones_V)))+(NuevoCiclo*(*(relaciones_V+1))) && *(encender+1)){
  *(t1+2)=millis();
  print_tiempo(*(t1+2));
  Serial.println("Empieza luz amarilla");
  digitalWrite(leds[1], LOW);
  digitalWrite(leds[0], HIGH);
  *(encender+1)=false;
  *(encender+2)=false;
  *(encender+3)=true;
  contador_parpadeo=0;
  stuck_contador=9;
  }

  if(millis()>*(t1+3)+(NuevoCiclo*(*(relaciones_V)))+(NuevoCiclo*(*(relaciones_V+1)))+(NuevoCiclo*(*(relaciones_V+2))) && *(encender+3)){
  *(t1+3)=millis();
  print_tiempo(*(t1+3));
  Serial.println("Empieza luz roja");
  digitalWrite(leds[2], LOW);
  digitalWrite(leds[1], HIGH);
  *(encender+3)=false;
  }



  if(millis()>*(t1+4)+NuevoCiclo){
    *(t1+4)=millis();
    print_tiempo(*(t1+4));
    Serial.println("Empieza luz roja");
    digitalWrite(leds[2],LOW);
    *encender=true;
    *(encender+2)=true;
    *t1=*(t1+4);
    *(t1+1)=*(t1+4);
    *(t1+2)=*(t1+4);
    *(t1+3)=*(t1+4);
    t_comprobador=*(t1+4);
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
  for (i = 0; i < 3; i++) {
    pinMode(SV4[i], OUTPUT);
  }
 
  for (i = 0; i < 2; i++) {
    pinMode(SP8[i], OUTPUT);
  }
 
  //Inicialización SP- Empieza en rojo
  digitalWrite(SP8[0], HIGH);
  digitalWrite(SP8[1], LOW);

  //inicializacion SV- Empieza en rojo
  digitalWrite(SV4[0],HIGH);
  digitalWrite(SV4[1],HIGH);
  digitalWrite(SV4[2],LOW);

  //Relacion Rojo SP
  relacionRojo_P=100.0/120;

  //Relacion SV
  relaciones_V[0]=50.0/120; //50 s en rojo1
  relaciones_V[1]=48.0/120; //45 s en verde
  relaciones_V[2]=2.0/120; //2 s en amarillo
  relaciones_V[3]=20.0/120; //20 s en rojo2
  relacionParpadeo_V=3.0/120;

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