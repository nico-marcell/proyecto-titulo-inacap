// Pines
const int sensorPin = 2; // Pin digital al que está conectado el cable de datos del sensor

// Variables
volatile int pulseCount = 0; // Conteo de pulsos
unsigned long previousTime = 0; // Tiempo previo para el cálculo del caudal
float flowRate = 0.0; // Caudal en litros por minuto
float totalLiters = 0.0; // Litros totales acumulados

// Constantes
const float calibrationFactor = 4.5; // Factor de calibración del sensor (ver datasheet del YF-S201)

void setup() {
  pinMode(sensorPin, INPUT_PULLUP); // Configura el pin del sensor
  attachInterrupt(digitalPinToInterrupt(sensorPin), pulseCounter, RISING); // Configura interrupción
  Serial.begin(9600); // Inicializa el monitor serial
  Serial.println("Iniciando sensor de caudal...");
}

void loop() {
  unsigned long currentTime = millis();
  unsigned long elapsedTime = currentTime - previousTime;

  // Calcula el caudal cada segundo
  if (elapsedTime >= 1000) {
    noInterrupts(); // Deshabilita las interrupciones temporalmente
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
}

// Función de interrupción para contar los pulsos
void pulseCounter() {
  pulseCount++;
}