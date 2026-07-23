#include <WiFi.h>
#include <WiFiUdp.h>
#include <coap-simple.h>
#include <ArduinoJson.h>
#include <math.h>

const char* ssid = "";
const char* password = "";

const char* ID_NODO = "Nodo01";
const int INTERVALO_ENVIO = 5000;

IPAddress IP_SERVIDOR(192, 168, 1, 100);
const int PUERTO_COAP = 5683;
const char* RECURSO = "sensores";

WiFiUDP udp;
Coap coap(udp);
float fase_temperatura = 0;
float fase_humedad = 0;

void conectarWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.print("Conectando a WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    Serial.println();
    Serial.print("WiFi conectado. IP: ");
    Serial.println(WiFi.localIP());
}

void setup() {
    Serial.begin(115200);
    conectarWiFi();
    coap.start();
    Serial.println("Cliente CBOR listo. Enviando solo CBOR...");
    Serial.print("Enviando a ");
    Serial.print(IP_SERVIDOR);
    Serial.print(":");
    Serial.println(PUERTO_COAP);
}

void loop() {
    static unsigned long ultimo_envio = 0;

    if (millis() - ultimo_envio >= INTERVALO_ENVIO) {
        ultimo_envio = millis();

        float temperaturaS = 25.0 + 5.0 * sin(fase_temperatura);
        float temperatura = round(temperaturaS * 100.0) / 100.0;
        fase_temperatura += 0.3;

        int humedad = (int)(60 + 10 * sin(fase_humedad));
        fase_humedad += 0.3;

        StaticJsonDocument<256> doc;
        doc["id_nodo"] = ID_NODO;
        doc["temperatura"] = temperatura;
        doc["humedad"] = humedad;
        doc["estado"] = "activo";

        uint8_t buffer[256];
        size_t tam = serializeMsgPack(doc, buffer);

        coap.send(IP_SERVIDOR, PUERTO_COAP, RECURSO,
                  COAP_NONCON, COAP_POST, NULL, 0,
                  buffer, tam, COAP_APPLICATION_CBOR);

        Serial.print("[CBOR] Tamanio: ");
        Serial.print(tam);
        Serial.print(" B | (binario no legible)");
        Serial.println();
    }

    coap.loop();
}
