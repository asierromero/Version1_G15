/*
#include <DHT.h>
#include <Servo.h>

#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Pines sensor ultrasonido
const int Trigger = 4;
const int Echo = 5;

// Comunicación
#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11); // RX, TX

// Servo
Servo servoMotor;
int posServo = 0;

// LEDs
const int led1 = 13; // LED verde - envío
const int led2 = 12; // LED rojo - error

// Variables control
bool enviarDatosTH = true;
bool enviarDatosDist = true;
bool modoRastreo = true;
unsigned long lastSendTH = 0;
unsigned long lastSendDist = 0;
unsigned int periodoTH = 2000; // 2 segundos
unsigned int periodoDist = 100; // 0.1 segundos

// Variables para cálculo de medias (si se hace en satélite)
float tempBuffer[10];
int bufferIndex = 0;
bool bufferLleno = false;
bool calcularMediasEnSatelite = false;

// Declaraciones de funciones
void procesarComandos();
void enviarDatosTH();
void enviarDatosDistancia();
void controlarRastreo();
float calcularMediaTemperatura();

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
  dht.begin();
  
  // Configurar pines
  pinMode(Trigger, OUTPUT);
  pinMode(Echo, INPUT);
  digitalWrite(Trigger, LOW);
  
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
  
  // Inicializar servo
  servoMotor.attach(9);
  servoMotor.write(90); // Posición central
  
  // Inicializar buffer temperaturas
  for(int i=0; i<10; i++) tempBuffer[i] = 0;
}

void loop() {
  unsigned long now = millis();
  
  // Procesar comandos desde tierra
  
  while (mySerial.available()) {
    String comando = mySerial.readStringUntil('\n');
    comando.trim();
    
    int fin = comando.indexOf(':', 0);
    if(fin == -1) continue;
    
    int codigo = comando.substring(0, fin).toInt();
    int inicio = fin + 1;
    
    switch(codigo) {
      case 1: // Cambiar periodo TH
        fin = comando.indexOf(':', inicio);
        periodoTH = comando.substring(inicio, fin).toInt();
        break;
        
      case 2: // Cambiar orientación servo
        fin = comando.indexOf(':', inicio);
        posServo = comando.substring(inicio, fin).toInt();
        servoMotor.write(posServo);
        modoRastreo = false;
        break;
        
      case 3: // Parar envío TH
        enviarDatosTH = false;
        break;
        
      case 4: // Reanudar envío TH
        enviarDatosTH = true;
        break;
        
      case 5: // Parar envío distancia
        enviarDatosDist = false;
        break;
        
      case 6: // Reanudar envío distancia
        enviarDatosDist = true;
        break;
        
      case 7: // Activar modo rastreo
        modoRastreo = true;
        break;
        
      case 8: // Cambiar donde se calculan medias
        calcularMediasEnSatelite = (comando.substring(inicio).toInt() == 1);
        break;
        
      case 9: // Cambiar periodo distancia
        fin = comando.indexOf(':', inicio);
        periodoDist = comando.substring(inicio, fin).toInt();
        break;
    }
  }
}

  
  // Enviar datos de temperatura/humedad
  if(enviarDatosTH && (now - lastSendTH >= periodoTH)) {
    lastSendTH = now;
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    
    if (!isnan(t) && !isnan(h)) {
      // Actualizar buffer para media
      tempBuffer[bufferIndex] = t;
      bufferIndex = (bufferIndex + 1) % 10;
      if(bufferIndex == 0) bufferLleno = true;
      
      // Calcular media si corresponde
      float media = 0;
      if(calcularMediasEnSatelite) {
        media = calcularMediaTemperatura();
      }
      
      // Enviar datos
      if(calcularMediasEnSatelite) {
        mySerial.print("1:");
        mySerial.print(t);
        mySerial.print(":");
        mySerial.print(h);
        mySerial.print(":");
        mySerial.println(media);
      } else {
        mySerial.print("1:");
        mySerial.print(t);
        mySerial.print(":");
        mySerial.println(h);
      }
      
      digitalWrite(led1, HIGH);
      delay(50);
      digitalWrite(led1, LOW);
      
    } else {
      // Error en lectura
      mySerial.println("3:");
      digitalWrite(led2, HIGH);
      delay(1000);
      digitalWrite(led2, LOW);
    }
  }
    }
    
    // Enviar datos de distancia
    if(enviarDatosDist && (now - lastSendDist >= periodoDist)) {
      lastSendDist = now;
      enviarDatosDistancia();
    }
    
    // Controlar modo rastreo
    if(modoRastreo) {
      controlarRastreo();
    }
  }


void enviarDatosTH() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  if (!isnan(t) && !isnan(h)) {
    // Actualizar buffer para media
    tempBuffer[bufferIndex] = t;
    bufferIndex = (bufferIndex + 1) % 10;
    if(bufferIndex == 0) bufferLleno = true;
    
    // Calcular media si corresponde
    float media = 0;
    if(calcularMediasEnSatelite) {
      media = calcularMediaTemperatura();
    }
    
    // Enviar datos
    if(calcularMediasEnSatelite) {
      mySerial.print("1:");
      mySerial.print(t);
      mySerial.print(":");
      mySerial.print(h);
      mySerial.print(":");
      mySerial.println(media);
    } else {
      mySerial.print("1:");
      mySerial.print(t);
      mySerial.print(":");
      mySerial.println(h);
    }
    
    digitalWrite(led1, HIGH);
    delay(50);
    digitalWrite(led1, LOW);
    
  } else {
    // Error en lectura
    mySerial.println("3:");
    digitalWrite(led2, HIGH);
    delay(1000);
    digitalWrite(led2, LOW);
  }
}

float calcularMediaTemperatura() {
  float suma = 0;
  int elementos = bufferLleno ? 10 : bufferIndex;
  
  if(elementos == 0) return 0;
  
  for(int i=0; i<elementos; i++) {
    suma += tempBuffer[i];
  }
  return suma / elementos;
}

void enviarDatosDistancia() {
  long t; // tiempo del eco
  long d; // distancia en cm

  digitalWrite(Trigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(Trigger, LOW);
  t = pulseIn(Echo, HIGH);
  d = t / 59;

  // Enviar ángulo y distancia
  mySerial.print("2:");
  mySerial.print(posServo);
  mySerial.print(":");
  mySerial.println(d);
}

void controlarRastreo() {
  static unsigned long lastMove = 0;
  static int direccion = 1;
  
  if(millis() - lastMove >= 50) { // Mover cada 50ms
    lastMove = millis();
    
    posServo += direccion;
    if(posServo >= 180 || posServo <= 0) {
      direccion = -direccion;
    }
    
    servoMotor.write(posServo);
  }
}
*/

#include <DHT.h>
#include <SoftwareSerial.h>
#include <Servo.h>

// ---------------- CONFIGURACIÓ ----------------
#define DHTPIN 2
#define DHTTYPE DHT11
#define TRIGGER 4
#define ECHO 5

DHT dht(DHTPIN, DHTTYPE);
Servo servoMotor;

SoftwareSerial mySerial(10, 11); // RX, TX amb la Terra

// ---------------- VARIABLES DE CONTROL ----------------
bool enviarDatos = true;
unsigned long lastSendMillis = 0;
unsigned long sendInterval = 2000UL; // valor per defecte: 2 s
int servoPos = 0; // orientació actual del servo
float tempBuffer[10];
int bufferIndex = 0;
bool bufferLleno = false;

// ---------------- SETUP ----------------
void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
  dht.begin();
  servoMotor.attach(90);

  pinMode(TRIGGER, OUTPUT);
  pinMode(ECHO, INPUT);

  for (int i = 0; i < 10; i++) tempBuffer[i] = 0;
}

// ---------------- FUNCIONS AUXILIARS ----------------
float medirDistancia() {
  digitalWrite(TRIGGER, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER, LOW);
  long t = pulseIn(ECHO, HIGH, 20000); // timeout 20ms
  if (t == 0) return -1; // sense eco
  return t / 59.0; // cm
}

float calcularMedia() {
  int n = bufferLleno ? 10 : bufferIndex;
  if (n == 0) return 0;
  float s = 0;
  for (int i = 0; i < n; i++) s += tempBuffer[i];
  return s / n;
}

// ---------------- LOOP PRINCIPAL ----------------
void loop() {
  // === Rebre missatges des de la Terra ===
  if (mySerial.available()) {
    String comando = mySerial.readStringUntil('\n');
    comando.trim();

    int fin = comando.indexOf(':');
    if (fin == -1) return; // missatge mal format

    int codigo = comando.substring(0, fin).toInt();
    int inicio = fin + 1;

    if (codigo == 1) {
      // canvi del període d’enviament
      sendInterval = comando.substring(inicio).toInt() * 1000UL;
      Serial.print("Nou període: "); Serial.println(sendInterval);
      mySerial.println("5:OK");

    } else if (codigo == 2) {
      // canvi d’orientació del servo
      servoPos = comando.substring(inicio).toInt();
      servoMotor.write(servoPos);
      Serial.print("Servo a: "); Serial.println(servoPos);
      mySerial.println("5:OK");

    } else if (codigo == 3) {
      // parar enviaments
      enviarDatos = false;
      Serial.println("PARADO.");
      mySerial.println("5:OK");

    } else if (codigo == 4) {
      // reprendre enviaments
      enviarDatos = true;
      Serial.println("REANUDADO.");
      mySerial.println("5:OK");
    }
  }

  // === Enviar dades periòdicament ===
  unsigned long now = millis();
  if (enviarDatos && (now - lastSendMillis >= sendInterval)) {
    lastSendMillis = now;

    float t = dht.readTemperature();
    float h = dht.readHumidity();
    float d = medirDistancia();

    if (isnan(t) || isnan(h)) {
      // Error en DHT
      mySerial.println("4:"); // codi 3 = error sensors
      Serial.println("Error de lectura DHT11!");
      return;
    }

    // Desa temperatura en buffer
    tempBuffer[bufferIndex] = t;
    bufferIndex = (bufferIndex + 1) % 10;
    if (!bufferLleno && bufferIndex == 0) bufferLleno = true;
    float mediaT = calcularMedia();

    // Enviar missatges segons protocol
    mySerial.print("1)T:");
    mySerial.print(t, 1); mySerial.print(":H:"); mySerial.println(h, 1); // temperatura, humitat

    mySerial.print("2)D:");
    mySerial.println(d, 1); // distància

    mySerial.print("3)M:");
    mySerial.println(mediaT, 1); // mitjana temperatura

    // També mostrar per monitor
    Serial.print("T="); Serial.print(t);
    Serial.print(" H="); Serial.print(h);
    Serial.print(" D="); Serial.print(d);
    Serial.print(" M="); Serial.println(mediaT);
  }
}
