const int Trigger = 4;   //Pin digital 2 para el Trigger del sensor
const int Echo = 5;   //Pin digital 3 para el Echo del sensor
#include <Servo.h>  // incluimos la librería Servo.h

Servo servoMotor;   // creamos el objeto Servo
int pos = 0;        // variable para guardar el valor de la posición del servo

void setup() {
  Serial.begin(9600);//iniciailzamos la comunicación
  pinMode(Trigger, OUTPUT); //pin como salida
  pinMode(Echo, INPUT);  //pin como entrada
  digitalWrite(Trigger, LOW);//Inicializamos el pin con 0

  Serial.begin(9600);
  servoMotor.attach(9);  // definimos el Servo en el pin 9
}


void loop() {
  long t; // tiempo del eco
  long d; // distancia en cm

  // Recorrido de 0° a 180°
  for (pos = 0; pos <= 180; pos += 1) {
    digitalWrite(Trigger, HIGH);
    delayMicroseconds(10);
    digitalWrite(Trigger, LOW);
    t = pulseIn(Echo, HIGH);
    d = t / 59;

    Serial.print(pos);
    Serial.print(":");
    Serial.println(d); 

    servoMotor.write(pos);
    delay(50);
  }

  // Regreso de 180° a 0°
  for (pos = 180; pos >= 0; pos -= 1) {
    digitalWrite(Trigger, HIGH);
    delayMicroseconds(10);
    digitalWrite(Trigger, LOW);
    t = pulseIn(Echo, HIGH);
    d = t / 59;

    Serial.print(pos);
    Serial.print(":");
    Serial.println(d);

    servoMotor.write(pos);
    delay(50);
  }
}

/*
void loop()
{

  long t; //timepo que demora en llegar el eco
  long d; //distancia en centimetros

  digitalWrite(Trigger, HIGH);
  delayMicroseconds(10);          //Enviamos un pulso de 10us
  digitalWrite(Trigger, LOW);
 
  t = pulseIn(Echo, HIGH); //obtenemos el ancho del pulso
  d = t/59;             //escalamos el tiempo a una distancia en cm
 
  Serial.print("Distancia: ");
  Serial.print(d);      //Enviamos serialmente el valor de la distancia
  Serial.print("cm");
  Serial.println();
  delay(1000);          //Hacemos una pausa de 1s


  //SERVO
  for (pos = 0; pos <= 180; pos += 1) {  // cambia la posición de 0 a 180 grado por grado
    Serial.println(pos);
    servoMotor.write(pos);
    delay(10);
  }

  for (pos = 180; pos >= 0; pos -= 1) {  // cambia la posición de 180 a 0 grado por grado
    Serial.println(pos);
    servoMotor.write(pos);
    delay(10);
  }
}
*/