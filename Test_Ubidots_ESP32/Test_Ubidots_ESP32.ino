#include <WiFi.h>
#include <UbidotsEsp32Mqtt.h>

// Configuración WiFi
const char* SSID = "Netis"; // Reemplaza con el nombre de tu red WiFi
const char* PASSWORD = "password"; // Reemplaza con la contraseña de tu red WiFi

// Configuración Ubidots
const char* UBIDOTS_TOKEN = "BBUS-6xLDpuKUJKwMhOqKCethu2PrLtnLhC"; // Reemplaza con tu token de Ubidots
const char* DEVICE_LABEL = "esp32"; // Etiqueta para tu dispositivo
const char* VARIABLE_LABEL = "temperatura"; // Etiqueta para la variable

// Pines
const int LM35_PIN = 34; // Pin analógico conectado al LM35

// Inicializa Ubidots MQTT
Ubidots ubidots(UBIDOTS_TOKEN);

void callback(char* topic, byte* payload, unsigned int length) {
  // Función de callback (opcional para recibir datos)
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(SSID, PASSWORD);
  Serial.println("Conectando a WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nConexión WiFi establecida.");
  Serial.println("IP asignada: " + WiFi.localIP().toString());

  ubidots.setCallback(callback);
  ubidots.setup();
  ubidots.reconnect();
}

void loop() {
  if (!ubidots.connected()) {
    ubidots.reconnect();
  }

  // Lee la salida del LM35
  int sensorValue = analogRead(LM35_PIN);

  // Convierte la lectura a temperatura en grados Celsius
  float voltage = sensorValue * (3.3 / 4095.0); // El ESP32 usa 3.3V y 12 bits (4095)
  float temperature = voltage * 100.0;

  // Publica la temperatura en Ubidots
  ubidots.add(VARIABLE_LABEL, temperature);
  ubidots.publish(DEVICE_LABEL);

  Serial.print("Temperatura enviada: ");
  Serial.println(temperature);

  delay(10000); // Enviar datos cada 10 segundos
}