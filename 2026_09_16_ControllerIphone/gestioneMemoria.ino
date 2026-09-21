// FUNZIONE PER SALVARE I DATI
void salvaImpostazioniInMemoria() {
  // RIMOZIONE DI resettaFileConfig(); 
  // Non cancelliamo più il file per proteggere i dati e non usurare la Flash!

  // Apriamo il file in scrittura. Se non esiste, LittleFS lo creerà la prima volta.
  // Se esiste, lo tiene aperto senza modificarne la dimensione.
  File file = InternalFS.open(PERCORSO_FILE, FILE_O_WRITE);
  
  if (file) {
    // Portiamo tassativamente il puntatore all'inizio del file (byte 0).
    // In questo modo i nuovi 42 byte sovrascriveranno esattamente i vecchi.
    file.seek(0); 
    
    // Scrittura della struct in Flash
    file.write((uint8_t*)&impostazioni, sizeof(impostazioni));
    file.close(); // Chiusura immediata per rendere effettivo il salvataggio
    
    Serial_stampaln("Impostazioni sovrascritte con successo nella Flash!");
    stampaImpostazioni();
  } else {
    Serial_stampaln("Errore nell'apertura del file config.bin in scrittura!");
  }
}

// FUNZIONE PER CARICARE I DATI ALL'AVVIO
void caricaImpostazioni() {
  if (InternalFS.exists(PERCORSO_FILE)) {
    File file = InternalFS.open(PERCORSO_FILE, FILE_O_READ);
    if (file) {
      file.read((uint8_t*)&impostazioni, sizeof(impostazioni));
      file.close();
      Serial_stampaln("Impostazioni caricate correttamente dalla memoria Flash.");
      stampaImpostazioni();
      return; 
    }
  }


  // --- VALORI DI EMERGENZA / FABBRICA ---
  Serial_stampaln("Configurazione non trovata. Carico i valori di emergenza standard...");
  
  impostazioni.tempoFiltroPulsante = 50; // (ms) tempo debouce pulsante (pulizia interferenze, il pulsante è stato effettivamente premuto)
  impostazioni.tempoCambioDispositivo = 5000; // (ms) tempo di chiusura pulsante per cambiare dispositivo (tra A Mester e B Slave)
  impostazioni.tempoLasciatoAttivo = 10000; // (ms) Tempo massimo di pressione per andare in deepSleep
  impostazioni.attesaPerSleepA = 15*60000; // (ms) tempo attesa per spegnimento automatico inattività scheda A
  impostazioni.attesaConnessioneA = 60000;
  impostazioni.potenzaSegnaleA = 5;
  impostazioni.luminositaLedA = 255;
  impostazioni.attesaPerSleepB = 15*60000; // (ms) tempo attesa per spegnimento automatico inattività scheda B
  impostazioni.attesaConnessioneB = 60000; // (s) tempo attesa per spegnimento automatico BLE non connesso scheda B
  impostazioni.potenzaSegnaleB = 5;
  impostazioni.luminositaLedB = 255;
  impostazioni.refreshBatteria = 30*60000; // Tempo refresh livello batteria
  impostazioni.voltMaxBatteria = 4.2; // Volt batteria 100%
  impostazioni.voltMinBatteria = 3.5; // Volt batteria 1%
  impostazioni.loopDelayA = 10; //Tempo di pausa del ciclo principale di scheda A
  impostazioni.loopDelayB = 10; //Tempo di pausa del ciclo principale di scheda B

  // Salva subito i valori di default per i prossimi avvii
  salvaImpostazioniInMemoria();
}

void resettaFileConfig() {
  Serial_stampaln("Rimuovo il file config.bin...");
  InternalFS.remove("config.bin"); // Elimina fisicamente il file sballato
}