#include <WiFi.h>
#include <UbidotsEsp32Mqtt.h>

// Configuración WiFi
const char* SSID = "Proyecto"; // Reemplaza con el nombre de tu red WiFi
const char* PASSWORD = "12345678"; // Reemplaza con la contraseña de tu red WiFi

// Configuración Ubidots
const char* UBIDOTS_TOKEN = "BBUS-RAtHlhOypvwyAUyyWlVZkbukac46tE"; // Reemplaza con tu token de Ubidots
const char* DEVICE_LABEL = "esp32_agua"; // Etiqueta para tu dispositivo
const char* VARIABLE_FLOW_RATE = "caudal"; // Etiqueta para el caudal
const char* VARIABLE_TOTAL_LITERS = "litros_totales"; // Etiqueta para los litros totales

// Pines
const int sensorPin = 2; // Pin digital al que está conectado el cable de datos del sensor

// Variables
volatile int pulseCount = 0; // Conteo de pulsos
unsigned long previousTime = 0; // Tiempo previo para el cálculo del caudal
unsigned long lastUbidotsTime = 0; // Tiempo de último envío a Ubidots
float flowRate = 0.0; // Caudal en litros por minuto
float totalLiters = 0.0; // Litros totales acumulados
bool isRunning = true; // Estado del sensado y envío de datos

// Constantes
const float calibrationFactor = 4.5; // Factor de calibración del sensor (ver datasheet del YF-S201)
// **Para ajustar la precisión del sensor, modifica el valor de calibrationFactor.**

const unsigned long UBIDOTS_INTERVAL = 60000; // Intervalo para enviar datos a Ubidots (en milisegundos)
// **Para cambiar el intervalo de envío de datos a Ubidots, modifica UBIDOTS_INTERVAL.**

// Inicializa Ubidots MQTT
Ubidots ubidots(UBIDOTS_TOKEN);

void callback(char* topic, byte* payload, unsigned int length) {
  // Función de callback (opcional para recibir datos)
}

void setup() {
  // Configuración de pines y serial
  pinMode(sensorPin, INPUT_PULLUP); // Configura el pin del sensor
  attachInterrupt(digitalPinToInterrupt(sensorPin), pulseCounter, RISING); // Configura interrupción
  Serial.begin(115200); // Inicializa el monitor serial
  Serial.println("Iniciando sensor de caudal...");

  // Configuración WiFi
  WiFi.begin(SSID, PASSWORD);
  Serial.println("Conectando a WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nConexión WiFi establecida.");
  Serial.println("IP asignada: " + WiFi.localIP().toString());

  // Configuración Ubidots
  ubidots.setCallback(callback);
  ubidots.setup();
  ubidots.reconnect();
}

void loop() {
  // Lee comandos desde el monitor serial
  if (Serial.available()) {
    char command = Serial.read();
    if (command == 'S' || command == 's') {
      isRunning = false;
      Serial.println("Sensado y envío de datos detenido.");
    } else if (command == 'I' || command == 'i') {
      isRunning = true;
      Serial.println("Sensado y envío de datos iniciado.");
    }
  }

  if (!isRunning) return;

  // Revisa la conexión a Ubidots
  if (!ubidots.connected()) {
    ubidots.reconnect();
  }

  // Calcula el caudal y los litros acumulados cada segundo
  unsigned long currentTime = millis();
  unsigned long elapsedTime = currentTime - previousTime;

  if (elapsedTime >= 1000) {
    noInterrupts(); // Deshabilita interrupciones temporalmente
    float pulseFrequency = pulseCount; // Guarda el conteo de pulsos
    pulseCount = 0; // Reinicia el conteo
    interrupts(); // Habilita las interrupciones de nuevo

    // Calcula el caudal en L/min
    flowRate = (pulseFrequency / calibrationFactor);

    // Calcula los litros totales
    float liters = (flowRate / 60.0); // Conversión de L/min a litros por segundo
    totalLiters += liters;

    // Muestra los datos en el monitor serial
    Serial.print("Caudal: ");
    Serial.print(flowRate);
    Serial.print(" L/min\t");
    Serial.print("Litros Totales: ");
    Serial.print(totalLiters);
    Serial.println(" L");

    previousTime = currentTime; // Actualiza el tiempo previo
  }

  // Enviar datos a Ubidots cada minuto
  if (currentTime - lastUbidotsTime >= UBIDOTS_INTERVAL) {
    ubidots.add(VARIABLE_FLOW_RATE, flowRate);
    ubidots.add(VARIABLE_TOTAL_LITERS, totalLiters);
    ubidots.publish(DEVICE_LABEL);

    Serial.println("Datos enviados a Ubidots:");
    Serial.print(" - Caudal: ");
    Serial.print(flowRate);
    Serial.println(" L/min");
    Serial.print(" - Litros Totales: ");
    Serial.print(totalLiters);
    Serial.println(" L");

    lastUbidotsTime = currentTime; // Actualiza el tiempo del último envío
  }
}

// Función de interrupción para contar los pulsos
void pulseCounter() {
  pulseCount++;
}
