/* --------------------------------------------MC5-------------------------------------------------*/

//Variables del programa en general
int NuevoCiclo = 120000, nuevoValor,i = 0;
unsigned long t_comprobador=0;
boolean nuevoCiclo=false;
boolean cambio=false;


//Vairables semáforo peatonal SP7
unsigned long tiempo_1 = 0;
unsigned long tiempo_2 = 0;
boolean encender=true;
float relacionRojo_P7=0;

//Vairables semáforo peatonal SP6
unsigned long tiempoP6_1 = 0;
unsigned long tiempoP6_2 = 0;
boolean encenderP6=true;
float relacionRojo_P6=0;

//Orden peatonal ROJO->VERDE
int SP7[2]={26,27}, SP6[2]= {33,25};


void setup() {

 
  for (i = 0; i < 2; i++) {
    pinMode(SP7[i], OUTPUT);
  }

  for (i = 0; i < 2; i++) {
    pinMode(SP6[i], OUTPUT);
  }
 
  //Inicialización SP7- Empieza en rojo
  digitalWrite(SP7[0], HIGH);
  digitalWrite(SP7[1], LOW);

  //Inicialización SP6- Empieza en rojo
  digitalWrite(SP6[0], HIGH);
  digitalWrite(SP6[1], LOW);

  
  //Relacion Rojo SP7
  relacionRojo_P7=100.0/120;
  //Relacion Rojo SP6
  relacionRojo_P6=50.0/120;

  Serial.begin(9600);
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
  SP_Comportamiento(SP7,NuevoCiclo,relacionRojo_P7,&tiempo_1,&tiempo_2,&encender);
  SP_Comportamiento(SP6,NuevoCiclo,relacionRojo_P6,&tiempoP6_1,&tiempoP6_2,&encenderP6);
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
  
void print_tiempo(unsigned long tiempo_millis){
    Serial.print("Tiempo: ");
    Serial.print(tiempo_millis);
    Serial.print("ms - ");
}
