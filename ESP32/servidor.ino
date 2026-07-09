#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPDash.h>

const char* ssid = ""; // SSID
const char* password = ""; // Password
const int LED = 2;

AsyncWebServer server(80);
ESPDash dashboard(server);

dash::GenericCard tarjetaNodo(dashboard, "Nodo");
dash::TemperatureCard tarjetaTemperatura(dashboard, "Temperatura");
dash::HumidityCard tarjetaHumedad(dashboard, "Humedad");
dash::GenericCard tarjetaLED(dashboard, "Estado LED");
dash::GenericCard tarjetaPaquete(dashboard, "Paquete");

struct DatosNodo {
  uint8_t nodo;
  float temperatura;
  float humedad;
  bool estadoLED;
  uint32_t paquete;
};

DatosNodo datos = {1, 0.0, 0.0, false, 0};

unsigned long tiempoAnterior = 0;
const unsigned long intervalo = 3000;

void setup() {
  Serial.begin(115200);

  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
 
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
 
  server.begin();
}

void loop() {
  unsigned long tiempoActual = millis();
  if (tiempoActual - tiempoAnterior >= intervalo) {
    tiempoAnterior = tiempoActual;
    
    // Simulación de datos
    datos.paquete++;
    datos.temperatura = random(200, 350) / 10.0;   //20.0 - 35.0 °C
    datos.humedad = random(400, 900) / 10.0;        //40 - 90 %

    // Cambiar estado LED
    datos.estadoLED = !datos.estadoLED;
    digitalWrite(LED, datos.estadoLED);

    // Mostrar por Serial
    Serial.print("Nodo: ");
    Serial.print(datos.nodo);
    Serial.print(" | Paquete: ");
    Serial.print(datos.paquete);
    Serial.print(" | Temperatura: ");
    Serial.print(datos.temperatura);
    Serial.print(" °C");
    Serial.print(" | Humedad: ");
    Serial.print(datos.humedad);
    Serial.print(" %");
    Serial.print(" | LED: ");
    Serial.println(datos.estadoLED ? "ON" : "OFF");

    // Actualizar Dashboard
    tarjetaNodo.setValue(String(datos.nodo));
    tarjetaPaquete.setValue(String(datos.paquete));
    tarjetaTemperatura.setValue(datos.temperatura);
    tarjetaHumedad.setValue(datos.humedad);
    tarjetaLED.setValue(datos.estadoLED ? "ON" : "OFF");
    
    dashboard.sendUpdates();
  }
}
