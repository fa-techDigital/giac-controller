void gestioneComunicazioneSchedaB() {
  riceviComunicazioneSchedaB();

  // VERIFICA TOLLERANZA: Se l'interrupt ha rilevato un errore e il timer è attivo...
  if (dispDestinazione == 1 && !checkSchedaB && clockInizioIncoerenza > 0) {
    // Se il disallineamento persiste da più di 20 millisecondi...
    if (millis() - clockInizioIncoerenza > 20) {
      // Se non abbiamo ancora inviato il ping di emergenza per questo blocco...
      if (!logErroreInviato && (statoSchedaB == 'C' || statoSchedaB == 'U' || statoSchedaB == ' ')) {
        Serial_stampaln("Errore di incoerenza su Scheda B");
        Serial_stampaln("Invio un ping...");
        
        clockPingSchedaB = millis();
        Serial1.println("*");
        
        logErroreInviato = true; 
      }
    }
  }

  // RIPRISTINO REALE: Solo se checkSchedaB torna TRUE (scheda reinserita o allineata)
  if (checkSchedaB) {
    clockInizioIncoerenza = 0;
    logErroreInviato = false;
  }

  // TIMEOUT PING (Invariato)
  if (clockPingSchedaB > 0 && millis() - clockPingSchedaB > 1000){
    Serial_stampaln("Dopo 1 secondo Scheda B non ha risposto al ping. Chiudo la segnalazione.");
    clockPingSchedaB = 0;
  }
}

void gestisciRitornoSchedaB() { //Funzione dell'Interrupt
  if (dispDestinazione == 1) {
    checkSchedaB = (digitalRead(pinCheckSchedaB) == statoGestionePulsante);

    if (!checkSchedaB) {
      // Se c'è incoerenza e il timer è fermo, facciamo partire la tolleranza
      if (clockInizioIncoerenza == 0) {
        clockInizioIncoerenza = millis();
      }
    } else {
      // Se torna coerente, azzeriamo subito il timer di errore
      clockInizioIncoerenza = 0;
    }
  } else {
    checkSchedaB = true;
    clockInizioIncoerenza = 0;
  }
}

void riceviComunicazioneSchedaB(){
  if (Serial1.available() > 0) {
    char tipo = Serial1.read();

    if (tipo == 'R') {
      Serial_stampaln("Ricevuta richiesta generica da scheda B, le invio le impostazioni.");
      inviaImpostazioniASchedaB('R');
    }
    else if (tipo == 'D') {
      Serial_stampaln("Scheda B sta andando in autospegnimento.");
      statoSchedaB = 'D';
    }
    else if (tipo == 'A') {
        Serial_stampaln("Scheda B si è avviata e mi chiede le impostazioni. Le invio.");
        inviaImpostazioniASchedaB('A');
      if (statoSchedaB == ' ' || statoSchedaB == 'D') {
        statoSchedaB = 'A';
      }
    }
    else if (tipo == 'C') {
      Serial_stampaln("Scheda B è connessa tramite Bluetooth.");
      statoSchedaB = 'C';
    }
    else if (tipo == 'U') {
      Serial_stampaln("Scheda B NON è più connessa tramite Bluetooth.");
      statoSchedaB = 'U';
    }
    else if (tipo == '*') {
      unsigned long tempo;
      tempo = millis() - clockPingSchedaB;
      clockPingSchedaB = 0;
      Serial_stampa("Ricevuta risposta ping da Scheda B in ");
      Serial_stampa(tempo);
      Serial_stampaln(" ms");
    }
  }
}

void inviaImpostazioniASchedaB(char motivoInvio) {
  Serial1.print(motivoInvio);                        Serial1.print(",");
  Serial1.print(impostazioni.attesaPerSleepB);       Serial1.print(",");
  Serial1.print(impostazioni.attesaConnessioneB);    Serial1.print(",");
  Serial1.print(impostazioni.potenzaSegnaleB);       Serial1.print(","); // <-- AGGIUNTO INVIO SULLA UART
  Serial1.print(impostazioni.luminositaLedB);        Serial1.print(",");
  Serial1.print(percentualeBatteria);                Serial1.print(",");
  Serial1.print(impostazioni.loopDelayB);
  Serial1.print("\n"); 

  Serial_stampaln("Ho inviato a Scheda B tutto il blocco di informazioni...");
}

template <typename T>
void inviaASchedaB(String motivoInvio, T valore) {
  Serial1.print(motivoInvio); Serial1.print(",");
  Serial1.print(valore);
  Serial1.print("\n"); // Carattere fondamentale di FINE MESSAGGIO
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

void gestisciComunicazioneIPhone() {
  // Controlliamo se l'applicazione iPhone ha iniziato a inviare richieste
  if (bleuart.available() > 0) {
    
    // Leggiamo il primo carattere per capire cosa vuole l'app
    char tipo = bleuart.read();
    
    // --- CASO 1: L'APP RICHIEDE I DATI I DATI (RICHIESTA) ---
    if (tipo == 'R') {
      Serial_stampaln("iPhone richiede le impostazioni. Rispondo..");

      // Svuotiamo il buffer BLEUART
      while (bleuart.available() > 0) {
        bleuart.read(); 
      }

      // TRASMISSIONE: Segue esattamente l'ordine di stampaImpostazioni()
      bleuart.print("#");
      bleuart.print(impostazioni.tempoFiltroPulsante);     bleuart.print(","); 
      bleuart.print(impostazioni.tempoCambioDispositivo);   bleuart.print(","); 
      bleuart.print(impostazioni.tempoLasciatoAttivo);      bleuart.print(","); 
      bleuart.print(impostazioni.attesaPerSleepA);         bleuart.print(","); 
      bleuart.print(impostazioni.attesaConnessioneA);      bleuart.print(","); 
      bleuart.print(impostazioni.potenzaSegnaleA);         bleuart.print(","); // <-- AGGIUNTO
      bleuart.print(impostazioni.luminositaLedA);          bleuart.print(","); 
      bleuart.print(impostazioni.attesaPerSleepB);         bleuart.print(","); 
      bleuart.print(impostazioni.attesaConnessioneB);      bleuart.print(","); 
      bleuart.print(impostazioni.potenzaSegnaleB);         bleuart.print(","); // <-- AGGIUNTO
      bleuart.print(impostazioni.luminositaLedB);          bleuart.print(","); 
      bleuart.print(impostazioni.refreshBatteria);         bleuart.print(","); 
      bleuart.print(impostazioni.voltMaxBatteria, 2);      bleuart.print(","); 
      bleuart.print(impostazioni.voltMinBatteria, 2);      bleuart.print(","); 
      bleuart.print(percentualeBatteria);                  bleuart.print(","); 
      bleuart.print(impostazioni.loopDelayA);              bleuart.print(","); 
      bleuart.print(impostazioni.loopDelayB);                                  
      bleuart.print("\n");
    }
    // --- CASO 2: L'APP INVIA NUOVE IMPOSTAZIONI (WRITE VIA STRINGA) ---
    else if (tipo == 'N') {
      Serial_stampaln("iPhone sta inviando modifiche. Leggo la stringa...");

      bleuart.setTimeout(500); 
      String stringaRicevuta = bleuart.readStringUntil('\n');
      
      if (stringaRicevuta.length() > 0) {
        Serial_stampa("Stringa ricevuta dall'iPhone: ");
        Serial_stampaln(stringaRicevuta);
        
        // Contenitori temporanei a 32-bit ordinati esattamente come la struct
        unsigned long tDBNC, tCambio, tAttivo;
        unsigned long sA, aConnA, pSegnA, lumLedA; // <-- AGGIUNTO pSegnA
        unsigned long sB, aConnB, pSegnB, lumLedB; // <-- AGGIUNTO pSegnB
        unsigned long refBatt;
        long vMaxIntero, vMinIntero, percBatt;
        unsigned long lDelayA, lDelayB;
        
        // SPECULARE: Estraiamo esattamente 15 elementi corrispondenti alle variabili sopra
        int elementiEstratti = sscanf(stringaRicevuta.c_str(), "%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%ld,%ld,%ld,%lu,%lu",
                                    &tDBNC, &tCambio, &tAttivo, 
                                    &sA, &aConnA, &pSegnA, &lumLedA, 
                                    &sB, &aConnB, &pSegnB, &lumLedB, 
                                    &refBatt, &vMaxIntero, &vMinIntero, 
                                    &percBatt, &lDelayA, &lDelayB);
                                    
        if (elementiEstratti == 17) { // <-- ALLINEATO A 17
          impostazioni.tempoFiltroPulsante    = (uint8_t)tDBNC;
          impostazioni.tempoCambioDispositivo  = (uint16_t)tCambio;
          impostazioni.tempoLasciatoAttivo     = (uint32_t)tAttivo;
          impostazioni.attesaPerSleepA        = (uint32_t)sA;
          impostazioni.attesaConnessioneA     = (uint32_t)aConnA;
          impostazioni.potenzaSegnaleA        = (uint8_t)pSegnA; // <-- AGGIUNTO
          impostazioni.luminositaLedA         = (uint8_t)lumLedA;
          impostazioni.attesaPerSleepB        = (uint32_t)sB;
          impostazioni.attesaConnessioneB     = (uint32_t)aConnB;
          impostazioni.potenzaSegnaleB        = (uint8_t)pSegnB; // <-- AGGIUNTO
          impostazioni.luminositaLedB         = (uint8_t)lumLedB;
          impostazioni.refreshBatteria        = (uint32_t)refBatt;
          impostazioni.voltMaxBatteria        = (float)vMaxIntero / 100.0f;
          impostazioni.voltMinBatteria        = (float)vMinIntero / 100.0f;
          percentualeBatteria                 = (int32_t)percBatt;
          impostazioni.loopDelayA             = (uint32_t)lDelayA;
          impostazioni.loopDelayB             = (uint32_t)lDelayB;
          
          Serial_stampaln("SUCCESSO: Tutti i 17 elementi estratti e variabili aggiornate.");

          // Salviamo nella Flash interna (InternalFS)
          salvaImpostazioniInMemoria();
          ledLampeggia(LED_BLUE, impostazioni.luminositaLedA);
          
          // TRASMISSIONE BINARIA ALLA SCHEDA B
          if (statoSchedaB != 'D'){ // Se scheda B non è in DeepSleep
            inviaImpostazioniASchedaB('N');
            Serial_stampaln("Nuova configurazione inoltrata alla Scheda B.");
          }

          // Applica subito la potenza BLE aggiornata alla Scheda A!
          // Mappa l'intero inviato (es. 0=-40dBm, 1=-20dBm, 2=-16dBm, 3=-12dBm, 4=-8dBm, 5=-4dBm, 6=0dBm, 7=+4dBm)
          if(impostazioni.potenzaSegnaleA <= 7) {
              Bluefruit.setTxPower(dbmArray[impostazioni.potenzaSegnaleA]);
          }
          aggiornaBatteria();
          
        } else {
          Serial_stampa("ERRORE: Stringa corrotta. Elementi estratti: ");
          Serial_stampa(elementiEstratti);
          Serial_stampaln(" su 15 richiesti. Operazione annullata.");
        }
      } else {
        Serial_stampaln("ERRORE: Stringa vuota o timeout ricezione da iPhone.");
      }
      
      while (bleuart.available() > 0) {
        bleuart.read();
      }
    }
  }
}

void stampaImpostazioni() {
  Serial_stampaln("Questi i dati presenti nella struct");
  Serial_stampa("tempoFiltroPulsante: "); Serial_stampa(impostazioni.tempoFiltroPulsante); Serial_stampaln(" ms");
  Serial_stampa("tempoCambioDispositivo: "); Serial_stampa(impostazioni.tempoCambioDispositivo); Serial_stampaln(" ms");
  Serial_stampa("tempoLasciatoAttivo: "); Serial_stampa(impostazioni.tempoLasciatoAttivo); Serial_stampaln(" ms");
  Serial_stampa("attesaPerSleepA: "); Serial_stampa(impostazioni.attesaPerSleepA); Serial_stampaln(" ms");
  Serial_stampa("attesaConnessioneA: "); Serial_stampa(impostazioni.attesaConnessioneA); Serial_stampaln(" ms");
  Serial_stampa("potenzaSegnaleA: "); Serial_stampa(dbmArray[impostazioni.potenzaSegnaleA]); Serial_stampaln(" dBm");
  Serial_stampa("luminositaLedA: "); Serial_stampaln(impostazioni.luminositaLedA);
  Serial_stampa("attesaPerSleepB: "); Serial_stampa(impostazioni.attesaPerSleepB); Serial_stampaln(" ms");
  Serial_stampa("attesaConnessioneB: "); Serial_stampa(impostazioni.attesaConnessioneB); Serial_stampaln(" ms");
  Serial_stampa("potenzaSegnaleB: "); Serial_stampa(dbmArray[impostazioni.potenzaSegnaleB]); Serial_stampaln(" dBm");
  Serial_stampa("luminositaLedB: "); Serial_stampaln(impostazioni.luminositaLedB);
  Serial_stampa("refreshBatteria: "); Serial_stampa(impostazioni.refreshBatteria); Serial_stampaln(" ms");
  Serial_stampa("voltMaxBatteria: "); Serial_stampa(impostazioni.voltMaxBatteria); Serial_stampaln(" V");
  Serial_stampa("voltMinBatteria: "); Serial_stampa(impostazioni.voltMinBatteria); Serial_stampaln(" V");
  Serial_stampa("percentualeBatteria: "); Serial_stampa(percentualeBatteria); Serial_stampaln(" %");
  Serial_stampa("loopDelayA: "); Serial_stampa(impostazioni.loopDelayA); Serial_stampaln(" ms");
  Serial_stampa("loopDelayB: "); Serial_stampa(impostazioni.loopDelayB); Serial_stampaln(" ms");
}