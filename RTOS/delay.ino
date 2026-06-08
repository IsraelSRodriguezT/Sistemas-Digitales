// Código Bloqueante (NO es RTOS)

void setup() {
 pinMode(2, OUTPUT); // Tarea 1
 pinMode(3, OUTPUT); // Tarea 2
 pinMode(4, OUTPUT); // Tarea 3
}

void loop() {
 // Tarea 1: Parpadea cada 100ms
 digitalWrite(2, HIGH);
 delay(100);
 digitalWrite(2, LOW);
 delay(100);
 // Tarea 2: Parpadea cada 500ms
 digitalWrite(3, HIGH);
 delay(500);
 digitalWrite(3, LOW);
 delay(500);
 // Tarea 3: Parpadea cada 1000ms
 digitalWrite(4, HIGH);
 delay(1000);
 digitalWrite(4, LOW);
 delay(1000);
}