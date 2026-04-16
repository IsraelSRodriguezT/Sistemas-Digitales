int pinesLED [] = {2, 3, 4, 5, 6, 7};
int pinBoton = 8;
int patronActual = 0;

int numerosLED = sizeof(pinesLED) / sizeof(pinesLED[0]);

bool estadoBoton = LOW;
bool ultimoEstadoBoton = LOW;
unsigned long ultimoTiempoDebounce = 0;
unsigned long delayDebounce = 50;

void setup() {
  Serial.begin(9600);
  for (int i = 0; i < numerosLED; i++) {
    pinMode(pinesLED [i], OUTPUT);
  }
  pinMode(pinBoton, INPUT);
  randomSeed(analogRead(0));
}

void loop() {
  leerBoton();
  switch (patronActual) {
    case 0:
      patronSecuencia();
      break;
    case 1:
      patronPersecucion();
      break;
    case 2:
      patronParpadeo();
      break;
    case 3:
      patronAleatorio();
      break;
    case 4:
      patronOnda();
      break;
  }
}

void leerBoton() {
  int lectura = digitalRead(pinBoton);
  if (lectura != ultimoEstadoBoton) {
    ultimoTiempoDebounce = millis();
  }
  if ((millis() - ultimoTiempoDebounce) > delayDebounce) {
    if (lectura != estadoBoton) {
      estadoBoton = lectura;
      if (estadoBoton == HIGH) {
        patronActual = random(0, 5);
        Serial.print("Patron: ");
        Serial.println(patronActual);
      }
    }
  }
  ultimoEstadoBoton = lectura;
}

void patronSecuencia() {
  int i = 0;
  while (i < numerosLED) {
    leerBoton();
    digitalWrite(pinesLED [i], HIGH);
    digitalWrite(pinesLED[numerosLED - 1 - i], HIGH);

    delay(100);
    leerBoton();
    
    digitalWrite(pinesLED [i], LOW);
    digitalWrite(pinesLED[numerosLED - 1 - i], LOW);
    i++;
  }
}

void patronPersecucion() {
  for (int i = 0; i < numerosLED; i++) {
    leerBoton();
    digitalWrite(pinesLED [i], HIGH);
    if (i > 0) {
      digitalWrite(pinesLED [i - 1], LOW);
    }
    delay(100);
    leerBoton();
  }
  digitalWrite(pinesLED [numerosLED - 1], LOW);
}

void patronParpadeo() {
  for (int i = 0; i < numerosLED; i++) {
    leerBoton();
    digitalWrite(pinesLED [i], HIGH);
  }
  delay(200);
  leerBoton();

  for (int i = 0; i < numerosLED; i++) {
    leerBoton();
    digitalWrite(pinesLED [i], LOW);
  }
  delay(200);
  leerBoton();
}

void patronAleatorio() {
  int led = random(0, numerosLED);
  digitalWrite(pinesLED [led], HIGH);
  delay(100);
  leerBoton();
  digitalWrite(pinesLED [led], LOW);
}

void patronOnda() {
  for (int i = 0; i < numerosLED; i++) {
    leerBoton();
    digitalWrite(pinesLED [i], HIGH);
    delay(100);
    leerBoton();
    digitalWrite(pinesLED [i], LOW);
  }
  for (int i = numerosLED - 1; i >= 0; i--) {
    leerBoton();
    digitalWrite(pinesLED [i], HIGH);
    delay(100);
    leerBoton();
    digitalWrite(pinesLED [i], LOW);
  }
}