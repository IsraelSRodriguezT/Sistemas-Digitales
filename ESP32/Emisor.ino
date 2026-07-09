const String ID_NODO = "01";

int contador = 1;
int temperatura = 24;
String estado = "ON";

const unsigned long intervalo = 1000;   // 1000 ms = 1 segundo
unsigned long tiempoAnterior = 0;

void setup() {
  Serial.begin(115200);
}

void loop() {
  unsigned long tiempoActual = millis();
  if (tiempoActual - tiempoAnterior >= intervalo) {
    tiempoAnterior = tiempoActual;
    // Enviar paquete
    Serial.print("ID Nodo: ");
    Serial.print(ID_NODO);
    Serial.print(" | Temperatura: ");
    Serial.print(temperatura);
    Serial.print("°C");
    Serial.print(" | Estado: ");
    Serial.print(estado);
    Serial.print(" | Contador: ");
    Serial.println(contador);
    // Actualizar datos
    contador++;
    temperatura++;
    if (temperatura > 30) {
      temperatura = 24;
    }
  }

  // Aquí podrían ejecutarse otras tareas sin quedar bloqueadas.
}