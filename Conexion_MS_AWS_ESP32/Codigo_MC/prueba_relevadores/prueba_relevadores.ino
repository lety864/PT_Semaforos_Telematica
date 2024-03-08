#define D13 13

int i, s1[2]={15,4}, s2[2]={14,27};





void setup() {
  for(i=0; i<=2; i++){
    pinMode(s1[i], OUTPUT);
  }
  for(i=0; i<=2; i++){
    pinMode(s2[i], OUTPUT);
  }

  pinMode(D13, OUTPUT);

}

void loop() {

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
