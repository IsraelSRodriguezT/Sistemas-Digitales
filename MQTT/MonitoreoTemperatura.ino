#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = ""; // SSID
const char* password = ""; // Password

const char* broker = ""; // Ip de Linux
const int puerto = 1883;

const char* topicoTemperatura = "esp32/temperatura";
const char* topicoLED = "esp32/led";

const int pinSensor = 34;
const int pinLED = 2;

WiFiClient clienteWiFi;
PubSubClient clienteMqtt(clienteWiFi);

bool estadoLED = false;

void conectarWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.print("Conectando");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        yield();
    }
    Serial.println();
    Serial.println("WiFi conectado");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
}

void conectarMqtt() {
    while (!clienteMqtt.connected()) {
        Serial.print("Conectando a MQTT...");
        if (clienteMqtt.connect("ESP32_LM35")) {
            clienteMqtt.subscribe(topicoLED);
            Serial.println("[MQTT] Conectado y suscrito.");
        } else {
            Serial.print("fallo, codigo: ");
            Serial.println(clienteMqtt.state());
            vTaskDelay(2000 / portTICK_PERIOD_MS);
        }
    }
}

void procesarMensajeMqtt(char* topico, byte* datos, unsigned int largo) {
    String mensaje = "";
    for (int i = 0; i < largo; i++) {
        mensaje += (char)datos[i];
    }
    Serial.print("Mensaje recibido en ");
    Serial.print(topico);
    Serial.print(": ");
    Serial.println(mensaje);
    
    if (String(topico) == topicoLED) {
        if (mensaje == "ON") {
            estadoLED = true;
            Serial.println("[ON] Estado del LED actualizado.");
        } else if (mensaje == "OFF") {
            estadoLED = false;
            Serial.println("[OFF] Estado del LED actualizado.");
        }
    }
}

void tareaLectura(void* parametros) {
    while (true) {
        int valor_adc = analogRead(pinSensor);
        float voltaje = valor_adc * (3.3 / 4095.0);
        float celsius = voltaje * 100.0;
        
        char buffer[10];
        dtostrf(celsius, 1, 2, buffer);
        clienteMqtt.publish(topicoTemperatura, buffer);
        
        Serial.print("[SENSOR] Temperatura publicada: ");
        Serial.println(buffer);

        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

void tareaMqtt(void* parametros) {
    while (true) {
        if (WiFi.status() != WL_CONNECTED) {
            conectarWiFi();
        }
        if (!clienteMqtt.connected()) {
            conectarMqtt();
        }
        clienteMqtt.loop();
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

void tareaLED(void* parametros) {
    while (true) {
        if (estadoLED) {
            digitalWrite(pinLED, HIGH);
        } else {
            digitalWrite(pinLED, LOW);
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void setup() {
    Serial.begin(115200);
    pinMode(pinLED, OUTPUT);
    digitalWrite(pinLED, LOW);

    conectarWiFi();
    clienteMqtt.setServer(broker, puerto);
    clienteMqtt.setCallback(procesarMensajeMqtt);

    xTaskCreatePinnedToCore(tareaLectura, "Tarea Sensor", 4096, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(tareaMqtt, "Tarea MQTT", 4096, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(tareaLED, "Tarea LED", 2048, NULL, 1, NULL, 1);
}

void loop() {}