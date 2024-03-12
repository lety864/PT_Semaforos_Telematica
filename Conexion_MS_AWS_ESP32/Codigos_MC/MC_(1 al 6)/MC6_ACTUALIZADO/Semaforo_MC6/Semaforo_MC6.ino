/* --------------------------------------------MC6-------------------------------------------------*/

//Variables del programa en general
int NuevoCiclo = 120000, nuevoValor,i = 0;
unsigned long t_comprobador=0;
boolean nuevoCiclo=false;
boolean cambio=false;


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

void setup() {

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
  SP_Comportamiento(SP8,NuevoCiclo,relacionRojo_P,&tiempo_1,&tiempo_2,&encender);
  SV_Comportamiento(SV4,NuevoCiclo,t_luces,relaciones_V,encender_V);
  comprobador(&NuevoCiclo,&nuevoValor);

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

  
