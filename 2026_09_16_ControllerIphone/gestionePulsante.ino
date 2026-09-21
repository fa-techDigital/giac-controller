void gestionePulsante() {
  // 1. SE IL PULSANTE NON È PREMUTO, GESTIAMO IL RILASCIO IMMEDIATO (Asincrono)
  if (clockPressionePulsante == 0) {
    if (statoGestionePulsante) {
      rilasciaComando();
      statoGestionePulsante = LOW;
      clockInattivita = millis();
    }
    return;
  }

  // 2. FILTRO DEBOUNCE ASINCRONO: Se il pulsante è attivo ma non sono ancora passati i 50ms,
  // ignoriamo il resto del codice e lasciamo girare il loop liberamente!
  if (millis() - clockPressionePulsante < impostazioni.tempoFiltroPulsante) {
    return; 
  }

  // 3. VERIFICA FALSI POSITIVI: Se dopo il debounce il pin è tornato HIGH, era un falso contatto
  if (digitalRead(pinPulsante) == HIGH) {
    clockPressionePulsante = 0;
    numeroCambiDispositivo = 0;
    return;
  }

  // 4. DA QUI IN POI IL SEGNALE È STABILE (ESEGUITO SOLO SE PREMUTO oltre il tempo di debounce)
  // Gestione spegnimento di emergenza (Deep Sleep)
  if (millis() - clockPressionePulsante > impostazioni.tempoLasciatoAttivo) {
    deepSleep();
    return; 
  }

  // Gestione cambio dispositivo automatico (Pressione prolungata)
  unsigned long attesaProssimoCambio = impostazioni.tempoCambioDispositivo * (numeroCambiDispositivo + 1);
  if (millis() - clockPressionePulsante >= attesaProssimoCambio) {
    rilasciaComando();
    dispDestinazione = !dispDestinazione;
    numeroCambiDispositivo++;
    inviaComando();
  }

  // Attivazione iniziale del comando al primo click valido
  if (!statoGestionePulsante) {
    inviaComando();
    statoGestionePulsante = HIGH;
  }
}

void inviaComando(){
  if (dispDestinazione == 0) {
    bool connessioneBle = Bluefruit.connected();
    if (connessioneBle) {
      uint8_t tasto_f24[6] = {115, 0, 0, 0, 0, 0};
      blehid.keyboardReport(BLE_CONN_HANDLE_INVALID, 0, tasto_f24);
    }
    ledAccendi(connessioneBle ? LED_BLUE : LED_RED, impostazioni.luminositaLedA);
    return; 
  }
  else if (dispDestinazione == 1) {
    digitalWrite(pinControlloSchedaB, HIGH);
    gestisciRitornoSchedaB();
    return;
  }
}

void rilasciaComando(){
  if (dispDestinazione == 0) {
    bool connessioneBle = Bluefruit.connected();
    if (connessioneBle) {
      uint8_t rilascio_tasto[6] = {0, 0, 0, 0, 0, 0};
      blehid.keyboardReport(BLE_CONN_HANDLE_INVALID, 0, rilascio_tasto);
    }
    ledSpegni();
    return; 
  }
  else if (dispDestinazione == 1) {
    digitalWrite(pinControlloSchedaB, LOW);
    gestisciRitornoSchedaB();
    return;
  }
}

// 3. LA FUNZIONE DI INTERRUPT (Deve essere cortissima e veloce)
void gestisciInterruzione() {
  if (digitalRead(pinPulsante) == LOW) {
    clockPressionePulsante = millis(); 
  } else {
    clockPressionePulsante = 0;
    numeroCambiDispositivo = 0;
  }
}