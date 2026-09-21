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
  analogReference(AR_INTERNAL_3_0);
  analogReadResolution(12);

  // 1. LETTURA A VUOTO (FONDAMENTALE PER I 100K+100K)
  // Serve a pre-caricare l'ADC dello XIAO prima del ciclo reale
  analogRead(pinLivelloBatteria);
  delay(5); // Diamo tempo elettrico di stabilizzarsi

  long sum = 0;
  for (int i = 0; i < 10; i++) {
    sum += analogRead(pinLivelloBatteria);
    delay(2); // 2. PAUSA TRA LE LETTURE per assecondare l'alta impedenza
  }

  float voltage = ((sum / 10.0) * 3.0 / 4096.0) * 2.0;
  int percentage = (int)((voltage - impostazioni.voltMinBatteria) * 100.0 / (impostazioni.voltMaxBatteria - impostazioni.voltMinBatteria));
  uint8_t nuovoValore = (uint8_t)constrain(percentage, 1, 100);

  // 3. LOGICA ANTI-RIMBALZO SOFTWARE
  // Creiamo una variabile che si ricorda il valore anche dopo lo Sleep
  static uint8_t ultimaPercentualeFissa = 0;

  // Al primissimo avvio assoluto inizializziamo la memoria interna
  if (ultimaPercentualeFissa == 0) {
    ultimaPercentualeFissa = percentualeBatteria; 
  }

  // Se la batteria NON è in carica, impediamo matematicamente al valore di salire!
  if (!statoRicaricaBatteria && nuovoValore > ultimaPercentualeFissa) {
    nuovoValore = ultimaPercentualeFissa; // Rifiuta il rimbalzo chimico (es: il passaggio da 38 a 41)
  }

  if (nuovoValore != percentualeBatteria){
    percentualeBatteria = nuovoValore;
    ultimaPercentualeFissa = nuovoValore; // Aggiorna il punto di riferimento più basso
    
    blebas.write(percentualeBatteria);
    inviaASchedaB(" B", percentualeBatteria);
    Serial_stampa(" Aggiornamento livello batteria è ");
    Serial_stampaln(percentualeBatteria);
  }
}
/*
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
}*/

void gestisciRicaricaBatteria() {
  if (digitalRead(pinRicaricaBatteria) && !statoRicaricaBatteria){
    statoRicaricaBatteria = true;
    notificaRicarica = true;
  } else if (!digitalRead(pinRicaricaBatteria) && statoRicaricaBatteria){
    statoRicaricaBatteria = false;
    notificaRicarica = true;
  }
}