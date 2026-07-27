#include <esp_now.h>
#include <WiFi.h>

typedef struct {
  uint8_t id_nodo;
  float temperatura;
  uint16_t nivel_humo;
  unsigned long timestamp;
} Mensaje;

uint8_t mac_central[] = {0xD4, 0xE9, 0xF4, 0xB3, 0x6C, 0x70};

Mensaje msg_recibido;
bool nuevo_mensaje = false;
SemaphoreHandle_t mutex;

// Callback receptor ESP-NOW (actualizado para versión 3.3.11+)
void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  if (len == sizeof(Mensaje)) {
    xSemaphoreTake(mutex, portMAX_DELAY);
    memcpy(&msg_recibido, data, sizeof(Mensaje));
    nuevo_mensaje = true;
    xSemaphoreGive(mutex);
  }
}

// Callback envío ESP-NOW
void OnDataSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {
  // No necesario
}

void task_forward(void *pvParams) {
  while (1) {
    xSemaphoreTake(mutex, portMAX_DELAY);
    if (nuevo_mensaje) {
      esp_now_send(mac_central, (uint8_t *)&msg_recibido, sizeof(Mensaje));
      nuevo_mensaje = false;
    }
    xSemaphoreGive(mutex);
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW error");
    return;
  }

  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, mac_central, 6);
  peer.channel = 0;
  peer.encrypt = false;
  esp_now_add_peer(&peer);

  mutex = xSemaphoreCreateMutex();
  xTaskCreatePinnedToCore(task_forward, "forward", 4096, NULL, 1, NULL, 1);
}

void loop() {
  vTaskDelay(portMAX_DELAY);
}