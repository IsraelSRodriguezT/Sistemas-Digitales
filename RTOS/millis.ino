// Definición de Pines
const int PIN_T1 = 2;
const int PIN_T2 = 3;
const int PIN_T3 = 4;
// Variables de Tiempo (Para simular el Reloj del Sistema)
unsigned long tiempoActual = 0;
// Definición de Tareas (Estructura de datos simple)
struct Tarea {
 int pin;
 unsigned long periodo; // Deadline / Periodo
 unsigned long proximaEjec; // Cuándo debe ejecutarse la próxima vez
 int estado; // 0 apagado, 1 encendido
};
// Crear las 3 tareas
Tarea tarea1 = {PIN_T1, 200, 0, 0};
Tarea tarea2 = {PIN_T2, 500, 0, 0};
Tarea tarea3 = {PIN_T3, 1000, 0, 0};

void ejecutarTarea(Tarea &t);

void setup() {
 Serial.begin(9600); // Para debugging
 pinMode(PIN_T1, OUTPUT);
 pinMode(PIN_T2, OUTPUT);
 pinMode(PIN_T3, OUTPUT);
}
void loop() {
 tiempoActual = millis(); // Leemos el reloj del sistema
 // --- PLANIFICADOR SIMPLE ---

 // Verificar Tarea 1
 if (tiempoActual >= tarea1.proximaEjec) {
 	ejecutarTarea(tarea1);
 	tarea1.proximaEjec += tarea1.periodo;
 }
 // Verificar Tarea 2
 if (tiempoActual >= tarea2.proximaEjec) {
 	ejecutarTarea(tarea2);
 	tarea2.proximaEjec += tarea2.periodo;
 }
 // Verificar Tarea 3
 if (tiempoActual >= tarea3.proximaEjec) {
 	ejecutarTarea(tarea3);
 	tarea3.proximaEjec += tarea3.periodo;
 }

 // Aquí podríamos poner una tarea "Idle" si el CPU está libre
}
// Función que simula el trabajo de la tarea (Cambiar estado LED)
void ejecutarTarea(Tarea &t) {
 t.estado = !t.estado; // Toggle (cambiar)
 digitalWrite(t.pin, t.estado);
 // Nota: En un RTOS real, aquí iría el código complejo de la tarea
}
