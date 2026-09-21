void ledAccendi(uint8_t colore, uint8_t luminosita) {
  analogWrite(colore, 255 - luminosita);
}

void ledSpegni(){
  analogWrite(LED_RED,   255);
  analogWrite(LED_GREEN, 255);
  analogWrite(LED_BLUE,  255);
}

void ledLampeggia(uint8_t colore, uint8_t luminosita) {
  for (int i = 0; i <= 2; i++) {
    ledAccendi(colore, luminosita);
    delay(50);
    ledSpegni();
    delay(50);
  }
}

void ledSleep(uint8_t colore, uint8_t luminosita, int velocitaDelay) {
  for (int i = luminosita; i >= 0; i--) {
    ledAccendi(colore, i); 
    delay(velocitaDelay); 
  }
}