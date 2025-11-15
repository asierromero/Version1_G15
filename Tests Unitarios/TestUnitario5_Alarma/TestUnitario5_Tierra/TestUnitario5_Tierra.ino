#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11); // RX, TX

const int BUZZER = 3;
const int led1 = 13; //led verd (enviaments)
const int led2 = 12; //led vermell (error)

//variables control led verd
unsigned long lastDataTime = 0;
const unsigned long LED_DELAY = 100;

//variables de l'alarma
bool esperandoTimeout = true;
unsigned long nextTimeoutHT = 0;

void setup() {
   Serial.begin(9600);   // conexió amb Python
   mySerial.begin(9600); // conexió amb satèl·lit

   nextTimeoutHT = millis() + 3000;

   pinMode(BUZZER, OUTPUT);
   pinMode(led1, OUTPUT);
   pinMode(led2, OUTPUT);
   
   // Inicialmente LEDs apagados
   digitalWrite(led1, LOW);
   digitalWrite(led2, LOW);
}

void loop() {
   // Dades de satèl·lit a Python
   while (mySerial.available()) {
      String data = mySerial.readStringUntil('\n');
      data.trim();
      Serial.println(data);

      // Encendre verd quan es rebin dades
      digitalWrite(led1, HIGH);
      lastDataTime = millis();
     
      if (data.indexOf("Fallo") >= 0){
         tone(BUZZER, 2000);
         digitalWrite(led2, HIGH); // Encendre led vermella- Fallo
      }
     
      if (data.indexOf("T:") >= 0 && data.indexOf("H:") >= 0){
         noTone(BUZZER);
         digitalWrite(led2, LOW);  // Apagar el led vermell quan es torni a enviar dades
      }
   }

   // Apagar LED verd (intermitent)
   if (millis() - lastDataTime >= LED_DELAY) {
      digitalWrite(led1, LOW);
   }

   // Rebre comandes de Python a satèl·lit
   while (Serial.available()) {
      String cmd = Serial.readStringUntil('\n');
      mySerial.println(cmd);
   }
}