const int LED_PIN = 13;
byte estadoLed = 0;
int contador = 0;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(9600);
  
  byte a = 5;  
  byte b = 3; 

  Serial.println("Operaciones Bitwise:");
  Serial.print("AND: ");
  Serial.println(a & b, BIN);
  Serial.print("OR: ");
  Serial.println(a | b, BIN);
  Serial.print("XOR: ");
  Serial.println(a ^ b, BIN);
  Serial.print("NOT a: ");
  Serial.println((byte)~a, BIN);
  Serial.print("Shift izquierda a: ");
  Serial.println(a << 1, BIN);
  Serial.print("Shift derecha a: ");
  Serial.println(a >> 1, BIN);

  estadoLed = estadoLed | (1 << 0);
}

void loop() {
  estadoLed = estadoLed ^ 0b00000001;
  if ((estadoLed & 1) == 1) {
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
  }
  delay(500);

  contador = (contador + 1) % 8;
  Serial.print("Shift actual: ");
  Serial.println(1 << contador, BIN);
  delay(500);
}