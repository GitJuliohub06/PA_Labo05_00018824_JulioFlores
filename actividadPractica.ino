#include <WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#define WLAN_SSID       "ARTEFACTOS"
#define WLAN_PASS       "87654321"
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "-" 
#define AIO_KEY         "-"         

#define TRIG_PIN 18
#define ECHO_PIN 19
#define LED_R 25
#define LED_G 26
#define LED_B 27

// ================= MQTT SETUP ===================
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// Configuración de los Feeds

Adafruit_MQTT_Publish feedDistancia = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/distancia");

// Suscriptor: Recibe las órdenes del Toggle
Adafruit_MQTT_Subscribe feedToggle = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/toggle");

// Variable global para controlar el Toggle
bool ledHabilitado = true;

// Prototipo de función para conectar/reconectar
void MQTT_connect();

void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);

  // Conectar a Wi-Fi
  Serial.println();
  Serial.print("Conectando a WiFi: ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado!");

  // Suscribirse al feed del botón
  mqtt.subscribe(&feedToggle);
}

void loop() {
  // Asegurar la conexión MQTT
  MQTT_connect();

  // 1. LEER SUSCRIPCIONES (Desafío del Toggle)
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(2000))) {
    if (subscription == &feedToggle) {
      Serial.print("Comando de Toggle recibido: ");
      Serial.println((char *)feedToggle.lastread);
      
      if (strcmp((char *)feedToggle.lastread, "ON") == 0) {
        ledHabilitado = true;
      } else {
        ledHabilitado = false;
        // Apagar LED inmediatamente
        analogWrite(LED_R, 0);
        analogWrite(LED_G, 0);
        analogWrite(LED_B, 0);
      }
    }
  }

  // 2. LECTURA DEL SENSOR ULTRASÓNICO
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duracion = pulseIn(ECHO_PIN, HIGH);
  
  if (duracion == 0) {
    Serial.println("No se detectó eco");
    delay(2000);
    return;
  }

  float distancia = (duracion * 0.0343) / 2;
  Serial.print("Distancia: "); Serial.print(distancia); Serial.println(" cm");

  // 3. LÓGICA DEL LED RGB (siempre y cuando el Toggle esté en ON)
  if (ledHabilitado) {
    if (distancia < 10) {
      // Cerca -> Rojo
      analogWrite(LED_R, 255);
      analogWrite(LED_G, 0);
      analogWrite(LED_B, 0);
    } else if (distancia >= 10 && distancia < 20) {
      // Medio -> Amarillo
      analogWrite(LED_R, 255);
      analogWrite(LED_G, 255);
      analogWrite(LED_B, 0);
    } else {
      // Lejos -> Verde
      analogWrite(LED_R, 0);
      analogWrite(LED_G, 255);
      analogWrite(LED_B, 0);
    }
  }

  // 4. PUBLICAR EN ADAFRUIT IO
  Serial.print("Enviando valor a Adafruit... ");
  if (! feedDistancia.publish(distancia)) {
    Serial.println("Fallo al enviar.");
  } else {
    Serial.println("OK!");
  }

  // Retardo de seguridad
  delay(3000); 
}

// Función para inicializar y mantener viva la conexión con Adafruit
void MQTT_connect() {
  int8_t ret;
  if (mqtt.connected()) {
    return;
  }
  Serial.print("Conectando a MQTT... ");
  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) {
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Reintentando en 5 segundos...");
       mqtt.disconnect();
       delay(5000);
       retries--;
       if (retries == 0) {
         while (1); // Congela el programa si definitivamente no hay conexión
       }
  }
  Serial.println("¡Conectado!");
}