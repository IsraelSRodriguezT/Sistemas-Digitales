const byte pinSensor = A0;
const byte pinBoton = 2;
const byte pinBuzzer = 6;
const byte pinLedRojo = 7;
const byte pinLedVerde = 8;

const unsigned long intervaloHeartbeat = 500;
const unsigned long intervaloTelemetria = 2000;
const unsigned long intervaloAlarma = 300;
const unsigned long debounce = 40;

const float umbralAlarma = 30.0;

unsigned long tiempoAnteriorHeartbeat = 0;
unsigned long tiempoAnteriorTelemetria = 0;
unsigned long tiempoAnteriorAlarma = 0;
unsigned long tiempoAnteriorDebounce = 0;

bool estadoLedRojo = LOW;
bool estadoLedVerde = LOW;
bool alarmaActiva = false;
bool alarmaSilenciada = false;

bool lecturaBotonEstable = LOW;
bool ultimaLecturaBoton = LOW;

float temperatura = 0.0;

void setup() {
	Serial.begin(9600);

    pinMode(pinSensor, INPUT);
	pinMode(pinBoton, INPUT);
	pinMode(pinLedVerde, OUTPUT);
	pinMode(pinLedRojo, OUTPUT);
	pinMode(pinBuzzer, OUTPUT);
	
    digitalWrite(pinLedVerde, LOW);
	digitalWrite(pinLedRojo, LOW);
	noTone(pinBuzzer);
    
	Serial.println("SISTEMA DE MONITOREO INICIADO");
    
	leerTemperatura(&temperatura);
}

void loop() {
	unsigned long tiempoActual = millis();
    
	actualizarHeartbeat(tiempoActual);   // Thread 1: LED verde cada 500 ms
	enviarTelemetria(tiempoActual);      // Thread 2: Telemetria cada 2 s
	procesarBoton(tiempoActual);         // Thread 4: Evento asincrono por boton
	procesarAlarma(tiempoActual);        // Thread 3: Alarma termica cada 300 ms
}

void leerTemperatura(float *temperatura) {
    int lecturaAnalogica = analogRead(pinSensor);
    float voltaje = (lecturaAnalogica * 5.0) / 1023.0;
    *temperatura = voltaje * 100.0;
}

void actualizarHeartbeat(unsigned long tiempoActual) {
    if (tiempoActual - tiempoAnteriorHeartbeat >= intervaloHeartbeat) {
        tiempoAnteriorHeartbeat = tiempoActual;
        estadoLedVerde = !estadoLedVerde;
        digitalWrite(pinLedVerde, estadoLedVerde);
    }
}

void enviarTelemetria(unsigned long tiempoActual) {
    if (tiempoActual - tiempoAnteriorTelemetria >= intervaloTelemetria) {
        tiempoAnteriorTelemetria = tiempoActual;
        leerTemperatura(&temperatura);

        Serial.print("Temperatura: ");
        Serial.print(temperatura, 2);
        Serial.println(" C");
    }
}

void procesarBoton(unsigned long tiempoActual) {
    bool lecturaActual = digitalRead(pinBoton);

    if (lecturaActual != ultimaLecturaBoton) {
        tiempoAnteriorDebounce = tiempoActual;
        ultimaLecturaBoton = lecturaActual;
    }

    if (tiempoActual - tiempoAnteriorDebounce >= debounce) {
        if (lecturaActual != lecturaBotonEstable) {
            lecturaBotonEstable = lecturaActual;

            if (lecturaBotonEstable == HIGH && alarmaActiva && !alarmaSilenciada) {
                alarmaSilenciada = true;
                estadoLedRojo = LOW;
                digitalWrite(pinLedRojo, estadoLedRojo);
                noTone(pinBuzzer);
                Serial.println("ALARMA SILENCIADA MANUALMENTE POR OPERADOR");
            }
        }
    }
}

void procesarAlarma(unsigned long tiempoActual) {
    bool debeActivarAlarma = (temperatura > umbralAlarma);

    if (!debeActivarAlarma) {
        if (alarmaActiva || alarmaSilenciada) {
            Serial.println("ALARMA TERMICA DESACTIVADA");
            alarmaActiva = false;
            alarmaSilenciada = false;
            estadoLedRojo = LOW;
            digitalWrite(pinLedRojo, estadoLedRojo);
            noTone(pinBuzzer);
        }
        return;
    }

    if (!alarmaActiva) {
        alarmaActiva = true;
        alarmaSilenciada = false;
        tiempoAnteriorAlarma = tiempoActual;
        Serial.println("ALARMA TERMICA ACTIVA: temperatura > 30 C");
    }

    if (alarmaSilenciada) {
        digitalWrite(pinLedRojo, LOW);
        noTone(pinBuzzer);
        return;
    }

    if (tiempoActual - tiempoAnteriorAlarma >= intervaloAlarma) {
        tiempoAnteriorAlarma = tiempoActual;
        estadoLedRojo = !estadoLedRojo;

        digitalWrite(pinLedRojo, estadoLedRojo);
        if (estadoLedRojo) {
            tone(pinBuzzer, 2000);
        } else {
            noTone(pinBuzzer);
        }
    }
}