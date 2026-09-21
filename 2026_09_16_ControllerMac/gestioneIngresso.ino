void gestioneIngresso() {
  bool connessioneBle = Bluefruit.connected();
  if (statoInterrupt == HIGH && statoGestioneIngresso == LOW) {
    if (connessioneBle) {
      gp.buttons = GAMEPAD_BUTTON_0; // Seleziona il pulsante 1
      blegamepad.report(&gp); //Riporta la selezione
    }
    digitalWrite(pinCheck, HIGH);
    ledAccendi(connessioneBle ? LED_GREEN : LED_RED, impostazioni.luminositaLedB);
    statoGestioneIngresso = HIGH;
  }
  else if (statoInterrupt == LOW && statoGestioneIngresso == HIGH) {
    if (connessioneBle) {
      gp.buttons = 0; //Seleziona nessun pulsante
      blegamepad.report(&gp); //riporta la selezione
    }
    digitalWrite(pinCheck, LOW);
    ledSpegni();
    statoGestioneIngresso = LOW;
    clockInattivita = millis();
  }
  
}

// 3. LA FUNZIONE DI INTERRUPT (Deve essere cortissima e veloce)
void gestisciInterruzione() {
  if (digitalRead(pinIngresso) == LOW) {
    statoInterrupt = LOW;
    digitalWrite(pinCheck, LOW);
  } else {
    statoInterrupt = HIGH;
    digitalWrite(pinCheck, HIGH);
  }
}