const int LED = 2;

unsigned long tiempoLED = 0;
const unsigned long DURACION_LED = 200;   // ms

void setup() {
  Serial.begin(115200);

  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  Serial.println("=== RECEPTOR ESP32 ===");
  Serial.println("Formato esperado:");
  Serial.println("ID Nodo: 01 | Temperatura: 25°C | Estado: ON | Contador: 16");
}

void loop() {

  // Procesar todas las líneas disponibles
  while (Serial.available()) {

    String paquete = Serial.readStringUntil('\n');
    paquete.trim();

    if (paquete.length() == 0)
      continue;

    Serial.println("\n===== PAQUETE RECIBIDO =====");
    Serial.println(paquete);
    Serial.println("============================");

    // Buscar los campos del paquete
    int p1 = paquete.indexOf("ID Nodo: ");
    int p2 = paquete.indexOf(" | Temperatura: ");
    int p3 = paquete.indexOf(" | Estado: ");
    int p4 = paquete.indexOf(" | Contador: ");

    // Buscar dónde termina la temperatura
    int pTemp = paquete.indexOf("°C");

    // Validar estructura
    bool formatoCorrecto =
      (p1 == 0) &&               // Debe comenzar con "ID Nodo:"
      (p2 > p1) &&
      (p3 > p2) &&
      (p4 > p3) &&
      (pTemp > p2);

    if (formatoCorrecto) {

      String id = paquete.substring(p1 + 9, p2);
      String temperatura = paquete.substring(p2 + 16, pTemp);
      String estado = paquete.substring(p3 + 11, p4);
      String contador = paquete.substring(p4 + 13);

      id.trim();
      temperatura.trim();
      estado.trim();
      contador.trim();

      // Validación adicional
      bool datosValidos = true;

      if (id.length() == 0) datosValidos = false;
      if (temperatura.toInt() <= 0) datosValidos = false;
      if (!(estado == "ON" || estado == "OFF")) datosValidos = false;
      if (contador.toInt() < 0) datosValidos = false;

      if (datosValidos) {

        // Encender LED
        digitalWrite(LED, HIGH);
        tiempoLED = millis();

        Serial.println("Paquete válido.");

        Serial.print("ID Nodo: ");
        Serial.println(id);

        Serial.print("Temperatura: ");
        Serial.print(temperatura);
        Serial.println(" °C");

        Serial.print("Estado: ");
        Serial.println(estado);

        Serial.print("Contador: ");
        Serial.println(contador);

      } else {

        Serial.println("Error: datos inválidos.");

      }

    } else {

      Serial.println("Error: formato de paquete incorrecto.");

    }

    Serial.println();
  }

  // Apagar LED sin usar delay()
  if (digitalRead(LED) == HIGH &&
      millis() - tiempoLED >= DURACION_LED) {

    digitalWrite(LED, LOW);
  }
}