#include <esp_now.h>
#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "TU_SSID";           // <<<<<<<<< Cambia por tu red Wi-Fi
const char* password = "TU_CONTRASEÑA"; // <<<<<<<<< Cambia por tu contraseña
const char* mqtt_server = "192.168.1.100"; // <<<<<<<<< Cambia por la IP de tu PC
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient mqtt(espClient);

typedef struct {
  uint8_t id_nodo;
  float temperatura;
  uint16_t nivel_humo;
  unsigned long timestamp;
} Mensaje;

#define HUMO_CRITICO  2800
#define TEMP_CRITICA  45.0f
#define HUMO_DELTA    500
#define TEMP_DELTA    5.0f

QueueHandle_t msg_queue;
float last_temp[3] = {0};
uint16_t last_humo[3] = {0};

// Callback receptor ESP-NOW (compatibilidad con versión nueva)
void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  if (len == sizeof(Mensaje)) {
    Mensaje *msg = (Mensaje *)malloc(sizeof(Mensaje));
    memcpy(msg, data, sizeof(Mensaje));
    xQueueSend(msg_queue, &msg, 0);
  }
}

void task_wifi_mqtt(void *pvParams) {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    vTaskDelay(pdMS_TO_TICKS(500));
  }
  Serial.println("\nWiFi conectado");

  mqtt.setServer(mqtt_server, mqtt_port);

  while (1) {
    if (!mqtt.connected()) {
      if (mqtt.connect("central_incendios")) {
        Serial.println("MQTT conectado");
      }
    }
    mqtt.loop();
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

void task_validation_alarm(void *pvParams) {
  Mensaje *msg = NULL;
  char payload[256];

  while (1) {
    if (xQueueReceive(msg_queue, &msg, pdMS_TO_TICKS(500)) == pdTRUE) {
      uint8_t id = msg->id_nodo;
      float temp = msg->temperatura;
      uint16_t humo = msg->nivel_humo;

      if (id < 3) {
        float dtemp = fabs(temp - last_temp[id]);
        uint16_t dhumo = abs((int)humo - (int)last_humo[id]);

        bool humo_critico = (humo > HUMO_CRITICO);
        bool temp_critica = (temp > TEMP_CRITICA);
        bool cambio_brusco = (dtemp > TEMP_DELTA || dhumo > HUMO_DELTA);

        snprintf(payload, sizeof(payload),
                 "{\"id\":%d,\"temp\":%.1f,\"humo\":%d,\"ts\":%lu}",
                 id, temp, humo, msg->timestamp);
        mqtt.publish("incendios/datos", payload);

        if (humo_critico && temp_critica) {
          snprintf(payload, sizeof(payload),
                   "{\"id\":%d,\"tipo\":\"FIRE\",\"temp\":%.1f,\"humo\":%d}",
                   id, temp, humo);
          mqtt.publish("incendios/alerta", payload);
          Serial.printf("ALERTA FIRE - Nodo %d\n", id);
        } else if (cambio_brusco) {
          snprintf(payload, sizeof(payload),
                   "{\"id\":%d,\"tipo\":\"CAMBIO_BRUSCO\",\"delta_temp\":%.1f,\"delta_humo\":%d}",
                   id, dtemp, dhumo);
          mqtt.publish("incendios/cambio_brusco", payload);
        }

        last_temp[id] = temp;
        last_humo[id] = humo;
      }

      free(msg);
    }
  }
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  WiFi.mode(WIFI_STA);

  msg_queue = xQueueCreate(10, sizeof(Mensaje *));

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW error");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);

  xTaskCreatePinnedToCore(task_wifi_mqtt, "wifi_mqtt", 8192, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(task_validation_alarm, "validation", 8192, NULL, 1, NULL, 1);
}

void loop() {
  vTaskDelay(portMAX_DELAY);
}