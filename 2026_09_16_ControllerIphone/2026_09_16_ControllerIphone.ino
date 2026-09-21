
#include <bluefruit.h>
#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>

using namespace Adafruit_LittleFS_Namespace;

BLEDis bledis;
BLEHidAdafruit blehid;
BLEBas blebas;
BLEUart bleuart;

struct ImpostazioniProgetto { //ATTENZIONE NON ALTERARE L'ORDINE DEGLI ELEMENTI
  uint8_t tempoFiltroPulsante;
  uint16_t tempoCambioDispositivo;
  uint32_t tempoLasciatoAttivo;

  uint32_t attesaPerSleepA;
  uint32_t attesaConnessioneA;
  uint8_t potenzaSegnaleA;
  uint8_t luminositaLedA;

  uint32_t attesaPerSleepB;
  uint32_t attesaConnessioneB;
  uint8_t potenzaSegnaleB;
  uint8_t luminositaLedB;

  uint32_t refreshBatteria;
  float voltMaxBatteria;       // 4 byte (Float reale)
  float voltMinBatteria;       // 4 byte (Float reale)

  uint32_t loopDelayA;
  uint32_t loopDelayB;
};
ImpostazioniProgetto impostazioni; // 2. Creiamo la variabile globale che userai in tutto il codice

const byte pinPulsante = D6; //Pin a cui è collegato il pulsante/switch
const byte pinControlloSchedaB = D9; //Pin uscita per controllare il secondo dispositivo slave
const byte pinLivelloBatteria = A3;
const byte pinRicaricaBatteria = D2;
const byte pinCheckSchedaB = D10;

const char* PERCORSO_FILE = "config.bin";

bool dispDestinazione = 0; //Device in uso. 0 = iPhone (questo dispositivo), 1 = Mac (dispositivo slave)

volatile unsigned long clockPressionePulsante = 0; // "volatile" obbligatorio per le variabili usate dentro gli interrupt!
volatile uint8_t numeroCambiDispositivo = 0; // "volatile" obbligatorio per le variabili usate dentro gli interrupt!
volatile bool statoRicaricaBatteria = false;
volatile bool notificaRicarica = false;
volatile bool checkSchedaB = 0;
unsigned long clockInizioIncoerenza = 0;
unsigned long clockPingSchedaB = 0;
unsigned long clockInattivita;
unsigned long clockGiro;
unsigned long clockLedBle = 0;
unsigned long clockNoBle = 0;
unsigned long clockUltimaLetturaBatteria;
bool statoGestionePulsante = LOW;
bool statoConnessioneBle = false;
int32_t percentualeBatteria = 0; 
int8_t dbmArray[] = {-40, -20, -16, -12, -8, -4, 0, 4};//(0=-40dBm, 1=-20dBm, 2=-16dBm, 3=-12dBm, 4=-8dBm, 5=-4dBm, 6=0dBm, 7=+4dBm))
char statoSchedaA = ' ';
char statoSchedaB = ' ';
bool logErroreInviato = false;

void setup() {
  Serial.begin(115200);
  while (!Serial && (millis() < 1000)) {
    delay(10);
  }
  Serial_stampaln("**************************************"); 
  Serial_stampaln("Eccomi! Sono il Controller di iPhone");

  Serial1.setPins(D7, D8);
  Serial1.begin(115200);
  Serial1.setTimeout(10);

  InternalFS.begin();
  // resettaFileConfig(); // Solo se necessario

  ledSpegni();

  // 1. CARICA LE IMPOSTAZIONI PRIMA DI TUTTO IL RESTO!
  // In questo modo la potenza del segnale e i delay avranno subito i valori corretti.
  caricaImpostazioni(); 

  // 2. INIZIALIZZA IL BLUETOOTH CON I DATI GIÀ CARICATI
  Bluefruit.begin();
  Bluefruit.setTxPower(dbmArray[impostazioni.potenzaSegnaleA]); // Ora leggerà il valore reale (es: 0 o 4 dBm)
  Bluefruit.setName("C1-i");
  
  // Registrazione delle callback per la stabilità
  Bluefruit.Periph.setConnectCallback(connection_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnection_callback);

  // Inizializzazione dei servizi informativi
  bledis.setManufacturer("Giacomo");
  bledis.setModel("iPhoneKey");
  bledis.begin();  
  
  // Inizializzazione dei servizi dati
  blehid.begin();
  blebas.begin();  // Servizio livello batteria
  bleuart.begin(); // Servizio comunicazione app iPhone (UART)

  // 3. AVVIA L'ADVERTISING (che userà la funzione corretta sotto)
  startAdv();

  // Configurazione dei Pin e degli Interrupt
  pinMode(pinControlloSchedaB, OUTPUT);
  digitalWrite(pinControlloSchedaB, LOW);

  attachInterrupt(digitalPinToInterrupt(pinPulsante), gestisciInterruzione, CHANGE);
  nrf_gpio_cfg_input(g_ADigitalPinMap[pinPulsante], NRF_GPIO_PIN_NOPULL); 

  attachInterrupt(digitalPinToInterrupt(pinCheckSchedaB), gestisciRitornoSchedaB, CHANGE);
  nrf_gpio_cfg_input(g_ADigitalPinMap[pinCheckSchedaB], NRF_GPIO_PIN_NOPULL); 

  attachInterrupt(digitalPinToInterrupt(pinRicaricaBatteria), gestisciRicaricaBatteria, CHANGE);
  nrf_gpio_cfg_input(g_ADigitalPinMap[pinRicaricaBatteria], NRF_GPIO_PIN_NOPULL); 

  statoRicaricaBatteria = digitalRead(pinRicaricaBatteria);

  // Aggiorna la batteria inviando il dato iniziale sia al servizio BLE che alla scheda B
  aggiornaBatteria();

  ledLampeggia(LED_BLUE, impostazioni.luminositaLedA);
}

void loop() {
  clockGiro = millis();
  gestionePulsante();
  gestioneBatteria();
  gestioneComunicazioneSchedaB();
  gestioneConnessioneBLE();
  gestisciComunicazioneIPhone();
  gestioneInattivita();

  delay(impostazioni.loopDelayA);
  //Serial_stampaln(millis() - clockGiro); //Per misurare la velocità di esecuzione.
}