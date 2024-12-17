#include <WiFi.h>
#include <UbidotsEsp32Mqtt.h>

// -------- Configuración WiFi -------- //
const char* SSID = "Proyecto";         // Reemplaza con tu red WiFi
const char* PASSWORD = "12345678";    // Reemplaza con la contraseña de tu WiFi

// -------- Configuración Ubidots -------- //
const char* UBIDOTS_TOKEN = "BBUS-2ySaT4wPIzgIiZV514Wh3uHnwzpd6M"; // Token de Ubidots
const char* DEVICE_LABEL = "esp32_agua";                           // Etiqueta del dispositivo
const char* VARIABLE_FLOW_RATE = "caudal";                         // Etiqueta del caudal
const char* VARIABLE_TOTAL_LITERS = "litros_totales";              // Etiqueta de litros totales
const char* VARIABLE_GAS = "deteccion_gas";                        // Etiqueta para la detección de gas

// -------- Pines -------- //
const int sensorCaudalPin = 2;   // Pin del caudalímetro
const int mq2Pin = 34;           // Pin analógico del MQ2

// -------- Configuración del MQ2 -------- //
const int thresholdGas = 1350;   // Umbral para detectar gas

// -------- Variables del Caudalímetro -------- //
volatile int pulseCount = 0;
float flowRate = 0.0;
float totalLiters = 0.0;
unsigned long previousTime = 0;
const float calibrationFactor = 4.5;

// -------- Variables de Ubidots -------- //
unsigned long lastUbidotsTime = 0;
const unsigned long UBIDOTS_INTERVAL = 60000; // Intervalo de envío a Ubidots (1 minuto)

// -------- Inicializa Ubidots -------- //
Ubidots ubidots(UBIDOTS_TOKEN);

void callback(char* topic, byte* payload, unsigned int length) {
  // Función callback opcional (no utilizada aquí)
}

void setup() {
  // Configuración de pines
  pinMode(sensorCaudalPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(sensorCaudalPin), pulseCounter, RISING);

  // Configuración de comunicación serial
  Serial.begin(115200);
  Serial.println("Iniciando sistema...");

  // Conexión WiFi
  WiFi.begin(SSID, PASSWORD);
  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nConexión WiFi establecida. IP: " + WiFi.localIP().toString());

  // Configuración de Ubidots
  ubidots.setCallback(callback);
  ubidots.setup();
  ubidots.reconnect();
}


void loop() {
  unsigned long currentTime = millis();

  // Leer valor del sensor MQ2
  int mq2Value = analogRead(mq2Pin);
  bool gasDetected = mq2Value > thresholdGas;

  if (gasDetected) {
    Serial.println("ALERTA: Fuga de gas detectada");
  } else {
    Serial.println("Todo normal");
  }

  // Calcular caudal y litros totales
  if (currentTime - previousTime >= 1000) { // Cada segundo
    noInterrupts();
    float pulseFrequency = pulseCount;
    pulseCount = 0;
    interrupts();

    flowRate = pulseFrequency / calibrationFactor;  // Caudal en L/min
    totalLiters += flowRate / 60.0;                 // Litros acumulados

    // Mostrar en el monitor serial
    Serial.print("Caudal: ");
    Serial.print(flowRate);
    Serial.print(" L/min | Litros Totales: ");
    Serial.println(totalLiters);

    previousTime = currentTime;
  }

  // Enviar datos a Ubidots
  if (currentTime - lastUbidotsTime >= UBIDOTS_INTERVAL) {
    if (!ubidots.connected()) {
      Serial.println("Reconectando a Ubidots...");
      ubidots.reconnect();
    }

    // Agregar datos a Ubidots
    ubidots.add(VARIABLE_FLOW_RATE, flowRate);
    ubidots.add(VARIABLE_TOTAL_LITERS, totalLiters);
    ubidots.add(VARIABLE_GAS, gasDetected ? 1.0 : 0.0);  // Indicador de detección de gas

    // Publicar datos
    if (ubidots.publish(DEVICE_LABEL)) {
      Serial.println("Datos enviados a Ubidots exitosamente.");
    } else {
      Serial.println("Error al enviar datos a Ubidots.");
    }

    lastUbidotsTime = currentTime;
  }

  delay(500);
}

//ANTIGUO
void pulseCounter() {
  pulseCount++;
}
