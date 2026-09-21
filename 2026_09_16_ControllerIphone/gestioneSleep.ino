void gestioneInattivita() {
  if (clockGiro >= clockInattivita + impostazioni.attesaPerSleepA) {
    Serial_stampaln("Limite tempo di inattività raggiunto...");
    deepSleep();
  }
}

void deepSleep() {
  statoSchedaA = 'D';
  // 1. Interrompi la trasmissione dei pacchetti pubblicitari (Advertising)
  Bluefruit.Advertising.stop();
  Serial_stampaln("Sto andando in DeepSleep...");
  Serial1.println("D"); //Avverto scheda B che vado in DeepSleep

  // 2. Forza la disconnessione se c'è un dispositivo centrale collegato (es. lo smartphone)
  if (Bluefruit.connected()) {
    // Ottieni l'ID della connessione attiva e disconnettila
    uint16_t conn_handle = Bluefruit.connHandle();
    BLEConnection* connection = Bluefruit.Connection(conn_handle);
    if (connection != NULL) {
      connection->disconnect();
      delay(100); // Dai il tempo hardware al chip di inviare il pacchetto di disconnessione
    }
  }
  
  // 3. Chiudi le comunicazioni seriali per spegnere i moduli UART interni (UARTE)
  Serial.end();
  Serial1.end();

  ledSleep(LED_BLUE, impostazioni.luminositaLedA, 10);
  ledSpegni();

  InternalFS.end(); 

  // Configurazione DINAMICA del pin per il risveglio:
  if (digitalRead(pinPulsante) == LOW) {
    // Se il pulsante è rimasto bloccato su attivo (LOW) digli di svegliarsi solo quando viene sbloccato (HIGH)
    nrf_gpio_cfg_sense_input(g_ADigitalPinMap[pinPulsante], NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_SENSE_HIGH);
  } else {
    // Se il pulsante è a riposo (HIGH), digli di svegliarsi normalmente quando viene attivato (LOW)
    nrf_gpio_cfg_sense_input(g_ADigitalPinMap[pinPulsante], NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_SENSE_LOW);
  }

    // Scollega il pull-up per azzerare i consumi parassiti
  nrf_gpio_cfg_input(g_ADigitalPinMap[pinCheckSchedaB], NRF_GPIO_PIN_NOPULL);

  // 5. Ora il chip è isolato e pronto: entra in Deep Sleep (System OFF)
  NRF_POWER->SYSTEMOFF = 1;
}