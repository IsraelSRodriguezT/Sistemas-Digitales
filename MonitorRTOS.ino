#include <Arduino_FreeRTOS.h>
#include <queue.h>
#include <LiquidCrystal.h> 

const byte pinSensorTemperatura = A1;
const byte pinSensorPulso = A2;
const byte pinBuzzer = 2;

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
QueueHandle_t colaSensores;
QueueHandle_t colaProcesamiento;

typedef struct {
  int temperatura;
  int pulso;
} DatosSensores;

typedef struct {
  float temperatura;
  int pulso;
  bool alerta;
} DatosProcesados;

void TareaAdquisicion(void *pvParameters);
void TareaProcesamiento(void *pvParameters);
void TareaSalida(void *pvParameters);

void setup() {
  Serial.begin(9600);

  pinMode(pinSensorTemperatura, INPUT);
  pinMode(pinSensorPulso, INPUT);
  pinMode(pinBuzzer, OUTPUT);
  noTone(pinBuzzer);
  
  lcd.begin(16, 2);

  colaSensores = xQueueCreate(5, sizeof(DatosSensores));
  colaProcesamiento = xQueueCreate(5, sizeof(DatosProcesados));

  if (!colaSensores || !colaProcesamiento) {
    lcd.clear();
    lcd.print("Error Sistema");
    while (true);
  } else {
    lcd.clear();
    lcd.print("Sistema");
    lcd.setCursor(0,1);
    lcd.print("Monitoreo");
    
    xTaskCreate(TareaAdquisicion, "Adquisicion", 128, NULL, 3, NULL);
    xTaskCreate(TareaProcesamiento, "Procesamiento", 128, NULL, 2, NULL);
    xTaskCreate(TareaSalida, "Salida", 128, NULL, 1, NULL);
  }
}

void loop() {}

void clasificarSignosVitales(float temperatura, int pulso, bool &alerta) {
  alerta = false;
  if(temperatura < 35 || temperatura > 39.5) {
    alerta = true;
  }
  if(pulso > 0 && (pulso < 60 || pulso > 120)) {
    alerta = true;
  }
}

void TareaAdquisicion(void *pvParameters) {
  DatosSensores datos;
  for(;;) {
    int lectura = analogRead(pinSensorTemperatura);
    int lecturaPulso = analogRead(pinSensorPulso);
    datos.temperatura = lectura;
    datos.pulso = map(lecturaPulso, 0, 1023, 50, 150); // Simulación de pulso basado en la lectura del sensor
    xQueueSend(colaSensores,&datos,portMAX_DELAY);
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

void TareaProcesamiento(void *pvParameters) {
  DatosSensores datosEntrada;
  DatosProcesados datosSalida;
  for(;;) {
    if(xQueueReceive(colaSensores, &datosEntrada, portMAX_DELAY) == pdPASS) {
      float voltaje = (datosEntrada.temperatura * 5.0) / 1023.0;
      datosSalida.temperatura = (voltaje - 0.5) * 100.0;
      datosSalida.pulso = datosEntrada.pulso;
      clasificarSignosVitales(datosSalida.temperatura, datosSalida.pulso, datosSalida.alerta);
      xQueueSend(colaProcesamiento, &datosSalida, portMAX_DELAY);
    }
  }
}

void TareaSalida(void *pvParameters) {
  DatosProcesados datos;
  for(;;) {
    if(xQueueReceive(colaProcesamiento, &datos, portMAX_DELAY) == pdPASS) {
      Serial.print("Temp: ");
      Serial.print(datos.temperatura);
      Serial.print(" C ");

      Serial.print("Pulso: ");
      Serial.print(datos.pulso);

      Serial.print(" BPM ");
      Serial.print("Alerta: ");
      Serial.println(datos.alerta);

      if(datos.alerta) {
        Serial.println("***** ALERTA MEDICA *****");
        tone(pinBuzzer,1000);
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("ALERTA!");
        
        lcd.setCursor(0,1);
        lcd.print("Revisar Paciente");
    } else {
        noTone(pinBuzzer);
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("T:");
        lcd.print(datos.temperatura);

        lcd.print("C");
        lcd.setCursor(0,1);

        lcd.print("P:");
        lcd.print(datos.pulso);
        lcd.print(" BPM");
      }
    }
  }
}