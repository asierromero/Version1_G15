#include <DHT.h>
#include <SoftwareSerial.h>
#include <Servo.h>
#include <string.h>

/***
 PROTOCOLO DE COMUNICACIÓN SATÉLITE-TIERRA--------------------------------------
  
  FORMATO GENERAL: <datos>:<checksum>
 
  MENSAJES ENVIADOS POR SATÉLITE:
  1. DHT: "1:temperatura:2:humedad[:3:media]" (3 sale solo si la media se calcula en satélite)
  2. RADAR: "4:angulo:5:distancia"
  3. ÓRBITA: "ORBIT|tiempo|x|y|z" (con checksum)
  4. CONFIRMACIÓN/ERROR: "5:OK", "5:FalloDHT", "5:FalloRadar"
  
  MENSAJES RECIBIDOS POR SATÉLITE:
  1: Intervalo DHT (segundos)
  2: Ángulo fijo (0-180)
  3: Intervalo radar (ms)
  6: Modo cálculo media (1=Satélite, 0=Tierra)
  7: Modo rastreo (1=Rastreo, 0=Fijo)
  8: Límite temperatura (float)
  9: Control DHT (1=Reanudar, 0=Parar)
  10: Control radar (1=Reanudar, 0=Parar)
  11: Control órbita (1=Reanudar, 0=Parar)
  12: Intervalo órbita (ms)
  
  CHECKSUM: Suma de bytes módulo 256
 -----------------------------------------------------------------------------*/

// CONFIGURACIÓN INICIAL
#define DHTPIN 2
#define DHTTYPE DHT11    
DHT dht(DHTPIN, DHTTYPE); 

SoftwareSerial mySerial(10, 11); // RX, TX

// CONFIGURACIÓN RADAR
#define TRIGGER 4
#define ECHO 5
#define SERVO 6

Servo servoMotor; 
int angulo = 0;
int pasoBarrido = 2;
unsigned long lastRadarMillis = 0;
unsigned long radarInterval = 100;

// CONSTANTES ORBITA
const double G = 6.67430e-11;
const double M = 5.97219e24;
const double R_EARTH = 6371000;
const double ALTITUDE = 400000;
const double EARTH_ROTATION_RATE = 7.2921159e-5;
unsigned long MILLIS_BETWEEN_UPDATES = 1000; // Ahora variable
const double TIME_COMPRESSION = 90.0;

// VARIABLES ORBITA
unsigned long nextUpdate;
double real_orbital_period;
double r;

// VARIABLES PARA MODO RASTREO
bool modoRastreo = true;
int anguloFijo = 90;

// VARIABLES DE CONTROL
bool enviarDHT = true;        // Control DHT (temperatura/humedad)
bool enviarRadar = true;      // Control radar
bool enviarOrbita = true;     // Control órbita
unsigned long lastSendMillis = 0;
unsigned long sendInterval = 2000UL;

// Buffer de temperaturas (cola circular)
float tempBuffer[10];
int bufferIndex = 0;
bool bufferLleno = false;
float mediaActual = 0.0;

// Control de cálculo de media
bool calcularMediaEnSatelite = true;

// Control de timeout comunicación
unsigned long lastCommandTime = 0;
const unsigned long COMMAND_TIMEOUT = 10000; // 10 segundos

/**
 * @brief Calcula el checksum de un mensaje
 * @param mensaje String con los datos a verificar
 * @return byte Checksum calculado (suma de bytes módulo 256)
 */
byte calcularChecksum(String mensaje) {
  byte checksum = 0;
  for (unsigned int i = 0; i < mensaje.length(); i++) {
    checksum += mensaje[i];
  }
  return checksum;
}

/**
 * @brief Envía un mensaje con checksum
 * @param mensaje String con los datos a enviar
 */
void enviarConChecksum(String mensaje) {
  byte checksum = calcularChecksum(mensaje);
  String mensajeCompleto = mensaje + ":" + String(checksum);
  mySerial.println(mensajeCompleto);
}

/**
 * @brief Simula la posición orbital del satélite
 * @param millis_time Tiempo actual en milisegundos
 * @param inclination Inclinación orbital en radianes
 * @param ecef Si es 1, convierte a coordenadas ECEF
 */
void simulate_orbit(unsigned long millis_time, double inclination, int ecef) {
    static unsigned long lastOrbitSend = 0;
    static double random_offset_x = 0;
    static double random_offset_y = 0;
    static double random_offset_z = 0;
    
    // Solo actualizar offsets cada 10 segundos (más estable)
    static unsigned long lastOffsetChange = 0;
    if (millis_time - lastOffsetChange > 10000) {
        lastOffsetChange = millis_time;
        random_offset_x = random(-5000, 5000);
        random_offset_y = random(-5000, 5000);
        random_offset_z = random(-2000, 2000);
    }
    
    double time = (millis_time / 1000.0) * TIME_COMPRESSION;
    double angle = 2 * PI * (time / real_orbital_period);
    
    // Calcular posición base
    double x = r * cos(angle);
    double y = r * sin(angle) * cos(inclination);
    double z = r * sin(angle) * sin(inclination);
    
    // Agregar variación aleatoria (más suave)
    x += random_offset_x;
    y += random_offset_y;
    z += random_offset_z;
    
    if (ecef) {
        double theta = EARTH_ROTATION_RATE * time;
        double x_ecef = x * cos(theta) - y * sin(theta);
        double y_ecef = x * sin(theta) + y * cos(theta);
        x = x_ecef;
        y = y_ecef;
    }
    
    // ENVIAR solo si está activado y ha pasado el intervalo
    if (enviarOrbita && (millis_time - lastOrbitSend >= MILLIS_BETWEEN_UPDATES)) {
        lastOrbitSend = millis_time;
        String orbitMsg = "ORBIT|" + String(time) + "|" + 
                         String(x) + "|" + String(y) + "|" + String(z);
        enviarConChecksum(orbitMsg); // Ahora con checksum
    }
}

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
  
  // Inicializar semilla aleatoria
  randomSeed(analogRead(0));
  
  // Inicializar órbita
  nextUpdate = millis() + MILLIS_BETWEEN_UPDATES;
  r = R_EARTH + ALTITUDE;
  real_orbital_period = 2 * PI * sqrt(pow(r, 3) / (G * M));
  
  // Inicializar sensores
  dht.begin();
  
  // Inicializar servo
  servoMotor.attach(SERVO);
  delay(100);
  servoMotor.write(0);
  delay(500);
  
  // Configurar pines ultrasónico
  pinMode(TRIGGER, OUTPUT);
  pinMode(ECHO, INPUT);
  digitalWrite(TRIGGER, LOW);
  
  // Inicializar buffer circular
  for (int i = 0; i < 10; i++) {
    tempBuffer[i] = 0;
  }
  
  lastCommandTime = millis();
  Serial.println("Sistema listo - Servo inicializado");
  Serial.println("Modo: RASTREO activado");
}

/**
 * @brief Mide la distancia con el sensor ultrasónico
 * @return long Distancia en cm, o -1 si hay error
 */
long medirDistancia() {
  digitalWrite(TRIGGER, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER, LOW);
  
  long duracion = pulseIn(ECHO, HIGH, 25000);
  if (duracion == 0) return -1;
  
  long distancia = duracion * 0.034 / 2;
  
  if (distancia == 0 || distancia > 400) return -1;
  return distancia;
}

/**
 * @brief Actualiza la posición del servo (modo rastreo o fijo)
 */
void actualizarRadar() {
  unsigned long ahora = millis();
  
  // Actualizar posición del servo solo si radar está activo
  if ((ahora - lastRadarMillis >= radarInterval) && enviarRadar) {
    lastRadarMillis = ahora;
    
    if (modoRastreo) {
      // MODO RASTREO: barrido continuo
      servoMotor.write(angulo);
      
      // Actualizar ángulo para próximo movimiento
      angulo += pasoBarrido;
      if (angulo >= 180) {  // Limitar a 180° para mejor visualización
        angulo = 180;
        pasoBarrido = -2;
      } else if (angulo <= 0) {
        angulo = 0;
        pasoBarrido = 2;
      }
    } else {
      // MODO FIJO: mantener ángulo establecido
      servoMotor.write(anguloFijo);
      angulo = anguloFijo;
    }
  } else if (!enviarRadar) {
    // Si radar está pausado, mantener última posición
    servoMotor.write(angulo);
  }
}

/**
 * @brief Calcula la media de las últimas 10 temperaturas (cola circular)
 * @return float Valor medio de las temperaturas
 */
float calcularMedia() {
  int elementos = bufferLleno ? 10 : bufferIndex;
  if (elementos == 0) return 0;
  
  float suma = 0;
  for (int i = 0; i < elementos; i++) {
    suma += tempBuffer[i];
  }
  return suma / elementos;
}

void loop() {
  unsigned long currentTime = millis();
  
  // Verificar timeout de comunicación
  if (currentTime - lastCommandTime > COMMAND_TIMEOUT) {
    Serial.println("ALERTA: Timeout de comunicación con tierra");
    mySerial.println("5:Timeout");
    lastCommandTime = currentTime; // Resetear
  }
  
  // ÓRBITA - enviar solo si está activo
  if (enviarOrbita && currentTime > nextUpdate) {
    simulate_orbit(currentTime, 0, 0);
    nextUpdate = currentTime + MILLIS_BETWEEN_UPDATES;
  }
  
  // RADAR - actualizar posición independientemente del envío
  actualizarRadar();
  
  // RECEPCIÓN COMANDOS
  if (mySerial.available()) {
    String comando = mySerial.readStringUntil('\n');
    comando.trim();
    
    int separador = comando.indexOf(':');
    if (separador == -1) return;
    
    int codigo = comando.substring(0, separador).toInt();
    String valor = comando.substring(separador + 1);
    
    // Actualizar tiempo del último comando
    lastCommandTime = millis();
    
    switch (codigo) {
      case 1: // Cambiar intervalo DHT
        sendInterval = valor.toInt() * 1000UL;
        Serial.print("Intervalo DHT: ");
        Serial.println(sendInterval);
        mySerial.println("5:OK");
        break;
        
      case 2: // Establecer ángulo fijo
        anguloFijo = valor.toInt();
        if (anguloFijo < 0) anguloFijo = 0;
        if (anguloFijo > 180) anguloFijo = 180;
        modoRastreo = false;
        servoMotor.write(anguloFijo);
        angulo = anguloFijo;
        Serial.print("Ángulo fijo: ");
        Serial.println(anguloFijo);
        mySerial.println("5:OK");
        break;
        
      case 3: // Intervalo radar
        unsigned long nuevoIntervalo = valor.toInt();
        if (nuevoIntervalo >= 50 && nuevoIntervalo <= 5000) {
          radarInterval = nuevoIntervalo;
          Serial.print("Intervalo radar cambiado a: ");
          Serial.print(radarInterval);
          Serial.println("ms");
        }
        mySerial.println("5:OK");
        break;
        
      case 6: // Modo cálculo media (Satélite/Tierra)
        calcularMediaEnSatelite = (valor.toInt() == 1);
        Serial.print("Cálculo media: ");
        Serial.println(calcularMediaEnSatelite ? "SATÉLITE" : "TIERRA");
        mySerial.println("5:OK");
        break;
        
      case 7: // Modo rastreo (1=Rastreo, 0=Fijo)
        modoRastreo = (valor.toInt() == 1);
        Serial.print("Modo: ");
        Serial.println(modoRastreo ? "RASTREO" : "FIJO");
        mySerial.println("5:OK");
        break;
        
      case 8: // Límite temperatura
        Serial.print("Límite recibido: ");
        Serial.println(valor.toFloat());
        mySerial.println("5:OK");
        break;
        
      case 9: // Parar/Reanudar DHT (temperatura/humedad)
        enviarDHT = (valor.toInt() == 1); // 1=Reanudar, 0=Parar
        Serial.print("DHT: ");
        Serial.println(enviarDHT ? "REANUDADO" : "PAUSADO");
        mySerial.println("5:OK");
        break;
        
      case 10: // Parar/Reanudar radar
        enviarRadar = (valor.toInt() == 1); // 1=Reanudar, 0=Parar
        Serial.print("Radar: ");
        Serial.println(enviarRadar ? "REANUDADO" : "PAUSADO");
        mySerial.println("5:OK");
        break;
        
      case 11: // Parar/Reanudar órbita
        enviarOrbita = (valor.toInt() == 1); // 1=Reanudar, 0=Parar
        Serial.print("Órbita: ");
        Serial.println(enviarOrbita ? "REANUDADO" : "PAUSADO");
        mySerial.println("5:OK");
        break;
        
      case 12: // Cambiar intervalo órbita (NUEVO)
        unsigned long nuevoIntervaloOrbita = valor.toInt();
        if (nuevoIntervaloOrbita >= 100 && nuevoIntervaloOrbita <= 10000) {
          MILLIS_BETWEEN_UPDATES = nuevoIntervaloOrbita;
          Serial.print("Intervalo órbita cambiado a: ");
          Serial.print(MILLIS_BETWEEN_UPDATES);
          Serial.println("ms");
          mySerial.println("5:OK");
        } else {
          mySerial.println("5:ErrorIntervalo");
        }
        break;
        
      default:
        Serial.print("Comando desconocido: ");
        Serial.println(codigo);
        break;
    }
  }
  
  // ENVÍO DATOS DHT Y RADAR
  if (enviarDHT && (millis() - lastSendMillis >= sendInterval)) {
    lastSendMillis = millis();
    
    float temperatura = dht.readTemperature();
    float humedad = dht.readHumidity();
    
    // Detectar fallo DHT
    if (isnan(temperatura) || isnan(humedad)) {
      enviarConChecksum("5:FalloDHT");
      Serial.println("ERROR: Fallo sensor DHT");
      return;
    }
    
    // Almacenar en buffer circular
    tempBuffer[bufferIndex] = temperatura;
    bufferIndex = (bufferIndex + 1) % 10;
    if (!bufferLleno && bufferIndex == 0) bufferLleno = true;
    
    // Calcular media si es necesario
    mediaActual = calcularMedia();
    
    // Medir distancia actual
    long distancia = -1;
    if (enviarRadar) {
      distancia = medirDistancia();
      // Detectar fallo radar
      if (distancia == -1) {
        enviarConChecksum("5:FalloRadar");
        Serial.println("ERROR: Fallo sensor radar");
      }
    }
    
    // ENVIAR DATOS DHT
    String mensajeDHT;
    if (calcularMediaEnSatelite) {
      mensajeDHT = "1:" + String(temperatura, 1) + ":2:" + String(humedad, 1) + ":3:" + String(mediaActual, 1);
    } else {
      mensajeDHT = "1:" + String(temperatura, 1) + ":2:" + String(humedad, 1);
    }
    enviarConChecksum(mensajeDHT);
    
    // ENVIAR DATOS RADAR (solo si activo y distancia válida)
    if (enviarRadar && distancia > 0) {
      String mensajeRadar = "4:" + String(angulo) + ":5:" + String(distancia);
      enviarConChecksum(mensajeRadar);
      
      // Mostrar en monitor serie
      Serial.print("DHT: ");
      Serial.print(mensajeDHT);
      Serial.print(" | RADAR: Ángulo=");
      Serial.print(angulo);
      Serial.print("°, Dist=");
      Serial.print(distancia);
      Serial.println("cm");
    } else if (enviarDHT) {
      Serial.print("DHT: ");
      Serial.println(mensajeDHT);
    }
  }
}
  