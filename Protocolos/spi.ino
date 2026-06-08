#include <SPI.h>
#include <SD.h>

File archivo;

void setup() {
  Serial.begin(9600);
  if (!SD.begin(10)) {
    Serial.println("Error SD");
    return;
  }
  Serial.println("SD inicializada");
  
  archivo = SD.open("prueba.txt", FILE_WRITE);
  if (archivo) {
    archivo.println("Hola desde SD");
    archivo.close();
    Serial.println("Escritura OK");
  } else {
    Serial.println("Error al abrir para escribir");
    return;
  }

  archivo = SD.open("prueba.txt");
  if (archivo) {
    Serial.println("Lectura del archivo:");
    while (archivo.available()) {
      Serial.write(archivo.read());
    }
    archivo.close();
    Serial.println();
    Serial.println("Lectura OK");
  } else {
    Serial.println("Error al abrir para leer");
  }
} 

void loop() {
}