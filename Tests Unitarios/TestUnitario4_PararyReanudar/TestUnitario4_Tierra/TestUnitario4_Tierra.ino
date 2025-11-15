#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11); // RX, TX 

void setup() {
   Serial.begin(9600);   // conexión con Python
   mySerial.begin(9600); // conexión con satélite
}

void loop() {
   // Recibir datos del satélite y enviarlos a Python
   while (mySerial.available()) {
      String data = mySerial.readStringUntil('\n');
      Serial.println(data);
   }

   // Recibir comandos de Python y enviarlos al satélite
   while (Serial.available()) {
      String cmd = Serial.readStringUntil('\n');
      mySerial.println(cmd);
   }
}
