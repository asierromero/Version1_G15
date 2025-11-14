#include <DHT.h>
#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11); // RX, TX

const int led1 = 13; // LED verd indica enviament
const int led2 = 12; // LED vermell indica error

bool enviarDatos = true; // controla si enviar dades
unsigned long lastSendMillis = 0;
const unsigned long sendInterval = 2000UL; // enviar cada 2 segons

bool esperandoTimeout = true;
int nextTimeoutHT = millis() + 5000;

void setup() {
  Serial.begin(9600);    // comunicació amb el PC
  mySerial.begin(9600);
  dht.begin();
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);

}

void loop() {
  // Llegir comandes des del PC
  while (mySerial.available()) {
    String cmd = mySerial.readStringUntil('\n');
    cmd.trim();
    if (cmd.equalsIgnoreCase("Parar")) {
      enviarDatos = false;
    } else if (cmd.equalsIgnoreCase("Reanudar")) {
      enviarDatos = true;
    }
  }

  // Enviar dades si toca
  unsigned long now = millis();
  if (enviarDatos && (now - lastSendMillis >= sendInterval)) {
    lastSendMillis = now;

    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (!isnan(t) && !isnan(h)) {
      Serial.print("T:");
      Serial.print(t);
      Serial.print(":H:");
      Serial.println(h);

      mySerial.print("T:");
      mySerial.print(t);
      mySerial.print(":H:");
      mySerial.println(h);

      // LED breu
      digitalWrite(led1, HIGH);
    } 
    else {
      digitalWrite(led2, HIGH); // error lectura
      esperandoTimeout = true;
      nextTimeoutHT = millis() + 5000;
      if (esperandoTimeout && (millis() >= nextTimeoutHT)){
        mySerial.print("Fallo");
    }

    if (isnan(h) || isnan(t)){
      esperandoTimeout = true;
      nextTimeoutHT = millis() + 5000; // 5 segundos
    }
        


  // Apagar led1 després de 100ms
  if (millis() - lastSendMillis >= 100) {
    digitalWrite(led1, LOW);
  }
}
}
}
