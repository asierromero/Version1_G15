// Definiciones necesarias
#include <DHT.h>
#include <SoftwareSerial.h>
#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

int i;
SoftwareSerial mySerial(10, 11); // RX, TX 


const int led1 = 13;  // LED en el pin 13 (Verde)
const int led2 = 12;  // LED en el pin 12 (Rojo)



void setup() {
   Serial.begin(9600);
   mySerial.begin(9600);
   // Inicialización del sensor
   dht.begin();

   pinMode(led1, OUTPUT);
   pinMode(led2, OUTPUT);
}



void loop() {
   delay(1000);
   float h = dht.readHumidity();
   float t = dht.readTemperature();

   if (isnan(h) || isnan(t)){
      Serial.println("Error al leer el sensor DHT11");
      digitalWrite(led2, HIGH);
   }
   else {
      Serial.print("Humedad: ");
      Serial.print(h);
      Serial.print("%\t");
      Serial.print("Temperatura: ");
      Serial.print(t);
      Serial.println("°C");

      mySerial.print("T:");
      mySerial.print(t);
      mySerial.print(":H:");
      mySerial.print(h);
      mySerial.println("\n");
      
      digitalWrite(led2, LOW);

      digitalWrite(led1, HIGH); // Enciende el LED
      delay(100);
      digitalWrite(led1, LOW);  // Apaga el LED
      delay(2000); 

   }
   

}