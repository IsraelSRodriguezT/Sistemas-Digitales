#include <esp_now.h>
#include <WiFi.h>

#define PIN_TMP36    35
#define PIN_POT      34
#define HUMO_CRITICO  2800
#define TEMP_CRITICA  45.0f
#define HUMO_DELTA    500
#define TEMP_DELTA    5.0f
#define SAMPLES       5

uint8_t mac_repetidor[] = {0xB4, 0xBF, 0xE9, 0x13, 0xAD, 0xFD};  // Repetidor con MAC distinta
int id_nodo = 1;  // Cambia a 2 en el segundo sensor

typedef struct {
  uint8_t id_nodo;
  float temperatura;
  uint16_t nivel_humo;
  unsigned long timestamp;
} Mensaje;

Mensaje mensaje;

float current_temp = 0, last_temp = 0;
uint16_t current_humo = 0, last_humo = 0;
SemaphoreHandle_t mutex;

void OnDataSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {
  // No necesitamos hacer nada aquí
}

void task_sensor_read(void *pvParams) {
  while (1) {
    float temp_sum = 0, volt;
    uint32_t humo_sum = 0;

    for (int i = 0; i < SAMPLES; i++) {
      int raw = analogRead(PIN_TMP36);
      volt = (raw * 3.3f) / 4095.0f;
      temp_sum += (volt - 0.5f) * 100.0f;
      humo_sum += analogRead(PIN_POT);
      delay(5);
    }

    xSemaphoreTake(mutex, portMAX_DELAY);
    current_temp = temp_sum / SAMPLES;
    current_humo = humo_sum / SAMPLES;
    xSemaphoreGive(mutex);

    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

void task_validation(void *pvParams) {
  while (1) {
    vTaskDelay(pdMS_TO_TICKS(2000));

    xSemaphoreTake(mutex, portMAX_DELAY);
    float dtemp = fabs(current_temp - last_temp);
    uint16_t dhumo = abs((int)current_humo - (int)last_humo);

    if (dtemp > TEMP_DELTA || dhumo > HUMO_DELTA) {
      Serial.printf("Nodo %d | Cambio brusco: dT=%.1f dH=%d\n", id_nodo, dtemp, dhumo);
    }
    if (current_humo > HUMO_CRITICO && current_temp > TEMP_CRITICA) {
      Serial.printf("Nodo %d | CRITICO: T=%.1f H=%d\n", id_nodo, current_temp, current_humo);
    }

    last_temp = current_temp;
    last_humo = current_humo;
    xSemaphoreGive(mutex);
  }
}

void task_comms(void *pvParams) {
  while (1) {
    xSemaphoreTake(mutex, portMAX_DELAY);
    mensaje.id_nodo = id_nodo;
    mensaje.temperatura = current_temp;
    mensaje.nivel_humo = current_humo;
    mensaje.timestamp = millis();
    xSemaphoreGive(mutex);

    esp_err_t res = esp_now_send(mac_repetidor, (uint8_t *)&mensaje, sizeof(mensaje));
    if (res != ESP_OK) Serial.println("Error envio ESP-NOW");

    vTaskDelay(pdMS_TO_TICKS(3000));
  }
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW error");
    return;
  }

  esp_now_register_send_cb(OnDataSent);

  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, mac_repetidor, 6);
  peer.channel = 0;
  peer.encrypt = false;
  esp_now_add_peer(&peer);

  mutex = xSemaphoreCreateMutex();

  xTaskCreatePinnedToCore(task_sensor_read, "sensor_read", 4096, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(task_validation, "validation", 4096, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(task_comms, "comms", 4096, NULL, 1, NULL, 1);
}

void loop() {
  vTaskDelay(portMAX_DELAY);
}