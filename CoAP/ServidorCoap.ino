#include <WiFi.h>
#include <WiFiUdp.h>
#include <coap-simple.h>
#include <ArduinoJson.h>

const char* ssid = "";
const char* password = "";

const int PUERTO_COAP = 5683;

WiFiUDP udp;
Coap coap(udp);

void alRecibirSensores(CoapPacket &paquete, IPAddress ip, int puerto) {
    bool es_json = (paquete.payloadlen > 0 && paquete.payload[0] == '{');

    StaticJsonDocument<256> doc;
    DeserializationError error;

    if (es_json) {
        error = deserializeJson(doc, (const char*)paquete.payload, paquete.payloadlen);
    } else {
        error = deserializeMsgPack(doc, paquete.payload, paquete.payloadlen);
    }

    if (error) {
        Serial.print(es_json ? "[JSON ERROR]" : "[CBOR ERROR]");
        Serial.print(" Tamanio: ");
        Serial.print(paquete.payloadlen);
        Serial.print(" B | ");
        Serial.println(error.c_str());
        return;
    }

    const char* id_nodo = doc["id_nodo"];
    float temperatura = doc["temperatura"];
    int humedad = doc["humedad"];
    const char* estado = doc["estado"];

    Serial.print(es_json ? "[RECIBIDO JSON]" : "[RECIBIDO CBOR]");
    Serial.print(" Tamanio: ");
    Serial.print(paquete.payloadlen);
    Serial.print(" B | ");
    Serial.print(id_nodo);
    Serial.print(" | ");
    Serial.print(temperatura, 2);
    Serial.print(" C | ");
    Serial.print(humedad);
    Serial.print("% | ");
    Serial.println(estado);
}

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

    coap.server(alRecibirSensores, "sensores");
    coap.start();

    Serial.print("Servidor CoAP listo en puerto ");
    Serial.println(PUERTO_COAP);
    Serial.println("Recurso: /sensores");
}

void loop() {
    coap.loop();
}
