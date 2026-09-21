
#include <bluefruit.h>

BLEDis bledis;
BLEHidGamepad blegamepad;
BLEBas blebas;
hid_gamepad_report_t gp;

struct ImpostazioniProgetto { //ATTENZIONE NON ALTERARE L'ORDINE DEGLI ELEMENTI
  uint32_t attesaPerSleepB;
  uint32_t attesaConnessioneB;
  uint8_t potenzaSegnaleB;
  uint8_t luminositaLedB;

  uint32_t loopDelayB;
};

ImpostazioniProgetto impostazioni; // 2. Creiamo la variabile globale che userai in tutto il codice

const byte pinIngresso = D4; //Pin di ingresso per il controllo
const byte pinCheck = D3;

volatile bool statoInterrupt = LOW; // "volatile" obbligatorio per le variabili usate dentro gli interrupt!

unsigned long clockGiro;
unsigned long clockInattivita;
unsigned long clockLedBle = 0;
unsigned long clockNoBle = 0;
unsigned long clockRichiestaAvvio = 0;
bool statoConnessioneBle = false;
bool statoGestioneIngresso = LOW;
bool statoConnessioneSchedaA = false;
char statoSchedaB = ' ';
int32_t percentualeBatteria = 1;
int8_t dbmArray[] = {-40, -20, -16, -12, -8, -4, 0, 4};

void setup() {
  // Valori default di base (Verranno sovrascritti quando Scheda A invierà i definitivi)
  impostazioni.loopDelayB = 10; 
  impostazioni.attesaPerSleepB = 15*60000; 
  impostazioni.attesaConnessioneB = 60000; 
  impostazioni.potenzaSegnaleB = 5;
  impostazioni.luminositaLedB = 255;

  Serial.begin(115200);
  while (!Serial && (millis() < 2000)) {
    delay(10);
  }

  Serial_stampaln("**************************************");
  Serial_stampaln("Eccomi! Sono il Controller di Mac");

  Serial1.setPins(D5, D6);
  Serial1.begin(115200);

  ledSpegni();

  attachInterrupt(digitalPinToInterrupt(pinIngresso), gestisciInterruzione, CHANGE);
  nrf_gpio_cfg_input(g_ADigitalPinMap[pinIngresso], NRF_GPIO_PIN_NOPULL); 

  pinMode(pinCheck, OUTPUT);
  digitalWrite(pinCheck, LOW);

  Bluefruit.begin();
  Bluefruit.setName("C1-M");
  Bluefruit.setTxPower(dbmArray[impostazioni.potenzaSegnaleB]);


  Bluefruit.Periph.setConnectCallback(connection_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnection_callback);

  bledis.setManufacturer("Giacomo");
  bledis.setModel("MacJoy");

  bledis.begin();
  blebas.begin();
  blegamepad.begin();

  avvioConnessioneBLE();

  startAdv();

  delay(500);

  avvioComunicazioneSchedaA();

  ledLampeggia(LED_GREEN, impostazioni.luminositaLedB);
}

void loop() {
  clockGiro = millis();
  gestioneIngresso();
  gestioneComunicazioneSchedaA();
  gestioneConnessioneBLE();
  gestioneInattivita();
  delay(impostazioni.loopDelayB); // System ON sleep automatico attivo durante il delay
}