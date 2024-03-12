/* --------------------------------------------MC1-------------------------------------------------*/

//Variables del programa en general
int NuevoCiclo = 89000, nuevoValor,i = 0;
unsigned long t_comprobador=0;
boolean nuevoCiclo=false;
boolean cambio=false;


//Vairables semáforo peatonal SP1
unsigned long tiempoP1_1 = 0;
unsigned long tiempoP1_2 = 0;
boolean encenderP1=true;
float relacionRojo_P1=0;
//Orden peatonal Verde->Rojo
int SP1[2]={4,15};

//Variables semaforo vehicular
unsigned long t_luces[5]={0,0,0,0,0};
boolean encenderV1[3]={true,false,true};
float relacionVerde_V1=0;
float relacionAmarillo_V1=0;
float relacionParpadeo_V1=0;
int contador_parpadeo=0;
int stuck_contador=9;
//Orden Vehicular Rojo->Amarillo->Verde
int SV1[3]={27,14,13};

void setup() {

  Serial.begin(9600);
  for (i = 0; i < 3; i++) {
    pinMode(SV1[i], OUTPUT);
  }
 
  for (i = 0; i < 2; i++) {
    pinMode(SP1[i], OUTPUT);
  }
 
  //Inicialización SP1- Empieza en rojo
  digitalWrite(SP1[0], HIGH);
  digitalWrite(SP1[1], LOW);

  //inicializacion SV1- Empieza en verde
  digitalWrite(SV1[0],LOW);
  digitalWrite(SV1[1],HIGH);
  digitalWrite(SV1[2],HIGH);

  //Relacion Rojo SP1
  relacionRojo_P1=100.0/120;

  //Relacion SV1
  relacionVerde_V1=33.0/120;
  relacionAmarillo_V1=2.0/120;
  relacionParpadeo_V1=3.0/120;
}

void loop() {

  // Verificar si hay datos disponibles en el puerto serial
  if (Serial.available() > 0) {
    // Leer el valor ingresado por el usuario
    nuevoValor = Serial.parseInt();
    Serial.read();

    // Mostrar el nuevo valor asignado
    Serial.print("Nuevo valor en milisegundos personalizado: ");
    Serial.print(nuevoValor);
    Serial.print(" Tiempo escrito en: ");
    Serial.println(millis());
    
    //Bandera
    cambio=true;
   
  }
  SP_Comportamiento(SP1,NuevoCiclo,relacionRojo_P1,&tiempoP1_1,&tiempoP1_2,&encenderP1);
  SV_Comportamiento(SV1,NuevoCiclo,t_luces,relacionVerde_V1,relacionAmarillo_V1,encenderV1);
  comprobador(&NuevoCiclo,&nuevoValor);

}


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
    }
    
}
  
// COMPORTAMIENTO PARA EL SEMAFORO VEHICULAR

void SV_Comportamiento(int leds[],int NuevoCiclo,unsigned long *t1,float relacionVerde,float relacionAmarillo,boolean *encender){
  //Funcion de comportamiento de Semáforo vehicular de 3 luces 
  if(contador_parpadeo<9){
    if(millis()>=*(t1+3)+(NuevoCiclo*relacionVerde)-(NuevoCiclo*(relacionParpadeo_V1/9)*stuck_contador) && millis()<=*(t1+3)+(NuevoCiclo*relacionVerde) && *(encender+2) ){
      //Serial.print("Empieza parpadeo en: ");
      //Serial.println(millis());
      digitalWrite(leds[0],!digitalRead(leds[0]));
      contador_parpadeo++;
      stuck_contador--;
    }
  }    

  if(millis()>*t1+(NuevoCiclo*relacionVerde) && *encender){
    *t1=millis();
    print_tiempo(*t1);
    Serial.println("Empieza luz amarilla");
    digitalWrite(leds[0], HIGH);
    digitalWrite(leds[1],LOW);
    *encender=false;
    *(encender+2)=false;
    *(encender+1)=true;
    contador_parpadeo=0;
    stuck_contador=9;
  } 
    

  if(millis()>*(t1+1)+(NuevoCiclo*relacionAmarillo)+(NuevoCiclo*relacionVerde) && *(encender+1)){
  *(t1+1)=millis();
  print_tiempo(*(t1+1));
  Serial.println("Empieza luz roja");
  digitalWrite(leds[1], HIGH);
  digitalWrite(leds[2], LOW);
  *(encender+1)=false;
  }

  if(millis()>*(t1+2)+NuevoCiclo){
    *(t1+2)=millis();
    print_tiempo(*(t1+2));
    Serial.println("Empieza luz verde");
    digitalWrite(leds[2], HIGH);
    digitalWrite(leds[0], LOW);
    *encender=true;
    *(encender+2)=true;
    *t1=*(t1+2);
    *(t1+1)=*(t1+2);
    *(t1+3)=*(t1+2);
    t_comprobador=*(t1+2);
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
