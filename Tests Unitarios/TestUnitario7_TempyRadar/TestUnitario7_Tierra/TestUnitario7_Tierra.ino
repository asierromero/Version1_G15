/*
#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11); // RX, TX

const int BUZZER = 3;
const int led1 = 13; // LED verde (envíos)
const int led2 = 12; // LED rojo (error)

// Variables control LED verde
unsigned long lastDataTime = 0;
const unsigned long LED_DELAY = 100;

// Variables timeout
unsigned long lastDataReceived = 0;
const unsigned long TIMEOUT_COMUNICACION = 5000;

void setup() {
  Serial.begin(9600);   // Conexión con Python
  mySerial.begin(9600); // Conexión con satélite

  pinMode(BUZZER, OUTPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  
  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
  
  lastDataReceived = millis();
}

void loop() {
  // Procesar datos desde satélite
  if (mySerial.available()) {
    String data = mySerial.readStringUntil('\n');
    data.trim();
    Serial.println(data); // Enviar a Python
    
    lastDataReceived = millis();
    digitalWrite(led2, LOW); // Apagar error comunicación
    
    // Encender LED verde al recibir datos
    digitalWrite(led1, HIGH);
    lastDataTime = millis();
    
    // Procesar tipo de mensaje
    int fin = data.indexOf(':');
    if(fin != -1) {
      int codigo = data.substring(0, fin).toInt();
      
      switch(codigo) {
        case 3: // Fallo en sensores
          tone(BUZZER, 2000);
          digitalWrite(led2, HIGH);
          break;
      }
    }
  }
  
  // Procesar comandos desde Python
  while (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    mySerial.println(cmd);
  }
  
  // Apagar LED verde después de delay
  if (millis() - lastDataTime >= LED_DELAY) {
    digitalWrite(led1, LOW);
  }
  
  // Verificar timeout de comunicación
  if (millis() - lastDataReceived >= TIMEOUT_COMUNICACION) {
    digitalWrite(led2, HIGH);
    tone(BUZZER, 1000);
  } else {
    noTone(BUZZER);
  }
}
*/

#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11); // RX, TX

const int BUZZER = 3;
const int led1 = 13; // LED verde (envíos correctos)
const int led2 = 12; // LED rojo (error temperatura o fallo comunicacion)
const int led3 = 6;  // LED azul (error sensor de distancia)

// Variables de control
unsigned long lastDataTime = 0;
const unsigned long LED_DELAY = 100;      // Duración LED verde
const unsigned long TIMEMAX = 3000;   // Tiempo máximo sin datos (5 s)
bool falloComunicacion = false;

void setup() {
   Serial.begin(9600);   // Conexión con Python
   mySerial.begin(9600); // Conexión con satélite

   pinMode(BUZZER, OUTPUT);
   pinMode(led1, OUTPUT);
   pinMode(led2, OUTPUT);
   pinMode(led3, OUTPUT);

   digitalWrite(led1, LOW);
   digitalWrite(led2, LOW);
   digitalWrite(led3, LOW);

   lastDataTime = millis(); // Inicializar el temporizador
}

void loop() {
   // --- Recepción desde el satélite ---
   while (mySerial.available()) {
      String data = mySerial.readStringUntil('\n');
      data.trim();
      Serial.println(data);  // Enviar a Python

      // Indicar comunicación activa
      digitalWrite(led1, HIGH);
      lastDataTime = millis(); // Actualizar hora del último dato

      // Si el dato contiene "Fallo", activar alarma
      if (data.indexOf("Fallo") >= 0) {
         tone(BUZZER, 2000);
         digitalWrite(led2, HIGH);
         falloComunicacion = true;
      }
      if (data.indexOf("Fallo Radar") >= 0) {
         tone(BUZZER, 2000);
         digitalWrite(led3, HIGH);

      // Si contiene datos correctos (por ejemplo, T: y H:)
      if (data.indexOf("T:") >= 0 && data.indexOf("H:") >= 0) {
         noTone(BUZZER);
         digitalWrite(led2, LOW);
         digitalWrite(led3, LOW);
         falloComunicacion = false;
      }
   }

   // --- LED verde intermitente ---
   if (millis() - lastDataTime >= LED_DELAY) {
      digitalWrite(led1, LOW);
   }

   // --- Detección de pérdida de comunicación ---
   if ((millis() - lastDataTime) > TIMEMAX && !falloComunicacion) {
      // Si pasa más del tiempo límite sin datos
      tone(BUZZER, 2000);
      digitalWrite(led2, HIGH);
      falloComunicacion = true;
      Serial.println("FalloComunicacion");  //avisar a Tierra
   }

   // --- Reenviar comandos de Python al satélite ---
   while (Serial.available()) {
      String cmd = Serial.readStringUntil('\n');
      mySerial.println(cmd);
   }
}
}
