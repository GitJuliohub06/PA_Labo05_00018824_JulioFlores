#define TRIG_PIN 18
#define ECHO_PIN 19

void setup() {
  Serial.begin(115200);

  // Completando las configuraciones de pines
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

void loop() {
  // Generar el pulso de disparo
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  // Leer el tiempo de retorno del eco
  long duracion = pulseIn(ECHO_PIN, HIGH);

  if (duracion == 0) {
    Serial.println("No se detectó eco");
  } else {
    // Fórmula para calcular la distancia en cm: d = (0.0343 * t) / 2
    float distancia = (duracion * 0.0343) / 2;

    Serial.print("Distancia: ");
    Serial.print(distancia);
    Serial.println(" cm");
  }

  delay(500);
}