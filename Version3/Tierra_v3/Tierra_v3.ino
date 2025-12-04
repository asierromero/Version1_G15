#include <SoftwareSerial.h>

// Configurar comunicación serial software (RX, TX)
SoftwareSerial mySerial(10, 11); // Pin 10 como RX, Pin 11 como TX

// ==================== CONFIGURACIÓN DE PINES Y HARDWARE ====================
const int BUZZER = 3;    // Pin del buzzer para alarmas sonoras
const int led1 = 13;     // LED verde - indica recepción de datos
const int led2 = 12;     // LED rojo - indica errores o fallos

// ==================== VARIABLES DE CONTROL DEL SISTEMA ====================
unsigned long lastDataTime = 0;        // Último tiempo de recepción de datos
const unsigned long LED_DELAY = 100;   // Tiempo LED encendido (ms)

// ==================== FUNCIÓN PARA VERIFICAR CHECKSUM ====================
bool verificarChecksum(String mensaje) {
    int ultimoSeparador = mensaje.lastIndexOf(':');
    if (ultimoSeparador == -1) return false;
    
    String datos = mensaje.substring(0, ultimoSeparador);
    String checksumStr = mensaje.substring(ultimoSeparador + 1);
    
    // Verificar que el checksum sea numérico
    for (unsigned int i = 0; i < checksumStr.length(); i++) {
        if (!isdigit(checksumStr[i])) return false;
    }
    
    int checksumRecibido = checksumStr.toInt();
    
    // Calcular checksum de los datos
    byte checksumCalculado = 0;
    for (unsigned int i = 0; i < datos.length(); i++) {
        checksumCalculado += datos[i];
    }
    
    return (checksumCalculado == checksumRecibido);
}

// ==================== CONFIGURACIÓN INICIAL (SETUP) ====================
void setup() {
   // Inicializar comunicación serial
   Serial.begin(9600);
   mySerial.begin(9600);
   
   // Configurar pines de salida
   pinMode(BUZZER, OUTPUT);
   pinMode(led1, OUTPUT);
   pinMode(led2, OUTPUT);
   
   // Estado inicial de los LEDs
   digitalWrite(led1, LOW);
   digitalWrite(led2, LOW);
   noTone(BUZZER);
   
   // Test inicial de LEDs y buzzer
   testHardware();
}

void testHardware() {
   // Test LED verde
   digitalWrite(led1, HIGH);
   delay(300);
   digitalWrite(led1, LOW);
   delay(100);
   
   // Test LED rojo
   digitalWrite(led2, HIGH);
   delay(300);
   digitalWrite(led2, LOW);
   delay(100);
   
   // Test buzzer
   tone(BUZZER, 1000, 200);
   delay(500);
}

// ==================== PROGRAMA PRINCIPAL (LOOP) ====================
void loop() {
   // ========== RECEPCIÓN DE DATOS DEL SATÉLITE ==========
   if (mySerial.available()) {
      String data = mySerial.readStringUntil('\n');
      data.trim();
      
      
      // ========== DETECTAR TIPO DE MENSAJE ==========
      // Mensajes ORBIT - sin checksum
      if (data.startsWith("ORBIT|")) {
         Serial.println(data);
         digitalWrite(led1, HIGH); delay(20); digitalWrite(led1, LOW);
         return;
      }
      
      // Mensajes DHT/radar - puede tener o no checksum
      if (data.startsWith("1:") || data.startsWith("4:")) {
         // Intentar verificar checksum
         if (verificarChecksum(data)) {
            // Tiene checksum válido
            Serial.println(data);
         } else {
            // No tiene checksum o es incorrecto
            // Asumir que es un mensaje válido sin checksum
            Serial.println(data);
            Serial.println("ADVERTENCIA: Mensaje sin checksum válido");
         }
         
         digitalWrite(led1, HIGH);
         lastDataTime = millis();
         delay(20);
         digitalWrite(led1, LOW);
         return;
      }
      
      // ========== MENSAJES DE CONFIRMACIÓN/ERROR ==========
      if (!verificarChecksum(data)) {
         Serial.println("ERROR: Checksum incorrecto - " + data);
         digitalWrite(led2, HIGH);
         tone(BUZZER, 800, 500);
         delay(100);
         return;
      }

   // ========== APAGAR LED VERDE DESPUÉS DEL DELAY ==========
   // Solo si no hay error activo
   if (digitalRead(led2) == LOW && millis() - lastDataTime >= LED_DELAY) {
      digitalWrite(led1, LOW);
   }

   // ========== APAGAR BUZZER SI HAY ERROR PERO YA PASÓ UN TIEMPO ==========
   if (digitalRead(led2) == HIGH && millis() - lastDataTime > 5000) {
      digitalWrite(led2, LOW);
      noTone(BUZZER);
   }

   // ========== REENVÍO DE COMANDOS DE PYTHON AL SATÉLITE ==========
   if (Serial.available()) {
      String cmd = Serial.readStringUntil('\n');
      cmd.trim();
      mySerial.println(cmd);
      Serial.println("Comando enviado: " + cmd);
   }
}
}