/* --------------------------------------------MC2-------------------------------------------------*/

//Variables del programa en general
int NuevoCiclo = 120000, nuevoValor,i = 0;
unsigned long t_comprobador=0;
boolean nuevoCiclo=false;
boolean cambio=false;


//Variables semáforo peatonal SP3
unsigned long tiempo_1 = 0;
unsigned long tiempo_2 = 0;
boolean encender3=true;
float relacionRojo_P3=0;
//Orden peatonal Verde-->Rojo
int SP3[2]={27,26};


//Variables semáforo peatonal SP2
unsigned long tiempo_3 = 0;
unsigned long tiempo_4 = 0;
boolean encender2=true;
float relacionRojo_P2=0;
//Orden peatonal Verde-->Rojo
int SP2[2]={25,33};


//Variables semaforo vehicular
unsigned long t_luces[5]={0,0,0,0,0};
boolean encender_V[4]={true,true,true,false};

//Relaciones depende del numero de comportamientos que hay
//En este caso son 4 sin contar el de parpadeo
//Primer rojo, verde,amarillo,segundo rojo
float relaciones_V2[3]={0,0,0};
int contador_parpadeo=0;
int stuck_contador=9;
float relacionParpadeo_V2=0;
//Orden vehicular Verde1---->Verde2---->Amarillo--->Rojo
int SV2[4]={18,5,19,21};


void setup() {

  Serial.begin(9600);
  for (i = 0; i < 4; i++) {
    pinMode(SV2[i], OUTPUT);
  }
 
  for (i = 0; i < 2; i++) {
    pinMode(SP3[i], OUTPUT);
    pinMode(SP2[i], OUTPUT);
  }
 
  //Inicialización SP2-Empieza en rojo
  digitalWrite(SP2[0], HIGH);
  digitalWrite(SP2[1], LOW);

  //Inicialización SP3-Empieza en rojo
  digitalWrite(SP3[0], HIGH);
  digitalWrite(SP3[1], LOW);


  //inicializacion SV2- Empieza en verde
  digitalWrite(SV2[0],LOW);
  digitalWrite(SV2[1],HIGH);
  digitalWrite(SV2[2],HIGH);
  digitalWrite(SV2[3],HIGH);

  //Relacion Rojo SP3
  relacionRojo_P3=100.0/120;

  //Relacion Rojo SP2
  relacionRojo_P2=35.0/120;

  //Relacion SV
  relaciones_V2[0]=48.0/120; //48 s en Verde1
  relaciones_V2[1]=35.0/120; //12 s en Verde2 (Vuelta a la izquierda)
  relaciones_V2[2]=2.0/120; //2 s en amarillo
  relacionParpadeo_V2=3.0/120;
}

void loop() {
   
  //MC6 tiene 2 semáforos
  /*
  SP8:100 s rojo---> 20 s verde  
  SV4: 50 s rojo--->45 s verde---->tintineo 3 s--->2 s amarillo-->20 s rojo 

  Pines de uso ESP32:
  
  2 relevadores (peatonal)
  D15,D4

  3 relevadores para vehicular 

  1 relevadores:
  D13

  2 relevadores:
  D14
  D27
  */

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
  SP_Comportamiento(SP3,NuevoCiclo,relacionRojo_P3,&tiempo_1,&tiempo_2,&encender3);
  SP_Comportamiento(SP2,NuevoCiclo,relacionRojo_P2,&tiempo_3,&tiempo_4,&encender2);
  SV_Comportamiento4(SV2,NuevoCiclo,t_luces,relaciones_V2,encender_V);
  comprobador(&NuevoCiclo,&nuevoValor);

}




void SV_Comportamiento4(int leds[],int NuevoCiclo,unsigned long *t1,float *relaciones_V,boolean *encender){
  //Funcion de comportamiento de Semáforo vehicular de 3 luces 
  
  
  if(millis()>*t1+((*relaciones_V)*NuevoCiclo) && *encender){
    *t1=millis();
    print_tiempo(*t1);
    Serial.println("Empieza luz amarilla");
    digitalWrite(leds[2], LOW);
    digitalWrite(leds[0],HIGH);
    digitalWrite(leds[1],HIGH);
    *encender=false;
    *(encender+2)=false;
    *(encender+3)=true;
    contador_parpadeo=0;
    stuck_contador=9;
  } 
  
  if(millis()>*(t1+2)+(NuevoCiclo*(*(relaciones_V+1))) && *(encender+1)){
  *(t1+2)=millis();
  print_tiempo(*(t1+2));
  Serial.println("Empieza luz Verde 2");
  digitalWrite(leds[1], LOW);
  *(encender+1)=false;
  *(encender+3)=true;
  }


  if(contador_parpadeo<9){
    if(millis()>=*(t1+1)+(NuevoCiclo*(*(relaciones_V)))-(NuevoCiclo*(relacionParpadeo_V2/9)*stuck_contador) && millis()<=*(t1+1)+(NuevoCiclo*(*(relaciones_V))) && *(encender+2) ){
      //Serial.print("Empieza parpadeo en: ");
      //Serial.println(millis());
      digitalWrite(leds[0],!digitalRead(leds[0]));
      digitalWrite(leds[1],!digitalRead(leds[1]));
      contador_parpadeo++;
      stuck_contador--;
    }
  }    

   
  
  if(millis()>*(t1+3)+(NuevoCiclo*(*(relaciones_V)))+(NuevoCiclo*(*(relaciones_V+2))) && *(encender+3)){
  *(t1+3)=millis();
  print_tiempo(*(t1+3));
  Serial.println("Empieza luz roja");
  digitalWrite(leds[2],HIGH);
  digitalWrite(leds[3], LOW);
  *(encender+3)=false;
  }



  if(millis()>*(t1+4)+NuevoCiclo){
    *(t1+4)=millis();
    print_tiempo(*(t1+4));
    Serial.println("Empieza luz verde");
    digitalWrite(leds[0],LOW);
    digitalWrite(leds[3],HIGH);
    *encender=true;
    *(encender+1)=true;
    *(encender+2)=true;
    *t1=*(t1+4);
    *(t1+1)=*(t1+4);
    *(t1+2)=*(t1+4);
    *(t1+3)=*(t1+4);
    t_comprobador=*(t1+4);
  }
}
  

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
  
void print_tiempo(unsigned long tiempo_millis){
    Serial.print("Tiempo: ");
    Serial.print(tiempo_millis);
    Serial.print("ms - ");
}

  

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
