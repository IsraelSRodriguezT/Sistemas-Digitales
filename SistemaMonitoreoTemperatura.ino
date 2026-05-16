#include <LiquidCrystal.h>;

LiquidCrystal lcd(6,7,8,9,10,11);

const int pinLM35 = A0;
const int pinLedAzul = 2;
const int pinLedVerde = 3;
const int pinLedRojo = 4;

const float temperaturaFriaMax = 20.0;
const float temperaturaCalienteMin = 30.0;

unsigned long tiempoAnterior = 0;
const long intervaloLectura = 500; 

void setup() {
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Iniciando...");
  configurarPines();
  tiempoAnterior = millis();
}

void loop() {
  unsigned long tiempoActual = millis();
  if (tiempoActual - tiempoAnterior >= intervaloLectura) {
    tiempoAnterior = tiempoActual; 
    float temperatura = leerTemperatura();
    controlarIndicadores(temperatura);
    actualizarLCD(temperatura);
  }
}

void configurarPines() {
  pinMode(pinLedAzul, OUTPUT);
  pinMode(pinLedVerde, OUTPUT);
  pinMode(pinLedRojo, OUTPUT);
}

float leerTemperatura() {
  int lecturaAnalogica = analogRead(pinLM35);
  float voltaje = (lecturaAnalogica * 5.0) / 1024.0;
  float temperaturaCelsius = voltaje * 100.0; 
  return temperaturaCelsius;
}

void controlarIndicadores(float temp) {
  digitalWrite(pinLedAzul, LOW);
  digitalWrite(pinLedVerde, LOW);
  digitalWrite(pinLedRojo, LOW);

  if (temp < temperaturaFriaMax) {
    digitalWrite(pinLedAzul, HIGH);
  } else if (temp >= temperaturaFriaMax && temp < temperaturaCalienteMin) {
    digitalWrite(pinLedVerde, HIGH);
  } else {
    digitalWrite(pinLedRojo, HIGH);
  }
}

void actualizarLCD(float temp) {
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temp, 1);  
  lcd.print(" C    ");

  lcd.setCursor(0, 1); 
  
  if (temp < temperaturaFriaMax) {
    lcd.print("Frio (<");
    lcd.print(temperaturaFriaMax, 0);
    lcd.print("C)     ");
  } else if (temp >= temperaturaFriaMax && temp < temperaturaCalienteMin) {
    lcd.print("Norm (");
    lcd.print(temperaturaFriaMax, 0);
    lcd.print("-");
    lcd.print(temperaturaCalienteMin, 0);
    lcd.print("C)   ");
  } else {
    lcd.print("Alto (>=");
    lcd.print(temperaturaCalienteMin, 0);
    lcd.print("C)    ");  
  }
}