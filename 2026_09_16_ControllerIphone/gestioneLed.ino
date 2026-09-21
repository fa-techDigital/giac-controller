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

/*
void attivaLampeggio(uint8_t colore, uint8_t luminosita) {
  ledStato.coloreAttivo = colore;
  ledStato.luminositaAct = luminosita;
  ledStato.conteggioLampeggi = 0;
  ledStato.fase = 1; // Stato 1: Il LED si accende
  ledStato.clockAnimazione = millis();
  
  ledAccendi(ledStato.coloreAttivo, ledStato.luminositaAct);
}

void gestisciLed() {
  if (ledStato.fase == 0) return; // Se l'animazione è inattiva, esce immediatamente

  // Controlla se sono passati i 50ms asincroni
  if (millis() - ledStato.clockAnimazione >= 50) {
    ledStato.clockAnimazione = millis(); 

    if (ledStato.fase == 1) { 
      // Era acceso: lo spegne ed entra nella fase di pausa
      ledSpegni();
      ledStato.fase = 2; 
    } 
    else if (ledStato.fase == 2) { 
      // Era spento: incrementa il contatore ed esegue il controllo
      ledStato.conteggioLampeggi++;
      
      if (ledStato.conteggioLampeggi >= 3) {
        ledStato.fase = 0; // Ha completato i 3 lampeggi: ferma l'animazione definitivamente
      } else {
        ledAccendi(ledStato.coloreAttivo, ledStato.luminositaAct);
        ledStato.fase = 1; // Riaccende e torna alla fase 1
      }
    }
  }
}*/