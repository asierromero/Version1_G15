#include <SoftwareSerial.h>

SoftwareSerial mySerial(10, 11);

void setup() {
  Serial.begin(9600);     
  mySerial.begin(9600);

}

void loop() {

  mySerial.println("Hola Tierra\n");

  Serial.print("Hola\n");

  delay(3000);

}
