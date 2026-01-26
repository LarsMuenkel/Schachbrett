int CountButtons(){
  int Buttonpushcounter = 0;
  while(true){
      if(digitalRead(buttonPin) == LOW){
          Buttonpushcounter++;
          //Serial.println(Buttonpushcounter); 
          delay(300); 
          } 
      
      if(digitalRead(DelButton) == LOW){
          Buttonpushcounter = Buttonpushcounter - 1;
          //Serial.println(Buttonpushcounter); 
          delay(300);
          }
     if(digitalRead(OkButton) == LOW){
          delay(300);
          return Buttonpushcounter; 
          }
     }
}



  
  
