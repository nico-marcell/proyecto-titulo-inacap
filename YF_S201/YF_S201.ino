const int pinSensor = 2;   //Pin digital donde está conectado el sensor
volatile int cuentaPulsos;  //Variable para contar los pulsos
unsigned long tiempoAnterior;  //Tiempo de referencia para el cálculo del flujo
float caudal;               //Variable para almacenar el caudal en L/min
bool medirCaudal = false;   //Variable para habilitar/iniciar o detener la medición

// Constante de calibración (varía según el sensor. En este caso, 7.5 para el modelo YF-S201)
const float factorCalibracion = 7.5;

// Interrupción que se activa en cada pulso del sensor
void contarPulsos() {
  if (medirCaudal) {  // Solo cuenta pulsos si la medición está activa
    cuentaPulsos++;
  }
}

void setup() {
  // Configuración del pin y de la interrupción
  pinMode(pinSensor, INPUT);
  attachInterrupt(digitalPinToInterrupt(pinSensor), contarPulsos, RISING);

  // Inicio
  Serial.begin(9600);
  tiempoAnterior = millis();

  // Mensaje inicial en la consola
  Serial.println("Ingrese 'I' para iniciar la medición y 'S' para detenerla.");
}

void loop() {
  // Revisar si hay datos disponibles en la consola serial
  // En esta sección se revisan si existen datos en la consola serial 
  if (Serial.available() > 0) {
    char comando = Serial.read();  // Aquí leemos para saber si se inicia el proceso o se detiene

    if (comando == 'I' || comando == 'i') {
      medirCaudal = true;  // Iniciar
      Serial.println("Medición iniciada.");
    } else if (comando == 'S' || comando == 's') {
      medirCaudal = false;  // Detener
      Serial.println("Medición detenida.");
    }
  }

  // Calcular y mostrar el caudal cada dos segundos [(2000) medidos en mili] si la medición está activa
  if (medirCaudal && (millis() - tiempoAnterior >= 2000)) {
    // Desactivar interrupción temporalmente para poder  leer cuenta de pulsos
    detachInterrupt(digitalPinToInterrupt(pinSensor));

    //La siguiente línea es para calcular el caudal en litros por minuto para poder enseñarlo en el monitor
    //
    caudal = (cuentaPulsos / factorCalibracion);

    //En esta parte se muestra en el monitor serial para que comprobemos la lectura de datos
    Serial.print("Caudal: ");
    Serial.print(caudal);
    Serial.println(" L/min");

    //Reiniciar la cuenta de pulsos y tiempo
    cuentaPulsos = 0;
    tiempoAnterior = millis();

    // Reactivar la interrupción
    attachInterrupt(digitalPinToInterrupt(pinSensor), contarPulsos, RISING);
  }
}
