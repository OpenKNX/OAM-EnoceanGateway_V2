#include "EnoceanModule.h"
#include "EnoceanProfils.h"
#include "knxprod.h"

#define KDEBUG_Received

#define proc_crc8(u8CRC, u8Data) (u8CRC8Table[u8CRC ^ u8Data])

uint8_t EnoceanModule::u8CRC8Table[256] = {
    0x00, 0x07, 0x0e, 0x09, 0x1c, 0x1b, 0x12, 0x15, 0x38, 0x3f, 0x36, 0x31, 0x24, 0x23, 0x2a, 0x2d, 0x70, 0x77, 0x7e,
    0x79, 0x6c, 0x6b, 0x62, 0x65, 0x48, 0x4f, 0x46, 0x41, 0x54, 0x53, 0x5a, 0x5d, 0xe0, 0xe7, 0xee, 0xe9, 0xfc, 0xfb,
    0xf2, 0xf5, 0xd8, 0xdf, 0xd6, 0xd1, 0xc4, 0xc3, 0xca, 0xcd, 0x90, 0x97, 0x9e, 0x99, 0x8c, 0x8b, 0x82, 0x85, 0xa8,
    0xaf, 0xa6, 0xa1, 0xb4, 0xb3, 0xba, 0xbd, 0xc7, 0xc0, 0xc9, 0xce, 0xdb, 0xdc, 0xd5, 0xd2, 0xff, 0xf8, 0xf1, 0xf6,
    0xe3, 0xe4, 0xed, 0xea, 0xb7, 0xb0, 0xb9, 0xbe, 0xab, 0xac, 0xa5, 0xa2, 0x8f, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9d,
    0x9a, 0x27, 0x20, 0x29, 0x2e, 0x3b, 0x3c, 0x35, 0x32, 0x1f, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0d, 0x0a, 0x57, 0x50,
    0x59, 0x5e, 0x4b, 0x4c, 0x45, 0x42, 0x6f, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7d, 0x7a, 0x89, 0x8e, 0x87, 0x80, 0x95,
    0x92, 0x9b, 0x9c, 0xb1, 0xb6, 0xbf, 0xb8, 0xad, 0xaa, 0xa3, 0xa4, 0xf9, 0xfe, 0xf7, 0xf0, 0xe5, 0xe2, 0xeb, 0xec,
    0xc1, 0xc6, 0xcf, 0xc8, 0xdd, 0xda, 0xd3, 0xd4, 0x69, 0x6e, 0x67, 0x60, 0x75, 0x72, 0x7b, 0x7c, 0x51, 0x56, 0x5f,
    0x58, 0x4d, 0x4a, 0x43, 0x44, 0x19, 0x1e, 0x17, 0x10, 0x05, 0x02, 0x0b, 0x0c, 0x21, 0x26, 0x2f, 0x28, 0x3d, 0x3a,
    0x33, 0x34, 0x4e, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5c, 0x5b, 0x76, 0x71, 0x78, 0x7f, 0x6A, 0x6d, 0x64, 0x63, 0x3e,
    0x39, 0x30, 0x37, 0x22, 0x25, 0x2c, 0x2b, 0x06, 0x01, 0x08, 0x0f, 0x1a, 0x1d, 0x14, 0x13, 0xae, 0xa9, 0xa0, 0xa7,
    0xb2, 0xb5, 0xbc, 0xbb, 0x96, 0x91, 0x98, 0x9f, 0x8a, 0x8D, 0x84, 0x83, 0xde, 0xd9, 0xd0, 0xd7, 0xc2, 0xc5, 0xcc,
    0xcb, 0xe6, 0xe1, 0xe8, 0xef, 0xfa, 0xfd, 0xf4, 0xf3};

// Konstruktor: keine eigene Initialisierung nötig, Rest übernimmt die Basisklasse.
EnoceanModule::EnoceanModule() {}

// Liefert den Modulnamen "Enocean" für Log-/Konsolenausgaben.
const std::string EnoceanModule::name() { return "Enocean"; }

// Liefert bewusst einen leeren String, damit das Modul nicht separat in der Versionsausgabe der Konsole erscheint.
const std::string EnoceanModule::version() {
  // hides the module in the version output on the console, because the firmware
  // version is sufficient.
  return "";
}

// Merkt sich den zu verwendenden UART-Stream für die Kommunikation mit dem EnOcean-Transceiver.
void EnoceanModule::initSerial(Stream &serial) { _serial = &serial; }

// Initialisiert die UART-Schnittstelle zum EnOcean-Transceiver und legt bei vorhandener Konfiguration die Kanäle an.
void EnoceanModule::setup(bool configured) {
  logInfoP("Setup0");
  logIndentUp();

  OPENKNX_ENO_SERIAL.setRX(OPENKNX_ENO_UART_RX_PIN);
  OPENKNX_ENO_SERIAL.setTX(OPENKNX_ENO_UART_TX_PIN);

  OPENKNX_ENO_SERIAL.begin(57600);
  initSerial(OPENKNX_ENO_SERIAL);

  SERIAL_PORT.begin(9600);

  if (configured) {
    // setupCustomFlash();
    setupChannels();
  }

  begin();

  logIndentDown();
}

// Erzeugt für jeden in der ETS sichtbaren Kanal eine EnoceanChannel-Instanz und ruft deren setup() auf.
void EnoceanModule::setupChannels() {
  for (uint8_t i = 0; i < ParamENO_VisibleChannels; i++) {
    _channels[i] = new EnoceanChannel(i);
    _channels[i]->setup();
  }
  _channelsReady = true;
}

// void EnoceanModule::setupCustomFlash() {
//   logDebugP("initialize Enocean flash");
//   OpenKNX::Flash::Driver _EnoceanStorage;
// #ifdef ARDUINO_ARCH_ESP32
//   _EnoceanStorage.init("Enocean");
// #else
//   _EnoceanStorage.init("Enocean", Enocean_FLASH_OFFSET, Enocean_FLASH_SIZE);
// #endif
//
//   logTraceP("write Enocean data");
//   // _EnoceanStorage.writeByte(0, 0x11);
//   // _EnoceanStorage.writeWord(1, 0xFFFF);
//   // _EnoceanStorage.writeInt(3, 6666666);
//   // for (size_t i = 0; i < 4095; i++)
//   // {
//   //     _EnoceanStorage.writeByte(i, i);
//   // }
//   // _EnoceanStorage.commit();
//
//   logDebugP("read Enocean data");
//   logIndentUp();
//   // logHexDebugP(_EnoceanStorage.flashAddress(), 4095);
//   // logDebugP("byte: %02X", _EnoceanStorage.readByte(0)); // UINT8
//   // logDebugP("word: %i", _EnoceanStorage.readWord(1));   // UINT16
//   // logDebugP("int: %i", _EnoceanStorage.readInt(3));     // UINT32
//
//   logIndentDown();
// }

// Zyklischer Aufruf: verarbeitet empfangene Telegramme und ruft Round-Robin die Kanal-Loops auf.
// Der Empfang läuft auch ohne ETS-Konfiguration (Testbetrieb: Telegramme werden nur geloggt, Bedienung über die
// Konsole, siehe processCommand()).
void EnoceanModule::loop(bool configured) {
  if (delayCheck(_timer1, 30000)) {
    logInfoP("Loop0");
    logInfoP("CH %u", ParamENO_VisibleChannels);

    _timer1 = millis();
  }

  task();

  // Teach-In-Fenster abgelaufen (falls der TCM kein CO_LRN_MODE_DISABLED gesendet hat)
  if (_teachChannel && delayCheck(_teachStart, teachWindowMs())) {
    logInfoP("Teach-in channel %u: time window expired", _teachChannel);
    stopTeachIn();
  }

  if (!configured || ParamENO_VisibleChannels == 0)
    return;

  uint8_t processed = 0;
  do {
    _channels[_currentChannel]->loop();
  } while (openknx.freeLoopIterate(ParamENO_VisibleChannels, _currentChannel, processed));
}

#ifdef OPENKNX_DUALCORE

// Setup für den zweiten RP2040-Kern (Dual-Core); aktuell nur eine Startverzögerung.
void EnoceanModule::setup1(bool configured) {
  delay(1000);
  // logInfoP("Setup1");
}

// Loop für den zweiten RP2040-Kern (Dual-Core); aktuell ohne Funktion.
void EnoceanModule::loop1(bool configured) {
  if (delayCheck(_timer2, 7200)) {
    // logInfoP("Loop1");
    _timer2 = millis();
  }
}
#endif

// Empfängt und verarbeitet ein einzelnes EnOcean-Telegramm vom UART und protokolliert periodisch ein Lebenszeichen.
void EnoceanModule::task() {
  static uint32_t lastTime = 0;
  if (millis() - lastTime > 30000) {
    logInfoP("Alive, runtime: %u", lastTime);
    lastTime = millis();
  }

  // EnOcean IN -> KNX OUT
  u8RetVal = ENOCEAN_NO_RX_TEL;
  u8RetVal = uart_getPacket(&m_Pkt_st, (uint16_t)DATBUF_SZ);
  getEnOceanMSG(u8RetVal, &m_Pkt_st);
}

// Einmalige Initialisierung: liest die Transceiver-Base-ID aus (Basis-ID-Vergleich und Repeater-Funktion sind auskommentiert).
void EnoceanModule::begin() {
  if (isInited)
    return;

  m_Pkt_st.u8DataBuffer = &u8datBuf[0];

  //****************** Read, Check & Set EnOcean Gateway Base ID
  //************************************
  // communicates via Enocean UART channel
  // 1.) read base-ID
  delay(2000);
  logInfoP("read BaseID");
  bool baseIdValid = readBaseId(&lui8_BaseID_p[0]);

  // 2.) compare old base-ID with the new ID (nur wenn die aktuelle Base-ID auch gelesen werden konnte)
  if (baseIdValid && ParamENO_SetBaseIdFunc)
  {
    if (checkBaseID())
    { // old != new
      logInfoP("Base-ID: OLD != NEW -> change! ");
      uint8_t newBaseId[BASEID_BYTES];
      getConfiguredBaseId(newBaseId);
      if (isValidBaseId(newBaseId))
        setBaseId(newBaseId);
      else
        logErrorP("Configured Base-ID %02X%02X%02X%02X out of range", newBaseId[0], newBaseId[1], newBaseId[2], newBaseId[3]);
      // 3.) read Base-ID again and print it out
      baseIdValid = readBaseId(&lui8_BaseID_p[0]);
    }
    else // old == new
    {
      logDebugP("Base-ID: OLD == NEW -> NO change! ");
    }
  }

  if (baseIdValid)
    logInfoP("Base-ID: %02X %02X %02X %02X", lui8_BaseID_p[0], lui8_BaseID_p[1], lui8_BaseID_p[2], lui8_BaseID_p[3]);
  else
    logErrorP("Base-ID could not be read from transceiver");

  //****************** Repeater Function ************************************
  // Ohne ETS-Konfiguration (alle Parameter 0) wird die Einstellung des Transceivers nicht verändert.
  if (knx.configured())
    setRepeaterFunc(ParamENO_RepeaterFunc, ParamENO_RepeaterLevel);
  // prüft ob Änderungen umgesetzt wurden
  readRepeaterFunc();

  // Transceiver-Info für Konsole und ETS (EURID, Versionen)
  readVersion();

  if (!knx.configured())
    logInfoP("Not configured: test mode (receive + log only, use console command 'eno')");

  isInited = true;
}

// Setzt die in der ETS parametrierte Base-ID aus den 8 Nibble-Parametern (Id0..Id7) zu 4 Bytes zusammen.
void EnoceanModule::getConfiguredBaseId(uint8_t out8BaseId[BASEID_BYTES])
{
  out8BaseId[0] = (ParamENO_Id0 << 4) | ParamENO_Id1;
  out8BaseId[1] = (ParamENO_Id2 << 4) | ParamENO_Id3;
  out8BaseId[2] = (ParamENO_Id4 << 4) | ParamENO_Id5;
  out8BaseId[3] = (ParamENO_Id6 << 4) | ParamENO_Id7;
}

// Prüft, ob die Base-ID im erlaubten Bereich 0xFF800000 … 0xFFFFFF80 liegt und die unteren 7 Bit 0 sind
// (128 Sende-IDs ab Base-ID), ESP3-Spec Kap. 2.5.9.
bool EnoceanModule::isValidBaseId(const uint8_t baseId[BASEID_BYTES])
{
  return baseId[0] == 0xFF && baseId[1] >= 0x80 && (baseId[3] & 0x7F) == 0;
}

// Liefert true, wenn die vom Transceiver gelesene Base-ID von der in der ETS parametrierten abweicht.
bool EnoceanModule::checkBaseID()
{
  uint8_t configuredBaseId[BASEID_BYTES];
  getConfiguredBaseId(configuredBaseId);
  return memcmp(lui8_BaseID_p, configuredBaseId, BASEID_BYTES) != 0;
}

// Schreibt die übergebene Base-ID in den Transceiver (CO_WR_IDBASE) und wartet auf die Antwort.
void EnoceanModule::setBaseId(const uint8_t *fui8_BaseID_p)
{
  PACKET_SERIAL_TYPE_ lRdBaseIDPkt_st;

  uint8_t lu8SndBuf[5];

  lu8SndBuf[0] = u8CO_WR_IDBASE;
  lu8SndBuf[1] = fui8_BaseID_p[0];
  lu8SndBuf[2] = fui8_BaseID_p[1];
  lu8SndBuf[3] = fui8_BaseID_p[2];
  lu8SndBuf[4] = fui8_BaseID_p[3];

  lRdBaseIDPkt_st.u16DataLength = 0x0005;
  lRdBaseIDPkt_st.u8OptionLength = 0x00;
  lRdBaseIDPkt_st.u8Type = u8RORG_COMMON_COMMAND;
  lRdBaseIDPkt_st.u8DataBuffer = &lu8SndBuf[0];

  logInfoP("Sending telegram (set base ID).");

  if (ENOCEAN_OK == uart_sendPacket(&lRdBaseIDPkt_st))
  {
    logInfoP("Receiving telegram (set base ID).");
    u8RetVal = waitForResponse(ENOCEAN_RESPONSE_TIMEOUT);

    switch (u8RetVal)
    {
    case ENOCEAN_OK:
    {
      {
        std::string hexStr;
        char hexByte[4];
        for (int i = 0; i < m_Pkt_st.u16DataLength + (uint16_t)m_Pkt_st.u8OptionLength; i++)
        {
          // SERIAL_PORT.print("%X", m_Pkt_st.u8DataBuffer[i]);
          sprintf(hexByte, "%02X ", m_Pkt_st.u8DataBuffer[i]);
          hexStr += hexByte;
        }
        logInfoP("Data: %s", hexStr.c_str());
      }

      switch (m_Pkt_st.u8Type)
      {
      case u8RESPONSE:
      {
        switch (m_Pkt_st.u8DataBuffer[0])
        {
        case 0x00:
          logInfoP("Received Response: RET_OK");
          break;
        case 0x02:
          logInfoP("Received Response: RET_NOT_SUPPORTED");
          break;
        case 0x82:
          logInfoP("Received Response: FLASH_HW_ERROR");
          break;
        case 0x90:
          logInfoP("Received Response: BASEID_OUT_OF_RANGE");
          break;
        case 0x91:
          logInfoP("Received Response: BASEID_MAX_REACHED");
          break;

        default:
          break;
        }
      }
      break;
      default:
      {
        /// SERIAL_PORT.println("%X", m_Pkt_st.u8Type);
        logInfoP("Wrong packet type. Expected response. Received: %u", m_Pkt_st.u8Type);
      }
      }
    }
    break;
    case ENOCEAN_NO_RX_TEL:
      logInfoP("ERROR Receiving telegram (set base ID).");
      break;
    default:
    {
      logInfoP("set receiving base ID");
    }
    } // ENDE SWITCH
  }
}

// Aktiviert/deaktiviert den Lernmodus des Transceivers für den angegebenen Kanal (CO_WR_LEARNMODE, ESP3-Spec
// Kap. 2.5.25) und wartet auf die Antwort. timeoutMs: Dauer des Lernmodus (0 = Default 60 s). Liefert true bei RET_OK.
bool EnoceanModule::activateLearnMode(uint8_t channel, uint8_t enable, uint32_t timeoutMs) {
  PACKET_SERIAL_TYPE_ lLearnModePkt_st;

  // Data: COMMAND Code (1) + Enable (1) + Timeout (4, big-endian) = 6 Bytes
  // Optional Data: Channel (1 Byte)
  uint8_t lu8SndBuf[7];
  lu8SndBuf[0] = u8CO_WR_LEARNMODE;
  lu8SndBuf[1] = enable; // Start Learn mode = 1, End Learn mode = 0
  lu8SndBuf[2] = (uint8_t)(timeoutMs >> 24);
  lu8SndBuf[3] = (uint8_t)(timeoutMs >> 16);
  lu8SndBuf[4] = (uint8_t)(timeoutMs >> 8);
  lu8SndBuf[5] = (uint8_t)timeoutMs;
  lu8SndBuf[6] = channel; // 0..0xFD = Kanal absolut, 0xFE = vorheriger, 0xFF = nächster Kanal (relativ)

  lLearnModePkt_st.u16DataLength = 6;
  lLearnModePkt_st.u8OptionLength = 1;
  lLearnModePkt_st.u8Type = u8RORG_COMMON_COMMAND;
  lLearnModePkt_st.u8DataBuffer = &lu8SndBuf[0];

  logDebugP("Sending telegram (learn mode %s), Channel: %u", enable ? "on" : "off", channel);

  if (ENOCEAN_OK != uart_sendPacket(&lLearnModePkt_st))
    return false;

  u8RetVal = waitForResponse(ENOCEAN_RESPONSE_TIMEOUT);
  if (u8RetVal != ENOCEAN_OK) {
    logDebugP("ERROR Receiving telegram (learn mode).");
    return false;
  }

  logDebugP("Learn mode Return Code: %u", m_Pkt_st.u8DataBuffer[0]);
  return m_Pkt_st.u8DataBuffer[0] == 0x00; // RET_OK
}

// Fragt beim Transceiver den aktuellen Lernmodus-Status ab (CO_RD_LEARNMODE, ESP3-Spec Kap. 2.5.26) und wartet auf
// die Antwort. fu8_Enable_p: Lernmodus aktiv (0/1). fu8_Channel_p: Kanal, für den der Lernmodus aktiv ist (0..0xFD).
// Liefert true, wenn der Status gelesen werden konnte.
bool EnoceanModule::readLearnMode(uint8_t *fu8_Enable_p, uint8_t *fu8_Channel_p) {
  PACKET_SERIAL_TYPE_ lRdLearnModePkt_st;

  uint8_t lu8SndBuf = u8CO_RD_LEARNMODE;

  lRdLearnModePkt_st.u16DataLength = 0x0001;
  lRdLearnModePkt_st.u8OptionLength = 0x00;
  lRdLearnModePkt_st.u8Type = u8RORG_COMMON_COMMAND;
  lRdLearnModePkt_st.u8DataBuffer = &lu8SndBuf;

  logDebugP("Sending telegram (read learn mode).");

  if (ENOCEAN_OK != uart_sendPacket(&lRdLearnModePkt_st))
    return false;

  u8RetVal = waitForResponse(ENOCEAN_RESPONSE_TIMEOUT);
  if (u8RetVal != ENOCEAN_OK) {
    logDebugP("ERROR Receiving telegram (read learn mode).");
    return false;
  }

  // Response: RET_CODE (1) + Learn Mode Status (1), Optional Data: Channel (1)
  if (m_Pkt_st.u8DataBuffer[0] != 0x00 || m_Pkt_st.u16DataLength < 2) {
    logDebugP("Read learn mode Return Code: %u", m_Pkt_st.u8DataBuffer[0]);
    return false;
  }

  *fu8_Enable_p = m_Pkt_st.u8DataBuffer[1];
  *fu8_Channel_p = (m_Pkt_st.u8OptionLength >= 1) ? m_Pkt_st.u8DataBuffer[2] : 0;
  logDebugP("Learn mode Enable: %u, Channel: %u", *fu8_Enable_p, *fu8_Channel_p);
  return true;
}

// Verarbeitet einen Einlern-Befehl (KO SetTeachChannel oder Konsole "eno teach"):
// 1..30 = Teach-In für diesen Kanal starten, 255 = Teach-In beenden, 100 = aktiven Kanal melden.
void EnoceanModule::handleTeachCommand(uint8_t value) {
  if (value > 0 && value <= ENO_ChannelCount) {
    uint8_t result = startTeachIn(value);
    if (result == ENO_FP_ERR_ALREADY_LEARNED)
      logInfoP("Channel %u already learned, delete it first ('eno teach del %u')", value, value);
    else if (result == ENO_FP_ERR_BUSY)
      logInfoP("Teach-in for channel %u is running, stop it first", _teachChannel);
  } else if (value == 255) {
    if (_teachChannel)
      stopTeachIn();
    else
      disableLearnMode();
  } else if (value == 100) {
    reportTeachChannel();
  }
}

// Dauer des Teach-In-Fensters laut ETS (Allgemein, Sekunden), ohne Konfiguration 60 s.
uint32_t EnoceanModule::teachWindowMs() {
  uint8_t seconds = ParamENO_TeachWindow;
  return (seconds ? seconds : 60) * 1000UL;
}

// Startet das Teach-In für einen Kanal (1..30): Lernmodus im TCM 615 für denselben Kanal öffnen (nötig für
// verschlüsselte Geräte) und das Teach-In-Fenster im Gateway starten. Ein bereits eingelernter Kanal wird nicht
// überschrieben. Liefert einen ENO_FP_*-Code.
uint8_t EnoceanModule::startTeachIn(uint8_t channel) {
  if (channel < 1 || channel > ENO_ChannelCount)
    return ENO_FP_ERR_CHANNEL;
  if (_teach[channel - 1].valid)
    return ENO_FP_ERR_ALREADY_LEARNED;
  if (_teachChannel && _teachChannel != channel)
    return ENO_FP_ERR_BUSY;

  _teachErrorChannel = 0;
  _teachErrorCause = 0;
  _teachChannel = channel;
  _teachStart = millis();
  if (!activateLearnMode(channel, true, teachWindowMs()))
    logErrorP("Transceiver did not confirm learn mode, only unencrypted devices can be learned");

  logInfoP("Teach-in channel %u started (%lu s)", channel, teachWindowMs() / 1000);
  setIsTeachChannel(channel);
  return ENO_FP_OK;
}

// Beendet ein laufendes Teach-In (Lernmodus im TCM aus, IsTeachChannel = 0).
void EnoceanModule::stopTeachIn() {
  if (_teachChannel == 0)
    return;
  activateLearnMode(_teachChannel, false);
  _teachChannel = 0;
  setIsTeachChannel(0);
}

// true, wenn die ID bereits einem Kanal zugeordnet ist (ETS-Parameter oder eingelernt).
bool EnoceanModule::isIdAssigned(const uint8_t id[4]) {
  for (uint8_t i = 0; i < ENO_ChannelCount; i++) {
    if (_teach[i].valid && memcmp(_teach[i].id, id, 4) == 0)
      return true;
    if (_channelsReady && i < ParamENO_VisibleChannels && _channels[i]->matchesDeviceId(id))
      return true;
  }
  return false;
}

// Prüft während eines Teach-In-Fensters, ob ein Funktelegramm ein Lerntelegramm eines noch unbekannten Senders ist,
// und ordnet es dann dem Kanal zu: 4BS und 1BS mit LRN-Bit = 0, UTE-Anfrage (VLD-Geräte) sowie bei unverschlüsselten
// Tastern (RPS) das erste Tastentelegramm. Liefert true, wenn das Telegramm eingelernt wurde.
bool EnoceanModule::tryTeachIn(PACKET_SERIAL_TYPE_ *f_Pkt_st) {
  // Kanal bereits belegt (z.B. verschachtelter Empfang, während die Teach-In-Antwort gesendet wird)
  if (_teach[_teachChannel - 1].valid)
    return false;

  uint8_t id[4];
  if (!extractSenderId(f_Pkt_st, id) || isIdAssigned(id))
    return false;

  uint8_t *d = f_Pkt_st->u8DataBuffer;
  switch (d[0]) {
  case u8RORG_4BS:
    // DB0 = d[4]: Bit 3 LRN (0 = Lerntelegramm), Bit 7 LRN-Typ (1 = mit EEP, "Variante 3")
    if (d[4] & 0x08)
      return false;
    if (d[4] & 0x80)
      storeTeachIn(id, u8RORG_4BS, d[1] >> 2, ((d[1] & 0x03) << 5) | (d[2] >> 3), 0);
    else
      storeTeachIn(id, u8RORG_4BS, 0, 0, 0);
    return true;
  case u8RORG_1BS:
    // DB0 Bit 3 LRN (0 = Lerntelegramm), Profil D5-00-01
    if (d[1] & 0x08)
      return false;
    storeTeachIn(id, u8RORG_1BS, 0x00, 0x01, 0);
    return true;
  case u8RORG_UTE:
    // DB6 = d[1] Bit 3-0: Kommando (0 = Teach-In-Anfrage); EEP: DB0 = d[7] R-ORG, DB1 = d[6] FUNC, DB2 = d[5] TYPE
    if ((d[1] & 0x0F) != 0 || f_Pkt_st->u16DataLength < 13)
      return false;
    storeTeachIn(id, d[7], d[6], d[5], 0);
    return true;
  case u8RORG_RPS:
    // Taster ohne Lerntelegramm: erstes Telegramm eines unbekannten Senders (EEP unbekannt)
    storeTeachIn(id, u8RORG_RPS, 0, 0, 0);
    return true;
  default:
    return false;
  }
}

// Speichert die eingelernte ID für den Kanal des laufenden Teach-In (nur im RAM, danach finishTeachIn() aufrufen).
void EnoceanModule::storeTeachIn(const uint8_t id[4], uint8_t rorg, uint8_t func, uint8_t type, uint8_t flags) {
  if (_teachChannel == 0)
    return;
  EnoceanTeachEntry &entry = _teach[_teachChannel - 1];
  entry.valid = 1;
  entry.flags = flags;
  memcpy(entry.id, id, 4);
  entry.eep[0] = rorg;
  entry.eep[1] = func;
  entry.eep[2] = type;
  logInfoP("Channel %u learned: ID %02X%02X%02X%02X, EEP %02X-%02X-%02X%s", _teachChannel, id[0], id[1], id[2], id[3], rorg,
           func, type, (flags & ENO_TEACH_FLAG_SECURE) ? " (secure)" : "");
}

// Sichert die eingelernte ID im Flash und beendet das Teach-In. Getrennt von storeTeachIn(), damit bidirektionale
// Geräte ihre Teach-In-Antwort vorher bekommen (Flash-Schreiben und Lernmodus-Befehl kosten Zeit).
void EnoceanModule::finishTeachIn() {
  openknx.flash.save(true);
  stopTeachIn();
}

// Löscht die eingelernte ID eines Kanals (1..30); verschlüsselte Geräte werden auch aus der Secure-Link-Tabelle des
// TCM entfernt.
void EnoceanModule::deleteTeachIn(uint8_t channel) {
  if (channel < 1 || channel > ENO_ChannelCount)
    return;
  EnoceanTeachEntry &entry = _teach[channel - 1];
  if (entry.valid && (entry.flags & ENO_TEACH_FLAG_SECURE))
    deleteSecureDevice(entry.id);
  memset(&entry, 0, sizeof(entry));
  if (_teachErrorChannel == channel)
    _teachErrorChannel = 0;
  openknx.flash.save(true);
  logInfoP("Channel %u: teach-in deleted", channel);
}

// Entfernt ein Gerät aus der Secure-Link-Tabelle des TCM (CO_WR_SECUREDEVICE_DEL, ESP3-Spec Kap. 2.5.28).
void EnoceanModule::deleteSecureDevice(const uint8_t id[4]) {
  PACKET_SERIAL_TYPE_ lPkt_st;
  uint8_t lu8SndBuf[6] = {u8CO_WR_SECUREDEVICE_DEL, id[0], id[1], id[2], id[3], 0x00}; // Optional: Inbound-Tabelle

  lPkt_st.u16DataLength = 5;
  lPkt_st.u8OptionLength = 1;
  lPkt_st.u8Type = u8RORG_COMMON_COMMAND;
  lPkt_st.u8DataBuffer = lu8SndBuf;

  if (uart_sendPacket(&lPkt_st) != ENOCEAN_OK || waitForResponse(ENOCEAN_RESPONSE_TIMEOUT) != ENOCEAN_OK)
    logErrorP("Secure device %02X%02X%02X%02X could not be deleted", id[0], id[1], id[2], id[3]);
  else
    logInfoP("Secure device %02X%02X%02X%02X deleted, return code %u", id[0], id[1], id[2], id[3], m_Pkt_st.u8DataBuffer[0]);
}

// Liefert die eingelernte ID eines Kanals (Index 0..29), wenn vorhanden.
bool EnoceanModule::learnedId(uint8_t channelIndex, uint8_t out8Id[4]) {
  if (channelIndex >= ENO_ChannelCount || !_teach[channelIndex].valid)
    return false;
  memcpy(out8Id, _teach[channelIndex].id, 4);
  return true;
}

// Flash: Versionsbyte + eingelernte IDs aller Kanäle
uint16_t EnoceanModule::flashSize() { return 1 + ENO_ChannelCount * sizeof(EnoceanTeachEntry); }

void EnoceanModule::writeFlash() {
  openknx.flash.writeByte(ENO_FLASH_VERSION);
  for (uint8_t i = 0; i < ENO_ChannelCount; i++)
    openknx.flash.write((uint8_t *)&_teach[i], sizeof(EnoceanTeachEntry));
}

void EnoceanModule::readFlash(const uint8_t *data, const uint16_t size) {
  if (size < flashSize() || data[0] != ENO_FLASH_VERSION) {
    logInfoP("No teach-in data in flash");
    return;
  }
  memcpy(_teach, data + 1, ENO_ChannelCount * sizeof(EnoceanTeachEntry));
  for (uint8_t i = 0; i < ENO_ChannelCount; i++)
    if (_teach[i].valid)
      logInfoP("Channel %u learned: ID %02X%02X%02X%02X", i + 1, _teach[i].id[0], _teach[i].id[1], _teach[i].id[2], _teach[i].id[3]);
}

// ETS-Buttons (Script EnoceanModule.script.js) über FunctionProperty 160/7. Antworten haben max. 10 Byte.
bool EnoceanModule::processFunctionProperty(uint8_t objectIndex, uint8_t propertyId, uint8_t length, uint8_t *data, uint8_t *resultData, uint8_t &resultLength) {
  if (objectIndex != ENO_FP_OBJECT || propertyId != ENO_FP_PROPERTY)
    return false;

  resultLength = 1;
  if (!knx.configured()) {
    resultData[0] = ENO_FP_ERR_NOT_CONFIGURED;
    return true;
  }
  if (length < 1) {
    resultData[0] = ENO_FP_ERR_COMMAND;
    return true;
  }

  uint8_t channel = length >= 2 ? data[1] : 0;
  bool channelValid = channel >= 1 && channel <= ENO_ChannelCount;
  logDebugP("ETS function property: command %u, channel %u", data[0], channel);

  switch (data[0]) {
  case ENO_FP_CMD_STATUS: {
    // [0] Ergebnis, [1] Status, [2..5] ID, [6..8] EEP, [9] Status RUNNING: Restzeit s / LEARNED: Flags / ERROR: Ursache
    if (!channelValid) {
      resultData[0] = ENO_FP_ERR_CHANNEL;
      return true;
    }
    EnoceanTeachEntry &entry = _teach[channel - 1];
    memset(resultData, 0, 10);
    resultLength = 10;
    if (entry.valid) {
      resultData[1] = ENO_TEACH_LEARNED;
      memcpy(&resultData[2], entry.id, 4);
      memcpy(&resultData[6], entry.eep, 3);
      resultData[9] = entry.flags;
    } else if (_teachChannel == channel) {
      resultData[1] = ENO_TEACH_RUNNING;
      uint32_t elapsed = millis() - _teachStart;
      resultData[9] = elapsed < teachWindowMs() ? (teachWindowMs() - elapsed) / 1000 : 0;
    } else if (_teachErrorChannel == channel) {
      resultData[1] = ENO_TEACH_ERROR;
      resultData[9] = _teachErrorCause;
    } else {
      resultData[1] = ENO_TEACH_FREE;
    }
    return true;
  }
  case ENO_FP_CMD_START: {
    // [0] Ergebnis, bei ENO_FP_ERR_ALREADY_LEARNED [1..4] eingelernte ID, bei ENO_FP_ERR_BUSY [1] laufender Kanal
    uint8_t result = startTeachIn(channel);
    resultData[0] = result;
    if (result == ENO_FP_ERR_ALREADY_LEARNED) {
      memcpy(&resultData[1], _teach[channel - 1].id, 4);
      resultLength = 5;
    } else if (result == ENO_FP_ERR_BUSY) {
      resultData[1] = _teachChannel;
      resultLength = 2;
    }
    return true;
  }
  case ENO_FP_CMD_DELETE:
    if (!channelValid) {
      resultData[0] = ENO_FP_ERR_CHANNEL;
      return true;
    }
    if (_teachChannel == channel)
      stopTeachIn();
    deleteTeachIn(channel);
    resultData[0] = ENO_FP_OK;
    return true;
  case ENO_FP_CMD_BASEID:
    // [0] Ergebnis, [1..4] Base-ID, [5] verbleibende Schreibzyklen (0xFF = unbegrenzt), [6..9] EURID
    if (!readBaseId(lui8_BaseID_p)) {
      resultData[0] = ENO_FP_ERR_TRANSCEIVER;
      return true;
    }
    resultData[0] = ENO_FP_OK;
    memcpy(&resultData[1], lui8_BaseID_p, 4);
    resultData[5] = _baseIdRemainingWrites;
    memcpy(&resultData[6], _eurid, 4);
    resultLength = 10;
    return true;
  case ENO_FP_CMD_LEARN:
    // [0] Ergebnis: ENO_FP_OK oder ENO_FP_ERR_PROFILE, wenn der Kanal nicht sendet
    if (!channelValid || !_channelsReady || channel > ParamENO_VisibleChannels) {
      resultData[0] = ENO_FP_ERR_CHANNEL;
      return true;
    }
    resultData[0] = _channels[channel - 1]->sendLearnTelegram() ? ENO_FP_OK : ENO_FP_ERR_PROFILE;
    return true;
  case ENO_FP_CMD_VERSION:
    // [0] Ergebnis, [1..4] App-Version, [5..8] API-Version
    if (!_versionValid) {
      resultData[0] = ENO_FP_ERR_TRANSCEIVER;
      return true;
    }
    resultData[0] = ENO_FP_OK;
    memcpy(&resultData[1], _appVersion, 4);
    memcpy(&resultData[5], _apiVersion, 4);
    resultLength = 9;
    return true;
  default:
    resultData[0] = ENO_FP_ERR_COMMAND;
    return true;
  }
}

// Meldet den Kanal, der bereit zum Einlernen ist (0 = keiner), im Log und - nur mit ETS-Konfiguration, da die
// Gruppenobjekte sonst nicht existieren - über das KO IsTeachChannel.
void EnoceanModule::setIsTeachChannel(uint8_t channel) {
  logInfoP("Teach channel: %u", channel);
  if (knx.configured())
    KoENO_IsTeachChannel.value(channel, Dpt(5, 10));
}

// Fragt den aktuellen Lernmodus ab und meldet über das KO IsTeachChannel den aktiven Kanal bzw. 0, wenn der
// Lernmodus nicht aktiv ist (ausgelöst über SetTeachChannel = 100).
void EnoceanModule::reportTeachChannel() {
  uint8_t enable = 0;
  uint8_t activeChannel = 0;
  if (!readLearnMode(&enable, &activeChannel))
    return;

  setIsTeachChannel((uint8_t)(enable ? activeChannel : 0));
}

// Beendet den Lernmodus des Transceivers (es ist immer nur ein Kanal im Lernmodus, siehe CO_RD_LEARNMODE) und
// meldet über IsTeachChannel = 0, dass kein Kanal mehr bereit zum Einlernen ist (SetTeachChannel = 255).
void EnoceanModule::disableLearnMode() {
  uint8_t enable = 0;
  uint8_t activeChannel = 0;
  if (readLearnMode(&enable, &activeChannel) && !enable) {
    setIsTeachChannel((uint8_t)0);
    return;
  }

  if (activateLearnMode(activeChannel, false))
    setIsTeachChannel((uint8_t)0);
}

// Klartext zur Ursache von CO_EVENT_SECUREDEVICES (ESP3-Spec Kap. 2.4.7).
static const char *secureEventText(uint8_t cause) {
  switch (cause) {
  case 0x00:
    return "teach-in failed, secure link table full";
  case 0x02:
    return "resynchronization with wrong private key";
  case 0x03:
    return "too many telegrams with wrong CMAC";
  case 0x04:
    return "teach-in failed, telegram corrupted";
  case 0x05:
    return "teach-in failed, no PSK set for device";
  case 0x06:
    return "teach-in failed, PSK required";
  case 0x07:
    return "CMAC or RLC not correct";
  case 0x08:
    return "unencrypted telegram from secure device (device sends unencrypted again? delete teach-in and teach again)";
  case 0x09:
    return "teach-in ok, device added";
  default:
    return "unknown cause";
  }
}

// Wertet ein EVENT-Paket des Transceivers aus (ESP3-Spec Kap. 2.4).
void EnoceanModule::handleEvent(PACKET_SERIAL_TYPE_ *f_Pkt_st) {
  if (f_Pkt_st->u16DataLength < 1)
    return;

  switch (f_Pkt_st->u8DataBuffer[0]) {
  case u8CO_READY:
    // Transceiver wurde (neu) gestartet, Data[1] = Wakeup Cause
    logInfoP("Transceiver ready, wakeup cause: %u", f_Pkt_st->u16DataLength >= 2 ? f_Pkt_st->u8DataBuffer[1] : 0xFF);
    break;
  case u8CO_DUTYCYCLE_LIMIT:
    // Data[1]: 0 = Senden wieder möglich, 1 = Duty-Cycle-Limit erreicht
    if (f_Pkt_st->u16DataLength >= 2 && f_Pkt_st->u8DataBuffer[1] == 0x01)
      logErrorP("Duty cycle limit reached, telegrams can not be sent");
    else
      logInfoP("Duty cycle limit released");
    break;
  case u8CO_TRANSMIT_FAILED:
    // Data[1]: 0 = Funkkanal belegt, 1 = kein Acknowledge
    logErrorP("Transmit failed, cause: %u", f_Pkt_st->u16DataLength >= 2 ? f_Pkt_st->u8DataBuffer[1] : 0xFF);
    break;
  case u8CO_LRN_MODE_DISABLED:
    // Lernmodus-Timeout abgelaufen -> kein Kanal mehr bereit zum Einlernen
    logInfoP("Learn mode disabled (timeout)");
    _teachChannel = 0;
    setIsTeachChannel((uint8_t)0);
    break;
  case u8CO_EVENT_SECUREDEVICES: {
    // Data[1] = Ursache, Data[2..5] = Geräte-ID (ESP3-Spec Kap. 2.4.7)
    if (f_Pkt_st->u16DataLength < 6)
      break;
    uint8_t cause = f_Pkt_st->u8DataBuffer[1];
    uint8_t *id = &f_Pkt_st->u8DataBuffer[2];
    logInfoP("Secure device event %u (%s), ID %02X%02X%02X%02X", cause, secureEventText(cause), id[0], id[1], id[2], id[3]);
    if (cause == 0x09 && _teachChannel) {
      // Teach-In erfolgreich, Gerät in der Secure-Link-Tabelle des TCM
      storeTeachIn(id, 0, 0, 0, ENO_TEACH_FLAG_SECURE); // Profil im Secure-Teach-In nicht enthalten
      finishTeachIn();
    } else if (_teachChannel && (cause == 0x00 || cause == 0x04 || cause == 0x05 || cause == 0x06)) {
      // Tabelle voll, fehlerhaftes Teach-In-Telegramm oder PSK erforderlich
      _teachErrorChannel = _teachChannel;
      _teachErrorCause = cause;
      logErrorP("Secure teach-in failed for channel %u, cause %u", _teachChannel, cause);
      stopTeachIn();
    }
    break;
  }
  case u8CO_TX_DONE:
    // Adressiertes Telegramm (z.B. Teach-In-Antwort) wurde gesendet
    logDebugP("Addressed telegram sent (TX done)");
    break;
  default:
    logDebugP("Event %u ignored", f_Pkt_st->u8DataBuffer[0]);
    break;
  }
}

// Fragt beim Transceiver die Basis-ID per ESP3-Common-Command an und wartet (mit Timeout) auf die Antwort.
// Liefert true, wenn eine gültige Base-ID gelesen wurde.
bool EnoceanModule ::readBaseId(uint8_t *fui8_BaseID_p) {
  PACKET_SERIAL_TYPE_ lRdBaseIDPkt_st;

  uint8_t lu8SndBuf = u8CO_RD_IDBASE;

  lRdBaseIDPkt_st.u16DataLength = 0x0001;
  lRdBaseIDPkt_st.u8OptionLength = 0x00;
  lRdBaseIDPkt_st.u8Type = u8RORG_COMMON_COMMAND;
  lRdBaseIDPkt_st.u8DataBuffer = &lu8SndBuf;

  logDebugP("Sending telegram (read base ID).");

  if (ENOCEAN_OK != uart_sendPacket(&lRdBaseIDPkt_st))
    return false;

  logDebugP("Receiving telegram (read base ID).");
  u8RetVal = waitForResponse(ENOCEAN_RESPONSE_TIMEOUT);
  if (u8RetVal != ENOCEAN_OK) {
    logDebugP("ERROR Receiving telegram (read base ID).");
    return false;
  }

  logHexDebugP(m_Pkt_st.u8DataBuffer, m_Pkt_st.u16DataLength + (uint16_t)m_Pkt_st.u8OptionLength);

  // Response: RET_CODE (1) + Base-ID (4) [+ Optional: remaining write cycles]
  if (m_Pkt_st.u8DataBuffer[0] != 0x00 || m_Pkt_st.u16DataLength < 1 + BASEID_BYTES) {
    logDebugP("Read base ID Return Code: %u", m_Pkt_st.u8DataBuffer[0]);
    return false;
  }

  memcpy(fui8_BaseID_p, &m_Pkt_st.u8DataBuffer[1], BASEID_BYTES);

  // Optional Data: verbleibende Base-ID-Schreibzyklen (0xFF = unbegrenzt, z.B. TCM 615)
  if (m_Pkt_st.u8OptionLength >= 1) {
    _baseIdRemainingWrites = m_Pkt_st.u8DataBuffer[m_Pkt_st.u16DataLength];
    if (_baseIdRemainingWrites == 0xFF)
      logInfoP("Base-ID write cycles: unlimited");
    else
      logInfoP("Base-ID write cycles remaining: %u", _baseIdRemainingWrites);
  }
  return true;
}

// Wartet maximal timeoutMs auf ein RESPONSE-Paket des Transceivers. In der Zwischenzeit empfangene Funktelegramme
// werden normal an die Kanäle verteilt, damit sie während der Wartezeit nicht verloren gehen.
// Meldet der Transceiver per COMMAND_ACCEPTED eine länger dauernde Operation (ESP3-Spec Kap. 2.10), wird die
// Wartezeit um die gemeldete Dauer (max. ENOCEAN_ACCEPTED_TIMEOUT_MAX) verlängert.
// Liefert ENOCEAN_OK (Response liegt in m_Pkt_st) oder ENOCEAN_NO_RX_TEL bei Timeout.
uint8_t EnoceanModule::waitForResponse(uint32_t timeoutMs) {
  uint32_t lStartTime = millis();
  while (millis() - lStartTime < timeoutMs) {
    if (uart_getPacket(&m_Pkt_st, (uint16_t)DATBUF_SZ) != ENOCEAN_OK)
      continue;

    if (m_Pkt_st.u8Type == u8RESPONSE)
      return ENOCEAN_OK;

    if (m_Pkt_st.u8Type == u8EVENT) {
      handleEvent(&m_Pkt_st);
      continue;
    }

    if (m_Pkt_st.u8Type == u8COMMAND_ACCEPTED) {
      // Data: Blocking (1) + Estimated Completion Time (2, MSB first, 0 = unbekannt)
      uint32_t estimatedMs = 0;
      if (m_Pkt_st.u16DataLength >= 3)
        estimatedMs = ((uint16_t)m_Pkt_st.u8DataBuffer[1] << 8) | m_Pkt_st.u8DataBuffer[2];
      if (estimatedMs == 0 || estimatedMs > ENOCEAN_ACCEPTED_TIMEOUT_MAX)
        estimatedMs = ENOCEAN_ACCEPTED_TIMEOUT_MAX;
      logDebugP("Command accepted, waiting up to %u ms for response", estimatedMs + timeoutMs);
      lStartTime = millis();
      timeoutMs += estimatedMs;
      continue;
    }

    if (_channelsReady)
      getEnOceanMSG(ENOCEAN_OK, &m_Pkt_st);
  }
  logInfoP("Timeout waiting for transceiver response (%u ms)", timeoutMs);
  return ENOCEAN_NO_RX_TEL;
}





// Rückweg KNX→EnOcean: ermittelt Kanal und lokalen KO-Index des beschriebenen Gruppenobjekts über die ENO_KoCalc*-Makros.
void EnoceanModule::processInputKo(GroupObject &iKo) {
  if (iKo.asap() == ENO_KoSetTeachChannel) {
    uint8_t teachChannel = iKo.value(Dpt(5, 10));
    logDebugP("processInputKo: SetTeachChannel = %u", teachChannel);
    handleTeachCommand(teachChannel);
    return;
  }

  int channel = ENO_KoCalcChannel(iKo.asap());
  if (channel < 0 || channel >= ParamENO_VisibleChannels)
    return; // Gruppenobjekt gehört zu keinem aktiven EnOcean-Kanal

  uint8_t _channelIndex = (uint8_t)channel;
  int koIndex = ENO_KoCalcIndex(iKo.asap());

  // logDebugP("processInputKo: Kanal %u, KO-Index %d", _channelIndex+1, koIndex);
  handleKnxEvent(_channelIndex, koIndex, iKo);
}







// Wird aufgerufen, wenn ein Gruppenobjekt eines EnOcean-Kanals von der KNX-Seite beschrieben wurde; _channelIndex ist der betroffene Kanal, koIndex die lokale
// Position (0-basiert) innerhalb des Kanal-Blocks, ko der Wert.
void EnoceanModule::handleKnxEvent(uint8_t _channelIndex, int koIndex, GroupObject &iKo) {
  //logDebugP("handleKnxEvent: KO-Index %d, Value: %he", koIndex);
  //logHexDebugP(iKo.valueRef(), iKo.valueSize());
  // TODO: je nach konfiguriertem EEP-Profil und koIndex das passende EnOcean-Funktelegramm senden.

  // Bidirektionale Geräte (A5-20-01, z.B. FKS-SV): Wert merken, gesendet wird mit der nächsten Antwort an das Gerät
  if (_channels[_channelIndex]->processKnxHvac(koIndex, iKo))
    return;

//  if (koIndex != _channelIndex) // prüft ob KO zum CHannel passt
//  {
//    return;
//  }

  // SERIAL_PORT.print(F("Profil: "));
  // SERIAL_PORT.println(knx.paramByte(ENO_CHProfilSelection + firstParameter));

  switch (ParamENO_CHProfilSelection) {
  case u8RORG_4BS:
    logInfoP("4BS %u", ParamENO_CHProfil4BS20); //knx.paramWord(ENO_CHProfil4BS20 + firstParameter));
    switch (ParamENO_CHProfil4BS20) {
    // *************** A5-20-04 ***********************************************************
    case A5_20_04:
//      switch (koNr) {
//      case KO_0: //  SET Pos
//        union1.val_A5_20_04[0] = (uint8_t)iKo.value(getDPT(VAL_DPT_5));
//        union1.val_A5_20_04[0] = round(union1.val_A5_20_04[0] / 2.55);
//#ifdef KDEBUG
//        SERIAL_PORT.print(F("SET Pos to: "));
//        SERIAL_PORT.print(union1.val_A5_20_04[0]);
//        SERIAL_PORT.println(F("%"));
//#endif
//        break;
//      case KO_1: //  SET Temp
//        union1.val_A5_20_04[1] = ((float)iKo.value(getDPT(VAL_DPT_9)) - 10.0) / 0.078;
//#ifdef KDEBUG
//        SERIAL_PORT.print(F("SET Temp to: "));
//        SERIAL_PORT.print(union1.val_A5_20_04[1] * 0.078 + 10.0);
//        SERIAL_PORT.println(F("°C"));
//#endif
//        break;
//      case KO_2: // Wake UP cycle
//        union3.A52004 = iKo.value(getDPT(VAL_DPT_5));
//        if (union3.A52004 <= 63) {
//          union1.val_A5_20_04[2] = ((uint8_t)union3.A52004); // Set WCU
//#ifdef KDEBUG
//          SERIAL_PORT.print(F("WCU: "));
//          SERIAL_PORT.println(union3.A52004);
//          SERIAL_PORT.print(F("DB1: "));
//          SERIAL_PORT.println(union1.val_A5_20_04[2]);
//#endif
//        } else {
//#ifdef KDEBUG
//          SERIAL_PORT.print(F("WCU: wrong Value"));
//#endif
//        }
//#ifdef KDEBUG
//        SERIAL_PORT.print(F("MC: "));
//        SERIAL_PORT.println((knx.paramByte(ENO_CHA52004MC + firstParameter) >> ENO_CHA52004MCShift) & 1);
//#endif
//        break;
//      case KO_3: // Button Lock
//        if (iKo.value(getDPT(VAL_DPT_1))) {
//          union1.val_A5_20_04[3] |= (1 << 2); // set   Bit2
//        } else {
//          union1.val_A5_20_04[3] &= ~(1 << 2); // clear Bit2
//        }
//#ifdef KDEBUG
//        SERIAL_PORT.print(F("BLC: "));
//        SERIAL_PORT.println((bool)iKo.value(getDPT(VAL_DPT_1)));
//#endif
//        break;
//      case KO_4: // Service CMD
//        union3.A52004 = iKo.value(getDPT(VAL_DPT_5));
//
//        switch (union3.A52004) {
//        case 0:                                // No Change
//          union1.val_A5_20_04[3] &= ~(1 << 0); // clear Bit0
//          union1.val_A5_20_04[3] &= ~(1 << 1); // clear Bit1
//          break;
//        case 1:                                // open Value
//          union1.val_A5_20_04[3] |= (1 << 0);  // set   Bit0
//          union1.val_A5_20_04[3] &= ~(1 << 1); // clear Bit1
//          break;
//        case 2:                                // RUN initilistation
//          union1.val_A5_20_04[3] &= ~(1 << 0); // clear Bit0
//          union1.val_A5_20_04[3] |= (1 << 1);  // set   Bit1
//          break;
//        case 3:                               // RUN initilistation
//          union1.val_A5_20_04[3] |= (1 << 0); // set   Bit0
//          union1.val_A5_20_04[3] |= (1 << 1); // set   Bit1
//          break;
//        default:
//          union1.val_A5_20_04[3] &= ~(1 << 0); // clear Bit0
//          union1.val_A5_20_04[3] &= ~(1 << 1); // clear Bit1
//          break;
//        }
//#ifdef KDEBUG
//        SERIAL_PORT.print(F("Service : "));
//        SERIAL_PORT.println(union3.A52004);
//#endif
//        break;
//      }
      break;
    // *************** A5-20-06 ***********************************************************
    case A5_20_06:
//
//      switch (koNr) {
//      case KO_Teachin: // Teach-in MSG
//        teachinCH = iKo.value(getDPT(VAL_DPT_5));
//#ifdef KDEBUG_min
//        SERIAL_PORT.println(teachinCH);
//        SERIAL_PORT.println(index + 1);
//#endif
//        if (teachinCH != index + 1) {
//          return;
//        }
//#ifdef KDEBUG_min
//        SERIAL_PORT.println(F("ready to send"));
//#endif
//        union1.val_A5_20_06[0] = 0x80;
//        union1.val_A5_20_06[1] = 0x30;
//        union1.val_A5_20_06[2] = 0x49;
//        union1.val_A5_20_06[3] = 0x80;
//        send_4BS_Msg(lui8_SendeID_p, index, union1.val_A5_20_06, 0);
//        break;
//
//      case KO_0: // SET Temp oder SET Pos
//        if ((knx.paramByte(ENO_CHA52006SPS + firstParameter) >> ENO_CHA52006SPSShift) & 1) {
//          union1.val_A5_20_06[0] = (float)iKo.value(getDPT(VAL_DPT_9)) * 2.0; // Set Point Temp#
//#ifdef KDEBUG
//          SERIAL_PORT.print(F("SET Temp to: "));
//          SERIAL_PORT.print(union1.val_A5_20_06[0] / 2.0);
//          SERIAL_PORT.println(F("°C"));
//#endif
//        } else {
//          union1.val_A5_20_06[0] = (uint8_t)iKo.value(getDPT(VAL_DPT_5)); // Set Point Pos
//          union1.val_A5_20_06[0] = round(union1.val_A5_20_06[0] / 2.55);
//#ifdef KDEBUG
//          SERIAL_PORT.print(F("SET Pos to: "));
//          SERIAL_PORT.print(union1.val_A5_20_06[0]);
//          SERIAL_PORT.println(F("%"));
//#endif
//        }
//        break;
//
//      case KO_1: // Sommer Umschaltung
//        if (iKo.value(getDPT(VAL_DPT_1))) {
//          union1.val_A5_20_06[2] = (uint8_t)union1.val_A5_20_06[2] | (1 << 3); // Set Bit
//#ifdef KDEBUG
//          SERIAL_PORT.println(F("Sommer Umschaltung: aktiv"));
//#endif
//        } else {
//          union1.val_A5_20_06[2] = (uint8_t)union1.val_A5_20_06[2] & ~(1 << 3); // clear Bit
//#ifdef KDEBUG
//          SERIAL_PORT.println(F("Sommer Umschaltung: inaktiv"));
//#endif
//        }
//        break;
//
//      case KO_2: // Set Communications Interval
//#ifdef KDEBUG
//        SERIAL_PORT.print(F("Com Interval: "));
//        SERIAL_PORT.println((uint8_t)iKo.value(getDPT(VAL_DPT_5)));
//#endif
//        union1.val_A5_20_06[2] &= ~(1 << ENO_CHA52006RFCShift);                                    // clear Bit4
//        union1.val_A5_20_06[2] &= ~(1 << (ENO_CHA52006RFCShift + 1));                              // clear Bit5
//        union1.val_A5_20_06[2] &= ~(1 << (ENO_CHA52006RFCShift + 2));                              // clear Bit6
//        union1.val_A5_20_06[2] |= ((uint8_t)iKo.value(getDPT(VAL_DPT_5)) << ENO_CHA52006RFCShift); // Set Bit
//        break;
//
//      case KO_3:                                                              // Raum Temperatur
//        union1.val_A5_20_06[1] = (uint8_t)iKo.value(getDPT(VAL_DPT_9)) * 4.0; // Raumtemperatur
//#ifdef KDEBUG
//        SERIAL_PORT.print(F("Raum-Temp: "));
//        SERIAL_PORT.print(union1.val_A5_20_06[1]);
//        SERIAL_PORT.println(F("°C"));
//#endif
//        break;
//
//      case KO_4: // Standby
//        if (iKo.value(getDPT(VAL_DPT_1))) {
//          union1.val_A5_20_06[2] = (uint8_t)union1.val_A5_20_06[2] | (1 << 0); // Set Bit
//#ifdef KDEBUG
//          SERIAL_PORT.println(F("Standby"));
//#endif
//        } else {
//          union1.val_A5_20_06[2] = (uint8_t)union1.val_A5_20_06[2] & ~(1 << 0); // clear Bit
//#ifdef KDEBUG
//          SERIAL_PORT.println(F("normal Mode"));
//#endif
//        }
//        break;
//
//      default:
//        break;
//      }
//      // gesendet wird in der TASK() da das Device nur nachrichten Empfangen kann, 1sek nachdem es eine Nachrticht
//      // geschickt hat send_4BS_Msg(lui8_SendeID_p, index, union1.val_A5_20_06);
//      break;
//    } // ENDE 2BS
    break;
  case u8RORG_VLD:
    switch (ParamENO_CHProfilSelectionVLD) {
    case D2_01:
//      switch (knx.paramWord(ENO_CHProfilVLD01 + firstParameter)) {
//      case D2_01_0E:
//        switch (koNr) {
//        case KO_0: // schalten Aktor
//          if (iKo.value(getDPT(VAL_DPT_1))) {
//            buttonStateSimulation1 = SIMULATE_PUSH;
//            buttonMessage1 = true;
//          } else {
//            buttonStateSimulation1 = SIMULATE_PUSH;
//            buttonMessage1 = false;
//          }
//#ifdef KDEBUG
//          SERIAL_PORT.println(F("KNX KO_0 handled"));
//#endif
//          break;
//        case KO_1: // Anfrage Messwerte
//#ifdef KDEBUG
//          SERIAL_PORT.println(F("KNX KO_1 handled"));
//#endif
//          getActorsMeasurmentValue(lui8_SendeID_p, index, union1.val_D2_01_0E_Energy,
//                                   iKo.value(getDPT(VAL_DPT_1))); // Energy = 0  / Power = 1
//          break;
//        case KO_2: // Anfrage Statuswerte SWITCH
//#ifdef KDEBUG
//          SERIAL_PORT.println(F("KNX KO_2 handled"));
//#endif
//          getStatusActors(lui8_SendeID_p, index); // Request Aktor Status
//          break;
//        }
//      default:
        break; // ENDE VLD D2_01_0E
      case D2_01_12:
//        switch (koNr) {
//        case KO_0: // schalten Aktor CH1
//          setStatusActors(lui8_SendeID_p, index, iKo.value(getDPT(VAL_DPT_1)));
//
//          if (iKo.value(getDPT(VAL_DPT_1))) {
//
//            /*buttonStateSimulation1 = SIMULATE_PUSH;
//            buttonMessage1 = true;*/
//#ifdef KDEBUG
//            SERIAL_PORT.println(F("KO value = HIGH"));
//#endif
//          } else {
//            /*buttonStateSimulation1 = SIMULATE_PUSH;
//            buttonMessage1 = false;*/
//#ifdef KDEBUG
//            SERIAL_PORT.println(F("KO value = LOW"));
//#endif
//          }
//#ifdef KDEBUG
//          SERIAL_PORT.println(F("KNX KO_0 handled"));
//#endif
//          break;
//        case KO_1: //  Schalten Aktor CH2
//          // union4.koIndex2 = index + 2;
//          if (iKo.value(getDPT(VAL_DPT_1))) {
//            buttonStateSimulation2 = SIMULATE_PUSH;
//            buttonMessage2 = true;
//          } else {
//            buttonStateSimulation2 = SIMULATE_PUSH;
//            buttonMessage2 = false;
//          }
//#ifdef KDEBUG
//          SERIAL_PORT.println(F("KNX KO_1 handled"));
//#endif
//          break;
//        case KO_4:
//#ifdef KDEBUG
//          SERIAL_PORT.println(F("KNX KO_4 handled"));
//#endif
//          getStatusActors(lui8_SendeID_p, index); // Request Aktor Status
//          break;
//        default:
//          break;
//        }
        break; // ENDE VLD D2_01_12
      }
      break; // ENDE VLD D2_01
    }
    break; // ENDE VLD

  case u8RORG_Rocker:
    // KNX -> EnOcean: Eingang KO_0 (GO_BASE__1) emuliert einen Taster, siehe EnoceanChannel::processKnxRocker()
    if (ParamENO_CHDirectionKnxEnocean == 1 && koIndex == ENO_KoGO_BASE__1)
      _channels[_channelIndex]->processKnxRocker(iKo);
    break;

  default:
    return;
    break; // ENDE ROCKER
  }
}

// Registriert den Hilfetext für den Konsolenbefehl "dummy".
// Registriert den Hilfetext für die Konsolenbefehle "eno ...".
void EnoceanModule::showHelp() { openknx.console.printHelpLine("eno", "EnOcean transceiver commands (teach, baseid, repeater, version)"); }

// Verarbeitet die Konsolenbefehle "eno ...". Sie funktionieren auch ohne ETS-Konfiguration (Testbetrieb).
bool EnoceanModule::processCommand(const std::string cmd, bool diagnoseKo) {
  if (cmd.substr(0, 3) != "eno")
    return false;

  if (cmd == "eno") {
    openknx.console.printHelpLine("eno teach N", "Start learn mode for channel N (1..30)");
    openknx.console.printHelpLine("eno teach off", "Stop learn mode");
    openknx.console.printHelpLine("eno teach ?", "Show channel in learn mode");
    openknx.console.printHelpLine("eno teach list", "List learned channels");
    openknx.console.printHelpLine("eno teach del N", "Delete learned device of channel N");
    openknx.console.printHelpLine("eno baseid", "Read Base-ID and remaining write cycles");
    openknx.console.printHelpLine("eno baseid FFxxxxxx", "Write Base-ID (FF800000..FFFFFF80)");
    openknx.console.printHelpLine("eno repeater", "Read repeater mode/level");
    openknx.console.printHelpLine("eno repeater M L", "Write repeater mode M (0..2) and level L (0..2)");
    openknx.console.printHelpLine("eno version", "Read transceiver version and EURID");
    openknx.console.printHelpLine("eno monitor on|off", "Log every received radio telegram");
    openknx.console.printHelpLine("eno learn N", "Send learn telegram of channel N to an actuator");
    return true;
  }

  if (cmd.substr(0, 10) == "eno teach ") {
    std::string arg = cmd.substr(10);
    if (arg == "off")
      handleTeachCommand(255);
    else if (arg == "?")
      handleTeachCommand(100);
    else if (arg == "list") {
      for (uint8_t i = 0; i < ENO_ChannelCount; i++)
        if (_teach[i].valid)
          logInfoP("Channel %u: ID %02X%02X%02X%02X, EEP %02X-%02X-%02X%s", i + 1, _teach[i].id[0], _teach[i].id[1], _teach[i].id[2],
                   _teach[i].id[3], _teach[i].eep[0], _teach[i].eep[1], _teach[i].eep[2], (_teach[i].flags & ENO_TEACH_FLAG_SECURE) ? " (secure)" : "");
      if (_teachChannel)
        logInfoP("Teach-in running for channel %u", _teachChannel);
    } else if (arg.substr(0, 4) == "del ") {
      uint8_t channel = (uint8_t)strtoul(arg.substr(4).c_str(), nullptr, 10);
      if (channel < 1 || channel > ENO_ChannelCount) {
        logInfoP("Channel must be 1..%u", ENO_ChannelCount);
        return true;
      }
      deleteTeachIn(channel);
    } else {
      uint8_t channel = (uint8_t)strtoul(arg.c_str(), nullptr, 10);
      if (channel < 1 || channel > 30) {
        logInfoP("Channel must be 1..30");
        return true;
      }
      handleTeachCommand(channel);
    }
    return true;
  }

  if (cmd == "eno baseid") {
    if (readBaseId(lui8_BaseID_p))
      logInfoP("Base-ID: %02X%02X%02X%02X", lui8_BaseID_p[0], lui8_BaseID_p[1], lui8_BaseID_p[2], lui8_BaseID_p[3]);
    else
      logErrorP("Base-ID could not be read from transceiver");
    return true;
  }

  if (cmd.substr(0, 11) == "eno baseid " && cmd.length() == 19) {
    uint32_t value = strtoul(cmd.substr(11).c_str(), nullptr, 16);
    uint8_t baseId[BASEID_BYTES] = {(uint8_t)(value >> 24), (uint8_t)(value >> 16), (uint8_t)(value >> 8), (uint8_t)value};
    if (!isValidBaseId(baseId)) {
      logErrorP("Base-ID out of range (FF800000..FFFFFF80, lower 7 bits 0)");
      return true;
    }
    setBaseId(baseId);
    if (readBaseId(lui8_BaseID_p))
      logInfoP("Base-ID: %02X%02X%02X%02X", lui8_BaseID_p[0], lui8_BaseID_p[1], lui8_BaseID_p[2], lui8_BaseID_p[3]);
    return true;
  }

  if (cmd == "eno repeater") {
    readRepeaterFunc();
    return true;
  }

  if (cmd.substr(0, 13) == "eno repeater " && cmd.length() == 16) {
    uint8_t mode = cmd[13] - '0';
    uint8_t level = cmd[15] - '0';
    if (mode > 2 || level > 2 || (mode == 0) != (level == 0)) {
      logInfoP("Mode 0..2 / level 0..2, level must be 0 exactly when mode is 0");
      return true;
    }
    setRepeaterFunc(mode, level);
    readRepeaterFunc();
    return true;
  }

  if (cmd.substr(0, 10) == "eno learn ") {
    uint8_t channel = (uint8_t)strtoul(cmd.substr(10).c_str(), nullptr, 10);
    if (!_channelsReady || channel < 1 || channel > ParamENO_VisibleChannels)
      logInfoP("Channel must be 1..%u", ParamENO_VisibleChannels);
    else if (!_channels[channel - 1]->sendLearnTelegram())
      logInfoP("Channel %u does not send to EnOcean (rocker KNX -> EnOcean only)", channel);
    return true;
  }

  if (cmd == "eno monitor on" || cmd == "eno monitor off") {
    _monitor = cmd == "eno monitor on";
    logInfoP("Telegram monitor %s", _monitor ? "on" : "off");
    return true;
  }

  if (cmd == "eno version") {
    readVersion();
    return true;
  }

  return false;
}

#ifdef ARDUINO_ARCH_RP2040
#ifndef OPENKNX_USB_EXCHANGE_IGNORE
// Registriert Beispiel-Callbacks für den USB-Dateiaustausch (Demo-Datei "Dummy.txt"); wird aktuell nirgends aufgerufen.
void EnoceanModule::registerUsbExchangeCallbacks() {
  // Sample
  openknxUsbExchangeModule.onLoad("Dummy.txt", [](UsbExchangeFile *file) -> void { file->write("Demo"); });
  openknxUsbExchangeModule.onEject("Dummy.txt", [](UsbExchangeFile *file) -> bool {
    // File is required
    if (file == nullptr) {
      logError("EnoceanModule", "File Dummy.txt was deleted but is mandatory");
      return false;
    }
    return true;
  });
}
#endif
#endif

// ESP3-Empfangs-Statemachine: liest byteweise vom UART, prüft Sync/CRC8 und liefert ein vollständiges Paket oder einen Fehlerstatus.
uint8_t EnoceanModule::uart_getPacket(PACKET_SERIAL_TYPE_ *pPacket, uint16_t u16BufferLength) {

  //! UART received byte code
  uint8_t u8RxByte;
  //! Checksum calculation
  static uint8_t u8CRC = 0;
  //! Nr. of bytes received
  static uint16_t u16Count = 0;
  //! State machine counter
  // static STATES_GET_PACKET_ u8State = GET_SYNC_STATE;
  //! Zeitpunkt des letzten empfangenen Bytes (Inter-Byte-Timeout)
  static uint32_t lastByteTime = 0;
  // Byte buffer pointing at the paquet address
  uint8_t *u8Raw = (uint8_t *)pPacket;
  // Temporal variable
  uint8_t i;
  // Inter-Byte-Timeout (ESP3-Spec Kap. 3.4.2): bricht ein Paket mittendrin ab, wieder auf das nächste Sync-Byte warten,
  // statt die folgenden Bytes als Rest des abgebrochenen Pakets zu lesen.
  if (u8State != GET_SYNC_STATE && (millis() - lastByteTime) > ENOCEAN_INTERBYTE_TIMEOUT) {
    logDebugP("Inter-byte timeout, resync");
    u8State = GET_SYNC_STATE;
  }
  // State machine goes on when a new byte is received
  // Nur bereits empfangene Bytes verarbeiten: read() blockiert nicht (im Gegensatz zu readBytes(), das bis zum
  // Stream-Timeout von 1 s auf weitere Bytes wartet).
  if (_serial->available() > 0) {
    while (_serial->available() > 0) {
      u8RxByte = (uint8_t)_serial->read();
      lastByteTime = millis();
      // State machine to load incoming packet bytes
      switch (u8State) {
      // Waiting for packet sync byte 0x55ENOModule::uart_getPacket
      case GET_SYNC_STATE:
        if (u8RxByte == SER_SYNCH_CODE) {
          u8State = GET_HEADER_STATE;
          u16Count = 0;
          u8CRC = 0;
        }
        break;
      // Read the header bytes
      case GET_HEADER_STATE:
        // Copy received data to buffer
        u8Raw[u16Count++] = u8RxByte;
        u8CRC = proc_crc8(u8CRC, u8RxByte);
        // All header bytes received?
        if (u16Count == SER_HEADER_NR_BYTES) {
          // Serial.print("Received all header bytes.\n");
          // Serial.print("pPacket->u16DataLength: ");
          // Serial.println(pPacket->u16DataLength, HEX);
          // Serial.print("u8Raw[1]");
          // Serial.println(u8Raw[1], HEX);
          // Serial.print("u8Raw[2]");
          // Serial.println(u8Raw[2], HEX);
          uint8_t temp;
          temp = u8Raw[0];
          u8Raw[0] = u8Raw[1];
          u8Raw[1] = temp;
          // Serial.print("pPacket->u16DataLength: ");
          // Serial.println(pPacket->u16DataLength, HEX);
          // Serial.print("u8Raw[1]");
          // Serial.println(u8Raw[1], HEX);
          // Serial.print("u8Raw[2]");
          // Serial.println(u8Raw[2], HEX);
          u8State = CHECK_CRC8H_STATE;
        }
        break;
      // Check header checksum & try to resynchonise if error happened
      case CHECK_CRC8H_STATE:
        // Header CRC correct?
        if (u8CRC != u8RxByte) {
          logDebugP("CRC check failed.");
          // No. Check if there is a sync byte (0x55) in the header
          int a = -1;
          for (i = 0; i < SER_HEADER_NR_BYTES; i++)
            if (u8Raw[i] == SER_SYNCH_CODE) {
              // indicates the next position to the sync byte found
              a = i + 1;
              break;
            };
          if ((a == -1) && (u8RxByte != SER_SYNCH_CODE)) {
            // Header and CRC8H does not contain the sync code
            u8State = GET_SYNC_STATE;
            break;
          } else if ((a == -1) && (u8RxByte == SER_SYNCH_CODE)) {
            // Header does not have sync code but CRC8H does.
            // The sync code could be the beginning of a packet
            u8State = GET_HEADER_STATE;
            u16Count = 0;
            u8CRC = 0;
            break;
          }
          // Header has a sync byte. It could be a new telegram.
          // Shift all bytes from the 0x55 code in the buffer.
          // Recalculate CRC8 for those bytes
          u8CRC = 0;
          for (i = 0; i < (SER_HEADER_NR_BYTES - a); i++) {
            u8Raw[i] = u8Raw[a + i];
            u8CRC = proc_crc8(u8CRC, u8Raw[i]);
          }
          u16Count = SER_HEADER_NR_BYTES - a;
          // u16Count = i; // Seems also valid and more intuitive than u16Count
          // -= a; Copy the just received byte to buffer
          u8Raw[u16Count++] = u8RxByte;
          u8CRC = proc_crc8(u8CRC, u8RxByte);
          if (u16Count < SER_HEADER_NR_BYTES) {
            u8State = GET_HEADER_STATE;
            break;
          }
          break;
        }
        // CRC8H correct. Length fields values valid?
        if ((pPacket->u16DataLength + pPacket->u8OptionLength) == 0) {
          // No. Sync byte received?
          if ((u8RxByte == SER_SYNCH_CODE)) {
            // yes
            u8State = GET_HEADER_STATE;
            u16Count = 0;
            u8CRC = 0;
            break;
          }
          // Packet with correct CRC8H but wrong length fields.
          u8State = GET_SYNC_STATE;
          return ENOCEAN_OUT_OF_RANGE;
        }
        // Correct header CRC8. Go to the reception of data.
        u8State = GET_DATA_STATE;
        u16Count = 0;
        u8CRC = 0;
        break;
      // Copy the information bytes
      case GET_DATA_STATE:
        // Copy byte in the packet buffer only if the received bytes have enough
        // room
        if (u16Count < u16BufferLength) {
          pPacket->u8DataBuffer[u16Count] = u8RxByte;
          u8CRC = proc_crc8(u8CRC, u8RxByte);
        }
        // When all expected bytes received, go to calculate data checksum
        if (++u16Count == (pPacket->u16DataLength + pPacket->u8OptionLength)) {
          u8State = CHECK_CRC8D_STATE;
        }

        // Serial.print(u16Count);
        // Serial.println(u16Count, DEC);

        break;
      // Check the data CRC8
      case CHECK_CRC8D_STATE:
        // In all cases the state returns to the first state: waiting for next
        // sync byte
        u8State = GET_SYNC_STATE;
        // Received packet bigger than space to allocate bytes?
        if (u16Count > u16BufferLength)
          return ENOCEAN_OUT_OF_RANGE;
        // Enough space to allocate packet. Equals last byte the calculated
        // CRC8?
        if (u8CRC == u8RxByte)
          return ENOCEAN_OK; // Correct packet received
        // False CRC8.
        // If the received byte equals sync code, then it could be sync byte for
        // next paquet.
        if ((u8RxByte == SER_SYNCH_CODE)) {
          u8State = GET_HEADER_STATE;
          u16Count = 0;
          u8CRC = 0;
        }
        return ENOCEAN_NOT_VALID_CHKSUM;
      default:
        // Yes. Go to the reception of info.
        u8State = GET_SYNC_STATE;
        break;
      }
    }
  }
  return (u8State == GET_SYNC_STATE) ? ENOCEAN_NO_RX_TEL : ENOCEAN_NEW_RX_BYTE;
}

// Sendet ein Paket im ESP3-Format über UART (Sync-Byte, Header inkl. CRC8, Daten inkl. CRC8).
uint8_t EnoceanModule::uart_sendPacket(PACKET_SERIAL_TYPE_ *pPacket) {
  uint16_t i;
  uint8_t u8CRC;
  uint8_t *u8Raw = (uint8_t *)pPacket;
  // When both length fields are 0, then this telegram is not allowed.

  if ((pPacket->u16DataLength || pPacket->u8OptionLength) == 0) {
    return ENOCEAN_OUT_OF_RANGE;
  }

  uint16_t lui16_PacketLength = pPacket->u16DataLength;
  uint8_t temp;
  temp = u8Raw[0];
  u8Raw[0] = u8Raw[1];
  u8Raw[1] = temp;
  // Sync
  while (_serial->write(0x55) != 1)
    ;
  // Header
  _serial->write((char *)pPacket, 4);
  // Header CRC
  u8CRC = 0;
  u8CRC = proc_crc8(u8CRC, ((uint8_t *)pPacket)[0]);
  u8CRC = proc_crc8(u8CRC, ((uint8_t *)pPacket)[1]);
  u8CRC = proc_crc8(u8CRC, ((uint8_t *)pPacket)[2]);
  u8CRC = proc_crc8(u8CRC, ((uint8_t *)pPacket)[3]);
  while (_serial->write(u8CRC) != 1)
    ;
  // Data
  u8CRC = 0;
  for (i = 0; i < lui16_PacketLength + pPacket->u8OptionLength; i++) {
    u8CRC = proc_crc8(u8CRC, pPacket->u8DataBuffer[i]);
    while (_serial->write(pPacket->u8DataBuffer[i]) != 1)
      ;
  }
  // Data CRC
  while (_serial->write(u8CRC) != 1)
    ;
  return ENOCEAN_OK;
}

// Wertet ein empfangenes ERP1-Funktelegramm aus und reicht es an alle konfigurierten Kanäle zur ID-Prüfung/Verarbeitung weiter.
void EnoceanModule::getEnOceanMSG(uint8_t u8RetVal, PACKET_SERIAL_TYPE_ *f_Pkt_st) {
  if (u8RetVal == ENOCEAN_OK) {
    
    logDebugP("Received Data:");
    logHexDebugP(f_Pkt_st->u8DataBuffer, f_Pkt_st->u16DataLength + (uint16_t)f_Pkt_st->u8OptionLength);

    if (f_Pkt_st->u8Type == u8RADIO_ERP1) {

      /*
      if (f_Pkt_st->u8DataBuffer[0] == u8RORG_RPS) {
        logDebugP("Typ: RPS, Eno-ID: %02X-%02X-%02X-%02X, Data: %02X", f_Pkt_st->u8DataBuffer[2],
                  f_Pkt_st->u8DataBuffer[3], f_Pkt_st->u8DataBuffer[4], f_Pkt_st->u8DataBuffer[5],
                  f_Pkt_st->u8DataBuffer[1]);
      } else if (f_Pkt_st->u8DataBuffer[0] == u8RORG_VLD) {
        logDebugP("Typ: VLD, Eno-ID: %02X-%02X-%02X-%02X", f_Pkt_st->u8DataBuffer[f_Pkt_st->u16DataLength - 5],
                  f_Pkt_st->u8DataBuffer[f_Pkt_st->u16DataLength - 4], f_Pkt_st->u8DataBuffer[f_Pkt_st->u16DataLength - 3],
                  f_Pkt_st->u8DataBuffer[f_Pkt_st->u16DataLength - 2]);
        logHexDebugP(&f_Pkt_st->u8DataBuffer[1], f_Pkt_st->u16DataLength - 6);
      } else if (f_Pkt_st->u8DataBuffer[0] == u8RORG_4BS) {
        logDebugP("Typ: 4BS, Eno-ID: %02X-%02X-%02X-%02X, Data: %02X %02X %02X %02X", f_Pkt_st->u8DataBuffer[5],
                  f_Pkt_st->u8DataBuffer[6], f_Pkt_st->u8DataBuffer[7], f_Pkt_st->u8DataBuffer[8],
                  f_Pkt_st->u8DataBuffer[1], f_Pkt_st->u8DataBuffer[2], f_Pkt_st->u8DataBuffer[3],
                  f_Pkt_st->u8DataBuffer[4]);
      } else if (f_Pkt_st->u8DataBuffer[0] == u8RORG_1BS) {
        logDebugP("Typ: 1BS, Eno-ID: %02X-%02X-%02X-%02X, Data: %02X", f_Pkt_st->u8DataBuffer[2],
                  f_Pkt_st->u8DataBuffer[3], f_Pkt_st->u8DataBuffer[4], f_Pkt_st->u8DataBuffer[5],
                  f_Pkt_st->u8DataBuffer[1]);
      }
      */
      bool packetWasHandled = false;
      for (uint8_t i = 0; i < ParamENO_VisibleChannels; i++) {
        if (_channels[i]->check_Eno_ID(f_Pkt_st)) {
          packetWasHandled = true;
        }
      } 

      // Testbetrieb ohne ETS-Konfiguration: jedes Telegramm mit Sender-ID und Empfangsstärke loggen
      if (!knx.configured() || _monitor)
        logReceivedTelegram(f_Pkt_st);

      // Secure-R-ORG (SEC 0x30, SEC_CDM 0x31/0x33, SEC_ENCAPS 0x32): Der TCM gibt entschlüsselte Telegramme mit diesem
      // R-ORG und den entschlüsselten Nutzdaten weiter; der Security-Level in den Optional Data zeigt, ob das geklappt
      // hat (2 = entschlüsselt, 3 = authentifiziert, 4 = beides). Nur nicht entschlüsselte Telegramme verwerfen.
      if (isSecureRorg(f_Pkt_st->u8DataBuffer[0])) {
        uint8_t security = (f_Pkt_st->u8OptionLength >= 7) ? f_Pkt_st->u8DataBuffer[f_Pkt_st->u16DataLength + 6] : 0;
        uint16_t len = f_Pkt_st->u16DataLength;
        if (security < 2) {
          if (len >= 6)
            logInfoP("Encrypted telegram (R-ORG %02X) from %02X%02X%02X%02X not decrypted by transceiver (device not in secure link table?)",
                     f_Pkt_st->u8DataBuffer[0], f_Pkt_st->u8DataBuffer[len - 5], f_Pkt_st->u8DataBuffer[len - 4], f_Pkt_st->u8DataBuffer[len - 3],
                     f_Pkt_st->u8DataBuffer[len - 2]);
          return;
        }
      }

      // Teach-In aktiv: Lerntelegramm eines unbekannten Senders dem Kanal zuordnen. Bidirektionale Geräte (z.B.
      // A5-20-01) erwarten darauf eine Teach-In-Antwort des Kanals.
      uint8_t teachChannel = _teachChannel;
      if (teachChannel && tryTeachIn(f_Pkt_st)) {
        if (knx.configured() && _channelsReady && teachChannel <= ParamENO_VisibleChannels)
          _channels[teachChannel - 1]->teachInReceived(f_Pkt_st);
        finishTeachIn();
        return;
      }

      // Senden Unknown ID
      if(packetWasHandled == false && ParamENO_NewID_OnOff == true)
      {
        checkAndReportUnknownId(f_Pkt_st);
      }

      logDebugP(packetWasHandled ? "Data handled :-)" : "Data not handled!");
    } else if (f_Pkt_st->u8Type == u8EVENT) {
      handleEvent(f_Pkt_st);
    }
  }
}

// Ermittelt anhand des RORG-Typs die Position der 4-Byte Sender-ID im empfangenen Telegramm.
// Liefert false, wenn der RORG-Typ nicht unterstützt wird bzw. das Telegramm dafür zu kurz ist.
bool EnoceanModule::extractSenderId(const PACKET_SERIAL_TYPE_ *f_Pkt_st, uint8_t out8SenderId[4]) {
  switch (f_Pkt_st->u8DataBuffer[0]) {
  case u8RORG_RPS:
  case u8RORG_1BS:
    // RORG (1) + Data (1) + Sender-ID (4) + Status (1)
    if (f_Pkt_st->u16DataLength < 7)
      return false;
    out8SenderId[0] = f_Pkt_st->u8DataBuffer[2];
    out8SenderId[1] = f_Pkt_st->u8DataBuffer[3];
    out8SenderId[2] = f_Pkt_st->u8DataBuffer[4];
    out8SenderId[3] = f_Pkt_st->u8DataBuffer[5];
    return true;

  case u8RORG_4BS:
    // RORG (1) + Data (4) + Sender-ID (4) + Status (1)
    if (f_Pkt_st->u16DataLength < 10)
      return false;
    out8SenderId[0] = f_Pkt_st->u8DataBuffer[5];
    out8SenderId[1] = f_Pkt_st->u8DataBuffer[6];
    out8SenderId[2] = f_Pkt_st->u8DataBuffer[7];
    out8SenderId[3] = f_Pkt_st->u8DataBuffer[8];
    return true;

  case u8RORG_VLD:
  case u8RORG_UTE:
  case u8RORG_SEC:
  case u8RORG_SEC_CDM:
  case u8RORG_SEC_ENCAPS:
  case u8RORG_SEC_CDM_ENCAPS:
    // RORG (1) + Data (1..14, UTE: 7) + Sender-ID (4) + Status (1)
    if (f_Pkt_st->u16DataLength < 7)
      return false;
    out8SenderId[0] = f_Pkt_st->u8DataBuffer[f_Pkt_st->u16DataLength - 5];
    out8SenderId[1] = f_Pkt_st->u8DataBuffer[f_Pkt_st->u16DataLength - 4];
    out8SenderId[2] = f_Pkt_st->u8DataBuffer[f_Pkt_st->u16DataLength - 3];
    out8SenderId[3] = f_Pkt_st->u8DataBuffer[f_Pkt_st->u16DataLength - 2];
    return true;

  default:
    return false;
  }
}

// Prüft kanalübergreifend, ob die Sender-ID des empfangenen Telegramms an einem Kanal parametriert ist.
// Ist sie an keinem Kanal bekannt, wird sie sofort über das KO "NewId" auf den KNX-Bus gemeldet.
void EnoceanModule::checkAndReportUnknownId(PACKET_SERIAL_TYPE_ *f_Pkt_st) {
  uint8_t senderId[4];
  if (!extractSenderId(f_Pkt_st, senderId))
    return;

  /*/  
  for (uint8_t i = 0; i < ParamENO_VisibleChannels; i++) {
    if (_channels[i]->matchesDeviceId(senderId))
      return; // ID ist an einem Kanal bekannt
  }
  */    

  char idStr[15];
  snprintf(idStr, sizeof(idStr), "%02X%02X%02X%02X", senderId[0], senderId[1], senderId[2], senderId[3]);
  logInfoP("Unknown Enocean-ID: %s", idStr);
  if (knx.configured())
    KoENO_NewId.value(idStr, Dpt(16, 1));
}

// Loggt ein empfangenes ERP1-Telegramm mit R-ORG, Sender-ID, Nutzdaten und Empfangsstärke (Testbetrieb).
void EnoceanModule::logReceivedTelegram(PACKET_SERIAL_TYPE_ *f_Pkt_st) {
  // Optional Data: SubTelNum (1), Destination ID (4), dBm (1), Security Level (1)
  uint8_t dBm = (f_Pkt_st->u8OptionLength >= 6) ? f_Pkt_st->u8DataBuffer[f_Pkt_st->u16DataLength + 5] : 0;
  uint8_t security = (f_Pkt_st->u8OptionLength >= 7) ? f_Pkt_st->u8DataBuffer[f_Pkt_st->u16DataLength + 6] : 0;

  // Sender-ID + Status = letzte 5 Bytes der Daten (gilt für alle ERP1-Telegramme), Nutzdaten dazwischen
  uint16_t len = f_Pkt_st->u16DataLength;
  if (len < 6) {
    logInfoP("RX R-ORG %02X (too short, %u bytes)", f_Pkt_st->u8DataBuffer[0], len);
    return;
  }
  uint8_t *id = &f_Pkt_st->u8DataBuffer[len - 5];

  std::string data;
  char hexByte[4];
  for (uint16_t i = 1; i + 5 < len; i++) {
    snprintf(hexByte, sizeof(hexByte), "%02X ", f_Pkt_st->u8DataBuffer[i]);
    data += hexByte;
  }

  logInfoP("RX R-ORG %02X ID %02X%02X%02X%02X -%u dBm Security %u Status %02X Data: %s", f_Pkt_st->u8DataBuffer[0], id[0], id[1],
           id[2], id[3], dBm, security, f_Pkt_st->u8DataBuffer[len - 1], data.c_str());
}

// Liest Versionsinformationen und EURID des Transceivers (CO_RD_VERSION, ESP3-Spec Kap. 2.5.5) und loggt sie.
void EnoceanModule::readVersion() {
  PACKET_SERIAL_TYPE_ lRdVersionPkt_st;

  uint8_t lu8SndBuf = u8CO_RD_VERSION;

  lRdVersionPkt_st.u16DataLength = 0x0001;
  lRdVersionPkt_st.u8OptionLength = 0x00;
  lRdVersionPkt_st.u8Type = u8RORG_COMMON_COMMAND;
  lRdVersionPkt_st.u8DataBuffer = &lu8SndBuf;

  if (ENOCEAN_OK != uart_sendPacket(&lRdVersionPkt_st))
    return;

  if (waitForResponse(ENOCEAN_RESPONSE_TIMEOUT) != ENOCEAN_OK) {
    logErrorP("Version could not be read from transceiver");
    return;
  }

  // Response: RET_CODE (1), App Version (4), API Version (4), EURID (4), Device Version (4), App Description (16)
  uint8_t *d = m_Pkt_st.u8DataBuffer;
  if (d[0] != 0x00 || m_Pkt_st.u16DataLength < 33) {
    logErrorP("Read version Return Code: %u", d[0]);
    return;
  }

  char description[17];
  memcpy(description, &d[17], 16);
  description[16] = '\0';

  memcpy(_appVersion, &d[1], 4);
  memcpy(_apiVersion, &d[5], 4);
  memcpy(_eurid, &d[9], 4);
  _versionValid = true;

  logInfoP("App version: %u.%u.%u.%u", d[1], d[2], d[3], d[4]);
  logInfoP("API version: %u.%u.%u.%u", d[5], d[6], d[7], d[8]);
  logInfoP("EURID: %02X%02X%02X%02X", d[9], d[10], d[11], d[12]);
  logInfoP("Device version: %02X%02X%02X%02X", d[13], d[14], d[15], d[16]);
  logInfoP("Description: %s", description);
}



// Sendet ein Funktelegramm (RADIO_ERP1) und wertet die RESPONSE des Transceivers aus (ESP3-Spec Kap. 2.1.2):
// RET_OK, RET_NOT_SUPPORTED, RET_WRONG_PARAM oder RET_LOCK_SET (Duty-Cycle-Sperre). Liefert true bei RET_OK.
bool EnoceanModule::sendRadioPacket(PACKET_SERIAL_TYPE_ *pPacket, const char *description)
{
    if (uart_sendPacket(pPacket) != ENOCEAN_OK) {
        logErrorP("Sending %s failed", description);
        return false;
    }

    if (waitForResponse(ENOCEAN_RESPONSE_TIMEOUT) != ENOCEAN_OK) {
        logErrorP("No response for %s", description);
        return false;
    }

    switch (m_Pkt_st.u8DataBuffer[0]) {
    case 0x00: // RET_OK
        logDebugP("Sent %s", description);
        return true;
    case 0x05: // RET_LOCK_SET
        logErrorP("Sending %s failed: duty cycle lock", description);
        return false;
    default:
        logErrorP("Sending %s failed, return code: %u", description, m_Pkt_st.u8DataBuffer[0]);
        return false;
    }
}

// Sendet ein Funktelegramm mit der Sender-ID Base-ID + Kanalindex, damit jeder Kanal eine eigene, in Aktoren
// einlernbare ID hat. data: Nutzdaten ohne R-ORG (z.B. 1 Byte RPS, 4 Byte 4BS). destId: nullptr = Broadcast, sonst
// adressiert an dieses Gerät (Optional Data mit Ziel-ID, der Transceiver sendet dann ein ADT-Telegramm).
bool EnoceanModule::sendChannelTelegram(uint8_t channelIndex, uint8_t rorg, const uint8_t *data, uint8_t dataLength, uint8_t status, const char *description, const uint8_t *destId)
{
    PACKET_SERIAL_TYPE_ lPacket_st;
    uint8_t lBuf_p[1 + 4 + 4 + 1 + 7]; // R-ORG + max. 4 Byte Daten + Sender-ID + Status + Optional Data
    if (dataLength > 4)
        return false;

    lPacket_st.u16DataLength = 1 + dataLength + 4 + 1;
    lPacket_st.u8OptionLength = destId ? 7 : 0;
    lPacket_st.u8Type = u8RADIO_ERP1;
    lPacket_st.u8DataBuffer = &lBuf_p[0];

    uint8_t pos = 0;
    lBuf_p[pos++] = rorg;
    for (uint8_t i = 0; i < dataLength; i++)
        lBuf_p[pos++] = data[i];
    // Base-ID: untere 7 Bit sind 0 -> 128 Sende-IDs, Kanalindex 0..29 erzeugt keinen Übertrag
    for (uint8_t i = 0; i < 4; i++)
        lBuf_p[pos++] = lui8_BaseID_p[i];
    lBuf_p[pos - 1] += channelIndex;
    lBuf_p[pos] = status;

    if (destId) {
        // Optional Data: SubTelNum 3, Ziel-ID, dBm 0xFF (Senden), Security-Level 0
        lBuf_p[++pos] = 0x03;
        for (uint8_t i = 0; i < 4; i++)
            lBuf_p[++pos] = destId[i];
        lBuf_p[++pos] = 0xFF;
        lBuf_p[++pos] = 0x00;
    }

    return sendRadioPacket(&lPacket_st, description);
}

// Sendet ein Rocker-Telegramm (RPS, F6-02-01) mit der Sender-ID des Kanals.
// db0: Tastencode (z.B. 0x30 = AO gedrückt, 0x10 = AI gedrückt, 0x00 = losgelassen).
bool EnoceanModule::sendRockerTelegram(uint8_t channelIndex, uint8_t db0)
{
    logDebugP("Send rocker %02X, channel %u", db0, channelIndex + 1);
    // Status: T21 = 1, NU = 1 bei gedrückter Taste, NU = 0 beim Loslassen
    return sendChannelTelegram(channelIndex, u8RORG_RPS, &db0, 1, (db0 & 0x10) ? 0x30 : 0x20, "RPS telegram");
}

// Sendet ein 4BS-Telegramm (data = DB3..DB0) mit der Sender-ID des Kanals, z.B. Antworten an bidirektionale Geräte.
bool EnoceanModule::send4BSTelegram(uint8_t channelIndex, const uint8_t data[4], const char *description, const uint8_t *destId)
{
    logDebugP("Send 4BS %02X %02X %02X %02X, channel %u", data[0], data[1], data[2], data[3], channelIndex + 1);
    return sendChannelTelegram(channelIndex, u8RORG_4BS, data, 4, 0x00, description, destId);
}

void EnoceanModule::setStatusActors(uint8_t *mySenderId, uint8_t idExtra, bool state)
{
    PACKET_SERIAL_TYPE_ lRdBaseIDPkt_st;

    uint8_t lu8SndBuf[16];

    lRdBaseIDPkt_st.u16DataLength = 0x0009;
    lRdBaseIDPkt_st.u8OptionLength = 0x07;
    lRdBaseIDPkt_st.u8Type = u8RADIO_ERP1;
    lRdBaseIDPkt_st.u8DataBuffer = &lu8SndBuf[0];

    lu8SndBuf[0] = u8RORG_VLD;
    lu8SndBuf[1] = VLD_CMD_ID_01;
    lu8SndBuf[2] = 0x1E; // Channel
    if (state == true)
        lu8SndBuf[3] = 0x01;
    else
        lu8SndBuf[3] = 0x00;
    lu8SndBuf[4] = mySenderId[0];
    lu8SndBuf[5] = mySenderId[1];
    lu8SndBuf[6] = mySenderId[2];
    lu8SndBuf[7] = mySenderId[3] + idExtra;
    lu8SndBuf[8] = 0x00;
    // optional data
    lu8SndBuf[9] = 0x03;
    lu8SndBuf[10] = 0xFF;
    lu8SndBuf[11] = 0xFF;
    lu8SndBuf[12] = 0xFF;
    lu8SndBuf[13] = 0xFF;
    lu8SndBuf[14] = 0x00;
    lu8SndBuf[15] = 0x00;

    sendRadioPacket(&lRdBaseIDPkt_st, "set actor status");
}

void EnoceanModule::getStatusActors(uint8_t *mySenderId, uint8_t idExtra)
{
    PACKET_SERIAL_TYPE_ lRdBaseIDPkt_st;

    uint8_t lu8SndBuf[15];

    lRdBaseIDPkt_st.u16DataLength = 0x0008;
    lRdBaseIDPkt_st.u8OptionLength = 0x07;
    lRdBaseIDPkt_st.u8Type = u8RADIO_ERP1;
    lRdBaseIDPkt_st.u8DataBuffer = &lu8SndBuf[0];

    lu8SndBuf[0] = u8RORG_VLD;
    lu8SndBuf[1] = VLD_CMD_ID_03;
    lu8SndBuf[2] = 0x1e;

    lu8SndBuf[3] = mySenderId[0];
    lu8SndBuf[4] = mySenderId[1];
    lu8SndBuf[5] = mySenderId[2];
    lu8SndBuf[6] = mySenderId[3] + idExtra;
    lu8SndBuf[7] = 0x00;
    // optional data
    lu8SndBuf[8] = 0x03;
    lu8SndBuf[9] = 0xFF;
    lu8SndBuf[10] = 0xFF;
    lu8SndBuf[11] = 0xFF;
    lu8SndBuf[12] = 0xFF;
    lu8SndBuf[13] = 0x00;
    lu8SndBuf[14] = 0x00;

    sendRadioPacket(&lRdBaseIDPkt_st, "request actor status");
}

void EnoceanModule::setActorsMeasurment(uint8_t *mySenderId, uint8_t idExtra, uint8_t *inputs)
{
    PACKET_SERIAL_TYPE_ lRdBaseIDPkt_st;

    uint8_t lu8SndBuf[19];

    lRdBaseIDPkt_st.u16DataLength = 0x000C;
    lRdBaseIDPkt_st.u8OptionLength = 0x07;
    lRdBaseIDPkt_st.u8Type = u8RADIO_ERP1;
    lRdBaseIDPkt_st.u8DataBuffer = &lu8SndBuf[0];

    lu8SndBuf[0] = u8RORG_VLD;
    lu8SndBuf[1] = VLD_CMD_ID_05;
    lu8SndBuf[2] = inputs[0];
    lu8SndBuf[3] = inputs[1];
    lu8SndBuf[4] = inputs[2];
    lu8SndBuf[5] = inputs[3];
    lu8SndBuf[6] = inputs[4];
    lu8SndBuf[7] = mySenderId[0];
    lu8SndBuf[8] = mySenderId[1];
    lu8SndBuf[9] = mySenderId[2];
    lu8SndBuf[10] = mySenderId[3] + idExtra;
    lu8SndBuf[11] = 0x00;
    // optional data
    lu8SndBuf[12] = 0x03;
    lu8SndBuf[13] = 0xFF;
    lu8SndBuf[14] = 0xFF;
    lu8SndBuf[15] = 0xFF;
    lu8SndBuf[16] = 0xFF;
    lu8SndBuf[17] = 0x00;
    lu8SndBuf[18] = 0x00;

    sendRadioPacket(&lRdBaseIDPkt_st, "measurement setup");
}

void EnoceanModule::getActorsMeasurmentValue(uint8_t *mySenderId, uint8_t idExtra, uint8_t *inputs, bool unit)
{
    PACKET_SERIAL_TYPE_ lRdBaseIDPkt_st;

    uint8_t lu8SndBuf[15];

    lRdBaseIDPkt_st.u16DataLength = 0x0008;
    lRdBaseIDPkt_st.u8OptionLength = 0x07;
    lRdBaseIDPkt_st.u8Type = u8RADIO_ERP1;
    lRdBaseIDPkt_st.u8DataBuffer = &lu8SndBuf[0];

    lu8SndBuf[0] = u8RORG_VLD;
    lu8SndBuf[1] = VLD_CMD_ID_06;
    lu8SndBuf[2] = 0x00; // 0x00 = Energy
    if (unit == 1)
        lu8SndBuf[2] = 0x20; // 0x20 = Power
    lu8SndBuf[2] = lu8SndBuf[2] + 0x1E;
    lu8SndBuf[3] = mySenderId[0];
    lu8SndBuf[4] = mySenderId[1];
    lu8SndBuf[5] = mySenderId[2];
    lu8SndBuf[6] = mySenderId[3] + idExtra;
    lu8SndBuf[7] = 0x00;
    // optional data
    lu8SndBuf[8] = 0x03;
    lu8SndBuf[9] = 0xFF;
    lu8SndBuf[10] = 0xFF;
    lu8SndBuf[11] = 0xFF;
    lu8SndBuf[12] = 0xFF;
    lu8SndBuf[13] = 0x00;
    lu8SndBuf[14] = 0x00;

    sendRadioPacket(&lRdBaseIDPkt_st, "request measurement");
}


// Schreibt Repeater-Mode (0 = aus, 1 = an, 2 = selektiv) und -Level (1/2) in den Transceiver (CO_WR_REPEATER).
void EnoceanModule::setRepeaterFunc(uint8_t mode, uint8_t level)
{
  PACKET_SERIAL_TYPE_ lRdBaseIDPkt_st;

  uint8_t lu8SndBuf[3];

  lu8SndBuf[0] = u8CO_WR_REPEATER;
  lu8SndBuf[1] = mode;
  // ESP3-Spec: bei Repeater Mode OFF muss das Level 0x00 sein (ETS bietet nur Level 1/2 an)
  lu8SndBuf[2] = mode ? level : 0x00;

  lRdBaseIDPkt_st.u16DataLength = 0x0003;
  lRdBaseIDPkt_st.u8OptionLength = 0x00;
  lRdBaseIDPkt_st.u8Type = u8RORG_COMMON_COMMAND;
  lRdBaseIDPkt_st.u8DataBuffer = &lu8SndBuf[0];

  logInfoP("Sending telegram (Repeater ON/OFF).");

  if (ENOCEAN_OK == uart_sendPacket(&lRdBaseIDPkt_st))
  {
    logInfoP("Receiving telegram (Repeater ON/OFF).");
    u8RetVal = waitForResponse(ENOCEAN_RESPONSE_TIMEOUT);

    switch (u8RetVal)
    {
    case ENOCEAN_OK:
    {
      {
        std::string hexStr;
        char hexByte[4];
        for (int i = 0; i < m_Pkt_st.u16DataLength + (uint16_t)m_Pkt_st.u8OptionLength; i++)
        {
          sprintf(hexByte, "%02X ", m_Pkt_st.u8DataBuffer[i]);
          hexStr += hexByte;
        }
        logInfoP("Data: %s", hexStr.c_str());
      }

      switch (m_Pkt_st.u8Type)
      {
      case u8RESPONSE:
      {
        switch (m_Pkt_st.u8DataBuffer[0])
        {
        case 0x00:
          logInfoP("Received Response = OK");
          break;
        case 0x02:
          logInfoP("Received Response = RET_NOT_SUPPORTED");
          break;
        case 0x03:
          logInfoP("Received Response = RET_WRONG_PARAM");
          break;
        default:
          break;
        }
      }
      break;
      default:
      {
        logInfoP("Wrong packet type. Expected response. Received: %u", m_Pkt_st.u8Type);
      }
      }
    }
    break;
    case ENOCEAN_NO_RX_TEL:
      logInfoP("ERROR Receiving telegram (read base ID).");
      break;
    default:
    {
      logInfoP("Error receiving base ID");
    }
    }
  }
}

void EnoceanModule::readRepeaterFunc()
{
  PACKET_SERIAL_TYPE_ lRdBaseIDPkt_st;

  uint8_t lu8SndBuf[1];
  // uint8_t loopCount = 0;

  lu8SndBuf[0] = u8CO_RD_REPEATER;

  lRdBaseIDPkt_st.u16DataLength = 0x0001;
  lRdBaseIDPkt_st.u8OptionLength = 0x00;
  lRdBaseIDPkt_st.u8Type = u8RORG_COMMON_COMMAND;
  lRdBaseIDPkt_st.u8DataBuffer = &lu8SndBuf[0];

  logInfoP("Sending telegram (Repeater ON/OFF).");

  if (ENOCEAN_OK == uart_sendPacket(&lRdBaseIDPkt_st))
  {
    logInfoP("Receiving telegram (Repeater ON/OFF).");
    u8RetVal = waitForResponse(ENOCEAN_RESPONSE_TIMEOUT);

    switch (u8RetVal)
    {
    case ENOCEAN_OK:
    {
      {
        std::string hexStr;
        char hexByte[4];
        for (int i = 0; i < m_Pkt_st.u16DataLength + (uint16_t)m_Pkt_st.u8OptionLength; i++)
        {
          sprintf(hexByte, "%02X ", m_Pkt_st.u8DataBuffer[i]);
          hexStr += hexByte;
        }
        logInfoP("Data: %s", hexStr.c_str());
      }

      switch (m_Pkt_st.u8Type)
      {
      case u8RESPONSE:
      {
        if (m_Pkt_st.u8DataBuffer[0] == 0x00)
        {
          logInfoP("Received Response = OK");
          switch (m_Pkt_st.u8DataBuffer[1])
          {
          case 0x00:
            logInfoP("Repeater = OFF");
            break;
          case 0x01:
            logInfoP("Repeater = ON");
            break;
          case 0x02:
            logInfoP("Repeater = Seletive");
            break;
          default:
            break;
          }
          switch (m_Pkt_st.u8DataBuffer[2])
          {
          case 0x01:
            logInfoP("Repeater = Level-1");
            break;
          case 0x02:
            logInfoP("Repeater = Level-2");
            break;
          default:
            break;
          }
        }
      }
      break;
      default:
      {
        logInfoP("Wrong packet type. Expected response. Received: %u", m_Pkt_st.u8Type);
      }
      }
    }
    break;
    case ENOCEAN_NO_RX_TEL:
      logInfoP("ERROR Receiving telegram (read base ID).");
      break;
    default:
    {
      logInfoP("Error receiving base ID");
    }
    }
  }
}

EnoceanModule openknxEnoceanModule;
