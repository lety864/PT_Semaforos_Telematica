/* --------------------------------------------MC4-------------------------------------------------*/

//Variables del programa en general
int NuevoCiclo = 120000, nuevoValor,i = 0;
unsigned long t_comprobador=0;
boolean nuevoCiclo=false;
boolean cambio=false;


//Vairables semáforo peatonal
unsigned long tiempo_1 = 0;
unsigned long tiempo_2 = 0;
boolean encender=true;
float relacionRojo_P=0;



//Orden peatonal Verde->Rojo
int SP5[2]={14,27};


void setup() {

 
  for (i = 0; i < 2; i++) {
    pinMode(SP5[i], OUTPUT);
  }
 
  //Inicialización SP5- Empieza en rojo
  digitalWrite(SP5[0], HIGH);
  digitalWrite(SP5[1], LOW);

  
  //Relacion Rojo SP5
  relacionRojo_P=100.0/120;

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
  SP_Comportamiento(SP5,NuevoCiclo,relacionRojo_P,&tiempo_1,&tiempo_2,&encender);
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
    Serial.print(tiempo_millis);
    Serial.print("ms - ");
}
