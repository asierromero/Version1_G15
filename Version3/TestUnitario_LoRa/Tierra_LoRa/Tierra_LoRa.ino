#include <SoftwareSerial.h>

SoftwareSerial mySerial(10, 11);

void setup() {
  Serial.begin(9600);     
  mySerial.begin(9600);

}

void loop() {
  // Leer una línea completa de Python (hasta caracter de nueva línea)
  String cmd = mySerial.readStringUntil('\n');
  
  Serial.print(cmd); 


}

