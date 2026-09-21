void startAdv(void) {
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addAppearance(BLE_APPEARANCE_HID_GAMEPAD);
  Bluefruit.Advertising.addService(blegamepad);
  Bluefruit.Advertising.addName();
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds
}

void avvioConnessioneBLE() {
  //Impostando questi timer lascio detto a gestioneConnessioneBLE che la scheda non è ancora connessa
  statoConnessioneBle = false;
  clockNoBle = millis(); 
  clockLedBle = millis();
}

void gestioneConnessioneBLE() {
  // Se il Mac è regolarmente connesso via BLE, non serve calcolare nessun timer
  if (statoConnessioneBle) {
    return;
  }

  // ---- DA QUI IN POI GESTIAMO SOLO LE TEMPISTICHE DA DISCONNESSO ----

  // 1. Lampeggio di avviso (Led Rosso) ogni 5 secondi
  if (millis() - clockLedBle >= 5000) {
    clockLedBle = millis();
    ledLampeggia(LED_RED, impostazioni.luminositaLedB);
  }
  
  // 2. Controllo scadenza timer per andare in Deep Sleep se disconnesso troppo a lungo
  if (clockNoBle > 0 && (millis() - clockNoBle >= impostazioni.attesaConnessioneB)) {
    Serial_stampaln("Tempo di attesa connessione BLE terminato.");
    deepSleep();
  }
}

void connection_callback(uint16_t conn_handle) {
  statoConnessioneBle = true;
  clockNoBle = 0; 
  
  Serial_stampaln("Connesso al Mac.");
  Serial1.println("C");

  // Forza il pairing sicuro obbligatorio per macOS
  BLEConnection* conn = Bluefruit.Connection(conn_handle);
  if (conn) {
    // Se il Mac ha già una vecchia chiave, questo comando forza il rinnovo sicuro
    conn->requestPairing(); 
  }
}


void disconnection_callback(uint16_t conn_handle, uint8_t reason) {
  statoConnessioneBle = false;
  clockLedBle = millis();
  clockNoBle = millis(); // Fissa il momento esatto in cui inizia l'attesa disconnessione
  
  Serial_stampa("Sono disconnesso da BLE... Motivo: 0x");
  Serial_stampaln(String(reason, HEX));
  Serial1.println("U"); // Notifica a Scheda A che il Mac si è disconnesso
  if (statoSchedaB != 'D') {
    Serial_stampaln("Riavvio Advertising.");
    startAdv(); 
  }
}