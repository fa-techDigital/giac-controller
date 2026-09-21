void gestioneBatteria() {
  // Gestione della notifica seriale nel flusso normale del loop (evita blocchi hardware)
  if (notificaRicarica) {
    notificaRicarica = false;
    inviaASchedaB("C", statoRicaricaBatteria ? HIGH : LOW);
  }

  // Timer di lettura della percentuale
  uint32_t valoreRefresh = statoRicaricaBatteria ? 30000 : impostazioni.refreshBatteria;
  if (clockGiro - clockUltimaLetturaBatteria > valoreRefresh) {
    clockUltimaLetturaBatteria = clockGiro;
    aggiornaBatteria();
  }
}

void aggiornaBatteria() {
  // Riferimento interno a 3.0V e risoluzione 12-bit
  analogReference(AR_INTERNAL_3_0);
  analogReadResolution(12);
  
  long sum = 0;
  for (int i = 0; i < 10; i++) sum += analogRead(pinLivelloBatteria);
  float voltage = ((sum / 10.0) * 3.0 / 4096.0) * 2.0;
  int percentage = (int)((voltage - impostazioni.voltMinBatteria) * 100.0 / (impostazioni.voltMaxBatteria - impostazioni.voltMinBatteria));
  uint8_t nuovoValore = (uint8_t)constrain(percentage, 1, 100);
  if (nuovoValore != percentualeBatteria){
    percentualeBatteria = nuovoValore;
    blebas.write(percentualeBatteria);
    inviaASchedaB("B", percentualeBatteria);
    Serial_stampa("Aggiornamento livello batteria è ");
    Serial_stampaln(percentualeBatteria);
  }
}

void gestisciRicaricaBatteria() {
  if (digitalRead(pinRicaricaBatteria) && !statoRicaricaBatteria){
    statoRicaricaBatteria = true;
    notificaRicarica = true;
  } else if (!digitalRead(pinRicaricaBatteria) && statoRicaricaBatteria){
    statoRicaricaBatteria = false;
    notificaRicarica = true;
  }
}