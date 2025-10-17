#include <SoftwareSerial.h>
#include <DHT.h>

#define DHTPIN 2
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE); // Inicializa el sensor DHT
SoftwareSerial mySerial(10, 11); // RX, TX

void setup() { 
  Serial.begin(9600);
  mySerial.begin(9600);
  dht.begin();
}

void loop() {
  delay(2000); // tiempo recomendado entre lecturas del DHT11

  float h = dht.readHumidity(); // Lee la humedad del sensor
  float t = dht.readTemperature(); // Lee la temperatura del sensor

  if (isnan(h) || isnan(t)) {  // Comprobar si la lectura del sensor fue correcta
    Serial.println("Error al leer el sensor DHT11");
  } else {
   //Envío por puerto serial del otro arduino (el otro ordenador)
    mySerial.print("T:");
    mySerial.print(t);
    mySerial.print(":H:");
    mySerial.print(h);
    mySerial.println("\n");

    //Printar en este mismo ordenador para comprobar
    Serial.print("T:");
    Serial.print(t);
    Serial.print(":H:");
    Serial.print(h);
    Serial.print("\n");
  }

}