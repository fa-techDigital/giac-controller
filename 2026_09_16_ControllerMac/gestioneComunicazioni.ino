void avvioComunicazioneSchedaA(){
  Serial_stampaln("Comunico il mio avvio a scheda A chiedendo le impostazioni...");
  Serial1.println("A");
  clockRichiestaAvvio = millis();

  gestioneComunicazioneSchedaA();
}

void gestioneComunicazioneSchedaA(){
  if (!statoConnessioneSchedaA && (millis() - clockRichiestaAvvio > 500)) {
    clockRichiestaAvvio = millis();
    Serial_stampaln("Sollecito invio impostazioni a Scheda A...");
    Serial1.println("A");
  }

  if (Serial1.available() > 0) {
  
    // Leggiamo l'intera riga fino al carattere di fine messaggio
    String stringaRicevuta = Serial1.readStringUntil('\n');
    
    // Variabile per estrarre il tipo di comunicazione
    char tipo = '\0';

    // Usiamo sscanf per catturare solo il primo carattere (tipo) e verificare che la stringa non sia vuota
    if (sscanf(stringaRicevuta.c_str(), "%c", &tipo) < 1) {
      return; // Se la stringa è corrotta o vuota, esce per sicurezza
    }



    if (tipo == 'A' || tipo == 'N'|| tipo == 'R') { // in questi casi arriva un pacchetto completo di impostazioni
    Serial_stampa("Ricevuto comunicazione tipo ");
    Serial_stampa(tipo);
    Serial_stampaln(", aggiorno le variabili coinvolte...");      

      // Variabili temporanee di appoggio richieste da sscanf per l'estrazione
      char motivoCompleto;
      unsigned int attesaSleep, attesaConn, potB, lum, batt, delayB;

      // sscanf estrae tutti e 7 i parametri separati da virgola
      int letti = sscanf(stringaRicevuta.c_str(), "%c,%u,%u,%u,%u,%u,%u", 
                        &motivoCompleto, &attesaSleep, &attesaConn, &potB, &lum, &batt, &delayB);

      if (letti == 7) { // <-- VERIFICA REQUISITO A 7
        uint8_t vecchiaPotenza = impostazioni.potenzaSegnaleB;
        uint8_t vecchiaBatteria = percentualeBatteria;

        impostazioni.attesaPerSleepB     = attesaSleep;
        impostazioni.attesaConnessioneB  = attesaConn;
        impostazioni.potenzaSegnaleB     = potB;
        impostazioni.luminositaLedB      = lum;
        percentualeBatteria              = batt;
        impostazioni.loopDelayB          = delayB;
        
        if (impostazioni.potenzaSegnaleB != vecchiaPotenza) {
            Bluefruit.setTxPower(dbmArray[impostazioni.potenzaSegnaleB]);
        }
        if (percentualeBatteria   != vecchiaBatteria) {
          blebas.write(percentualeBatteria);
        }
        statoConnessioneSchedaA = true;
      }
      stampaImpostazioni();
    }

    if (tipo == 'B'){
      // Se il tipo è 'B', riutilizziamo sscanf per estrarre solo il valore singolo della batteria dopo la virgola
      char motivoBatteria;
      unsigned int nuovaBatteria;
      
      if (sscanf(stringaRicevuta.c_str(), "%c,%u", &motivoBatteria, &nuovaBatteria) == 2) {
        percentualeBatteria = (uint8_t) nuovaBatteria; // Aggiorna il valore nella struct
      }

      //ricevuto aggiornamento batteria, lo invio al Mac tramite BLE
      Serial_stampaln("Invio aggiornamento percentuale batteria al Mac.");
      blebas.write(percentualeBatteria);
    }
    
    if (tipo == 'D') {
      Serial_stampaln("Scheda A mi comunica che sta andando in DeepSleep, la seguo...");
      deepSleep();
    }

    if (tipo == 'C') {
      // Batteria in carica
    }

    if (tipo == '*') {
      Serial1.println("*");
      Serial_stampaln("Scheda A mi ha inviato un ping ed ho risposto.");
    }
  }
}

void stampaImpostazioni() {
  Serial_stampaln("Questi i dati presenti nella struct");
  Serial_stampa("attesaPerSleepB: "); Serial_stampa(impostazioni.attesaPerSleepB); Serial_stampaln(" ms");
  Serial_stampa("attesaConnessioneB: "); Serial_stampa(impostazioni.attesaConnessioneB); Serial_stampaln(" ms");
  Serial_stampa("potenzaSegnaleB: "); Serial_stampa(dbmArray[impostazioni.potenzaSegnaleB]); Serial_stampaln(" dBm");
  Serial_stampa("luminositaLedB: "); Serial_stampaln(impostazioni.luminositaLedB);
  Serial_stampa("percentualeBatteria: "); Serial_stampa(percentualeBatteria); Serial_stampaln(" %");
  Serial_stampa("loopDelayB: "); Serial_stampa(impostazioni.loopDelayB); Serial_stampaln(" ms");
}

template <typename T>
void Serial_stampaln(T stringa) {
  if (Serial) { 
    Serial.println((String)stringa);
  }
}

template <typename T>
void Serial_stampa(T stringa) {
  if (Serial) { 
    Serial.print((String)stringa);
  }
}