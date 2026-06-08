unsigned long tiempoAnterior = 0;
const unsigned long intervalo = 1000;

void setup() {
  Serial.begin(9600);
}

void loop() {
  unsigned long tiempoActual = millis();

  if (tiempoActual - tiempoAnterior >= intervalo) {
    tiempoAnterior = tiempoActual;
    Serial.println("Hola desde Arduino 1");
  }
}