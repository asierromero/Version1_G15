#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11); // RX, TX



void setup() {
   // Inicia el puerto serie 
   Serial.begin(9600); 
   mySerial.begin(9600);

   
}

void loop() {
   if (mySerial.available()) {
      String data = mySerial.readString();  //Lee los datos recibidos
      Serial.print(data); 
      
   }
}

