// Configuración de pines
#define RELAY_PIN  26   // Pin conectado al módulo de relé

// Variables
bool relayState = false;  // Estado actual del relé

void setup() {
  // Configuración de pines
  pinMode(RELAY_PIN, OUTPUT);    // Configurar el pin del relé como salida

  // Inicializar el estado del relé
  digitalWrite(RELAY_PIN, LOW); // LOW desactiva el relé (depende del módulo)
  Serial.begin(115200);         // Iniciar comunicación serial para entrada de usuario
  Serial.println("Sistema iniciado. Escriba 'ON' para encender el relé o 'OFF' para apagarlo.");
}

void loop() {
  // Verificar si hay datos disponibles en la terminal serial
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n'); // Leer la entrada hasta un salto de línea
    input.trim(); // Eliminar espacios adicionales

    if (input.equalsIgnoreCase("ON")) {
      relayState = true;
      digitalWrite(RELAY_PIN, HIGH); // Activar el relé
      Serial.println("Relé encendido.");
    } else if (input.equalsIgnoreCase("OFF")) {
      relayState = false;
      digitalWrite(RELAY_PIN, LOW); // Desactivar el relé
      Serial.println("Relé apagado.");
    } else {
      Serial.println("Comando no reconocido. Use 'ON' o 'OFF'.");
    }
  }
}
