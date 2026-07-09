#include <Arduino.h>

const uint8_t PIN_POTENCIOMETRO = 34;   
const uint8_t PIN_SERVOMOTOR = 13;   
const uint16_t FRECUENCIA_PWM = 50;  
const uint8_t RESOLUCION_PWM = 16;    // Resolución del PWM (16 bits)

// Tiempo del pulso necesario para alcanzar los extremos del servo
const uint16_t PULSO_MINIMO_US = 500;
const uint16_t PULSO_MAXIMO_US = 2400;

QueueHandle_t colaValorPotenciometro;

void TareaLecturaPotenciometro(void *parametros) {
  uint16_t valorPotenciometro;
  while (true) {
    // Leer el valor analógico del potenciómetro (0 - 4095)
    valorPotenciometro = analogRead(PIN_POTENCIOMETRO);
    // Enviar el valor leído a la cola
    xQueueSend(colaValorPotenciometro, &valorPotenciometro, portMAX_DELAY);
    // Esperar 20 ms antes de realizar otra lectura
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

void TareaControlServomotor(void *parametros) {
  uint16_t valorPotenciometro;
  while (true) {
    // Esperar hasta recibir un dato de la cola
    if (xQueueReceive(colaValorPotenciometro, &valorPotenciometro, portMAX_DELAY) == pdTRUE) {
      // Convertir la lectura del ADC (0-4095) a un ángulo entre 0° y 180°
      int anguloServo = map(valorPotenciometro, 0, 4095, 0, 180);
      // Convertir el ángulo a un ancho de pulso comprendido entre 500 us y 2400 us
      int anchoPulsoMicrosegundos = map(anguloServo, 0, 180, PULSO_MINIMO_US, PULSO_MAXIMO_US);
      // Calcular el valor máximo permitido por la resolución del PWM
      uint32_t valorMaximoPWM = (1 << RESOLUCION_PWM) - 1;
      // Convertir el ancho del pulso al valor de Duty Cycle
      uint32_t valorDutyPWM = (anchoPulsoMicrosegundos * valorMaximoPWM) / 20000;
      // Actualizar el PWM enviado al servomotor
      ledcWrite(PIN_SERVOMOTOR, valorDutyPWM);
      // Mostrar los datos en el Monitor Serie
      Serial.print("ADC: ");
      Serial.print(valorPotenciometro);
      Serial.print(" | Angulo: ");
      Serial.print(anguloServo);
      Serial.print("° | Duty: ");
      Serial.println(valorDutyPWM);
    }
  }
}

void setup() {
  Serial.begin(115200);
  // Configurar el ADC con resolución de 12 bits
  analogReadResolution(12);
  // Configurar el periférico LEDC para generar PWM
  ledcAttach(PIN_SERVOMOTOR, FRECUENCIA_PWM, RESOLUCION_PWM);
  // Crear una cola con capacidad para 5 elementos tipo uint16_t
  colaValorPotenciometro = xQueueCreate(5, sizeof(uint16_t));
  // Crear la tarea encargada de leer el potenciómetro
  xTaskCreatePinnedToCore(TareaLecturaPotenciometro, "LecturaPotenciometro", 2048, NULL, 1, NULL, 1);
  // Crear la tarea encargada de mover el servomotor
  xTaskCreatePinnedToCore(TareaControlServomotor, "ControlServomotor", 4096, NULL, 1, NULL, 1);
}

void loop() {}