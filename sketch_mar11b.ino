void setup(){
 Serial.begin(9600); 
 pinMode(2, OUTPUT);
 pinMode(3, OUTPUT);
 pinMode(4, OUTPUT);
 pinMode(5, OUTPUT);
 digitalWrite(2, LOW);

 digitalWrite(3, LOW); 
  digitalWrite(4, LOW); 
 digitalWrite(5, LOW); 
}



void loop(){
 if (Serial.available() > 0){
   char com = Serial.read();
   Serial.print(com);
    if (com == '4') { // вперед
      digitalWrite(2, LOW); 
      digitalWrite(3, HIGH); // правая вперед 
      digitalWrite(4, HIGH);  //Лвая вперед
      digitalWrite(5, LOW); 
   }
      else if (com == '2') { //право
      digitalWrite(2,  HIGH); //правая назад
      digitalWrite(3, LOW); 
      digitalWrite(4, HIGH); //левая вперед
      digitalWrite(5, LOW); 
   }
      else if (com == '3') { //назад
      digitalWrite(2, HIGH); // правая назад
      digitalWrite(3, LOW); 
      digitalWrite(4, LOW); 
      digitalWrite(5, HIGH); //левая назад
   }
      else if (com == '1') {//лево
      digitalWrite(2, LOW); 
      digitalWrite(3, HIGH); 
      digitalWrite(4, LOW); 
      digitalWrite(5, HIGH); 
   }
   
      else if (com == '0') {
      digitalWrite(2, LOW); 
      digitalWrite(3, LOW); 
      digitalWrite(4, LOW); 
      digitalWrite(5, LOW); 
   }
   
 }
  
  
}
