struct Canal {
  int pin;
  int estadoActual;
  int estadoAnterior;
  unsigned long tiempoAnterior;
  unsigned long periodo;
  float frecuencia;
  unsigned long tiempoCambio;
  unsigned long tHigh;
  unsigned long tLow;
  float duty;
};

void medicionCanal(Canal &canal, unsigned long tiempoActual);
Canal canal1 = {2, LOW, LOW, 0, 0, 0.0, 0, 0, 0, 0.0};
Canal canal2 = {3, LOW, LOW, 0, 0, 0.0, 0, 0, 0, 0.0};
unsigned long tiempoImpresion = 0;
const unsigned long intervalo = 1000; // ms

void setup() {
  pinMode(canal1.pin, INPUT);
  pinMode(canal2.pin, INPUT);
  Serial.begin(9600);
}

void loop() {
  unsigned long tiempoActual = millis();
  medicionCanal(canal1, tiempoActual);
  medicionCanal(canal2, tiempoActual);
  // Impimir resultados cada intervalo
  if (tiempoActual - tiempoImpresion >= intervalo) {
    tiempoImpresion = tiempoActual;
    Serial.print("F1: ");
    Serial.print(canal1.frecuencia);
    Serial.print("\tF2: ");
    Serial.print(canal2.frecuencia);
    Serial.print("\tD1: ");
    Serial.print(canal1.duty);
    Serial.print("\tD2: ");
    Serial.println(canal2.duty);
  }
  // Plotter
  Serial.print(canal1.estadoActual);
  Serial.print(",");
  Serial.println(canal2.estadoActual + 1);
}

void medicionCanal(Canal &canal, unsigned long tiempoActual) {
  canal.estadoActual = digitalRead(canal.pin);
  // Deteccion de flanco | Medicion de periodo | Calculo de frecuencia
  if (canal.estadoActual == HIGH && canal.estadoAnterior == LOW) {
    canal.periodo = tiempoActual - canal.tiempoAnterior;
    canal.tiempoAnterior = tiempoActual;
    if (canal.periodo > 0) {
      canal.frecuencia = 1000.0 / canal.periodo;
    }
  }
  // Medicion de duty cycle
  if (canal.estadoActual != canal.estadoAnterior) {
    if (canal.estadoActual == HIGH) {
      canal.tLow = tiempoActual - canal.tiempoCambio;
    } else {
      canal.tHigh = tiempoActual - canal.tiempoCambio;
    }
    canal.tiempoCambio = tiempoActual;

    if ((canal.tHigh + canal.tLow) > 0) {
      canal.duty = (float)canal.tHigh / (canal.tHigh + canal.tLow) * 100.0;
    }
  }
  canal.estadoAnterior = canal.estadoActual;
}