#include <HardwareSerial.h>
#include <Arduino.h>

#define RXD2 5
#define TXD2 4
#define DE_RE 3  // GPIO para controlar direção RS-485

HardwareSerial mySerial(1);  // UART2

void setup() {
  Serial.begin(115200);         // Debug via USB
  mySerial.begin(115200, SERIAL_8N1, RXD2, TXD2);

  pinMode(DE_RE, OUTPUT);
  digitalWrite(DE_RE, LOW);     // Inicialmente como receptor
  Serial.println("ESP32 pronto");
}

void loop() {
  if (mySerial.available()) {
    String msg = mySerial.readStringUntil('\n');
    Serial.print("Recebido via RS-485: ");
    Serial.println(msg);

    if (msg == "R") {
      Serial.println("Enviando pong...");
      digitalWrite(DE_RE, HIGH);  // Modo transmissão
      delay(1);  // Pequeno atraso para garantir direção correta

      mySerial.print("Pong \n");

      mySerial.flush();  // Aguarda envio terminar
      digitalWrite(DE_RE, LOW);  // Volta para recepção
    }
  }
}


