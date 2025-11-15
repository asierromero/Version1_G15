// Definiciones necesarias
#include <DHT.h>
#include <SoftwareSerial.h>
#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

int i;
SoftwareSerial mySerial(10, 11); // RX, TX 

void setup() {
   Serial.begin(9600);
   mySerial.begin(9600);
   // Inicialización del sensor
   dht.begin();
}



void loop() {
   delay(2000);
   float h = dht.readHumidity();
   float t = dht.readTemperature();
   if (isnan(h) || isnan(t))
      Serial.println("Error al leer el sensor DHT11");
   else {
      Serial.print("Humedad: ");
      Serial
.print(h);
      Serial.print("%\t");
      Serial.print("Temperatura: ");
      Serial.print(t);
      Serial.println("°C");

      mySerial.print("Humedad: ");
      mySerial.print(h);
      mySerial.print("%\t");
      mySerial.print("Temperatura: ");
      mySerial.print(t);
      mySerial.println("°C");
   }
}