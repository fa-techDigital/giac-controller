void gestioneConnessioneBLE() {
  // Se la scheda è connessa, non dobbiamo fare calcoli sui timer o lampeggi
  if (statoConnessioneBle) {
    clockNoBle = 0;
    return; 
  }
  else {
    // ---- DA QUI IN POI GESTIAMO SOLO IL CASO "DISCONNESSO" ----

    // 1. Lampeggio di avviso rosso ogni 5 secondi
    if (millis() - clockLedBle > 5000) {
      ledLampeggia(LED_RED, impostazioni.luminositaLedA);
      clockLedBle = millis();
    }
    // Se la Scheda B è confermata in Deep Sleep ('D') e Scheda A è disconnessa
    // può andare subito in Deep Sleep
    if (statoSchedaB == 'D') {
      Serial_stampaln("Scheda B è già spenta. Mi spengo anche io.");
      deepSleep();
      return;
    }
    // GESTIONE DEL TIMER SE LA SCHEDA B È ANCORA SVEGLIA MA DISCONNESSA ('U' o ' ')
    if (statoSchedaB == 'U' || statoSchedaB == ' ') {
      // Fai partire il timer coordinato solo se non è già attivo
      if (clockNoBle == 0) {
        clockNoBle = clockGiro;
        Serial_stampaln("Sia Scheda A che Scheda B sono disconnesse. Avvio il timer per spegnermi.");
      }
      
      // Se scade il tempo di attesa standard prima che la Scheda B mandi la 'D'...
      if (clockGiro - clockNoBle >= impostazioni.attesaConnessioneA) {
        Serial_stampaln("Tempo di attesa per la connessione Bluetooth terminato. Mi spengo.");
        deepSleep();
      }
    }
    else if (statoSchedaB == 'C' && clockNoBle > 0) {
      // Se la Scheda B nel frattempo si è connessa al Mac, congeliamo il timer di spegnimento della A.
      // L'utente sta usando il Mac, quindi la Scheda A deve rimanere sveglia a fare da ponte!
      clockNoBle = 0;
      Serial_stampaln("Scheda B è connessa. Resetto il mio timer di autospegnimento.");
    }
  }
}

void startAdv(void) {
  Bluefruit.Advertising.clearData();
  Bluefruit.ScanResponse.clearData();

  // Pacchetto principale (Max 31 byte) - Mettiamo solo l'essenziale
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addAppearance(0x03C7);
  Bluefruit.Advertising.addService(blehid); // Servizio Tastiera
  Bluefruit.Advertising.addService(blebas); // Servizio Batteria
  Bluefruit.Advertising.addName();          // Nome della scheda

  // PACCHETTO DI RISPOSTA ALLA SCANSIONE (Scan Response)
  // Spostiamo qui l'UART che è pesantissima (16 byte), liberando il pacchetto principale!
  Bluefruit.ScanResponse.addService(bleuart);

  Bluefruit.Advertising.restartOnDisconnect(true);
  
  // Intervalli secondo linee guida Apple...
  Bluefruit.Advertising.setInterval(32, 244);//20 ms per Fast e 152.5 ms per Slow
  Bluefruit.Advertising.setFastTimeout(30);//Impone passaggio da modalità Fast a Slow dopo 30 secondi

  // ... o in alternativa Gemini dice di forzare così per aumentare gli advertise:
  //Bluefruit.Advertising.setInterval(32, 32);  

  Bluefruit.Advertising.start(0);//0 vuol dire che non smetterà mai di inviare l'Advertise.  
}


void connection_callback(uint16_t conn_handle) {
  statoConnessioneBle = true;
  clockNoBle = 0; // Resetta immediatamente il timer di deep sleep
  Serial_stampaln("Sono connesso tramite Bluetooth. Resetto il timer.");
  
  // Richiede il pairing sicuro all'iPhone
  BLEConnection* conn = Bluefruit.Connection(conn_handle);
  if (conn) {
    conn->requestPairing();
  }
}

void disconnection_callback(uint16_t conn_handle, uint8_t reason) {
  statoConnessioneBle = false;
  clockLedBle = millis(); // Fissa il tempo per il primo lampeggio rosso
  
  Serial_stampa("Non sono più connesso tramite Bluetooth... Motivo: 0x");
  Serial_stampaln(String(reason, HEX));
  ledLampeggia(LED_RED, impostazioni.luminositaLedA);
  if (statoSchedaA != 'D') {
    Serial_stampaln("Riavvio Advertising.");
    startAdv(); 
  }
}