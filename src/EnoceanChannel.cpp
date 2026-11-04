#include "EnoceanChannel.h"
#include "1BS_Telegram.h"
#include "4BS_Telegram.h"
#include "Arduino.h"
#include "EnoceanModule.h"
#include "Product_Eltako.h"
#include "RPS_Telegram.h"
#include "VLD_Telegram.h"
#include "knxprod.h"

EnoceanHandle unionMSG;

// Konstruktor: speichert den Kanal-Index dieses EnoceanChannel-Objekts.
EnoceanChannel::EnoceanChannel(uint8_t index) { _channelIndex = index; }

// Liefert den Anzeigenamen des Kanals für Logging/Debug-Ausgaben.
const std::string EnoceanChannel::name() { return "EnoceanChannel"; }

// Wird einmalig beim Start des Kanals aufgerufen: startet die Ausfallüberwachung ab dem Neustart.
void EnoceanChannel::setup() {
  logDebugP("setup");
  _lastSeen = millis();
  _setupTime = millis();
}

// Das Gerät dieses Kanals hat gesendet: Ausfallüberwachung neu starten und einen gemeldeten Ausfall zurücknehmen.
void EnoceanChannel::deviceSeen() {
  _lastSeen = millis();
  if (_failed) {
    _failed = false;
    logInfoP("Device available again");
    KoENO_CHFailure.value(false, Dpt(1, 5));
  }
}

// Meldet über KO_10 einen Ausfall, wenn das Gerät länger als die per ETS eingestellte Zeit nichts gesendet hat.
void EnoceanChannel::checkFailure() {
  uint8_t hours = ParamENO_CHMonitorTime;
  if (hours == 0 || _failed || ParamENO_CHProductsel == CH_inaktive)
    return;
  if (delayCheck(_lastSeen, hours * 3600000UL)) {
    _failed = true;
    logInfoP("Device failure: no telegram for %u h", hours);
    KoENO_CHFailure.value(true, Dpt(1, 5));
  }
}

// Wird zyklisch aufgerufen: sendet bei der Taster-Emulation (KNX -> EnOcean) das ausstehende Loslass-Telegramm und
// erkennt bei gedrücktem Taster (EnOcean -> KNX) den langen Tastendruck (Zeit per ETS-Parameter in 10 ms).
void EnoceanChannel::loop() {
  checkFailure();

  // Antwort an ein bidirektionales Gerät: wird beim Empfang vorbereitet und hier gesendet, weil der Empfangspuffer
  // beim Senden für die Antwort des Transceivers wiederverwendet wird (Gerät hört ca. 1 s nach seinem Telegramm zu)
  sendPendingHvacReply();

  // D2-01 mit konfigurierbarer Messung: Messkonfiguration nach dem Start, je Kanal versetzt, damit nicht alle Kanäle gleichzeitig senden
  if (!_vldMeasurementSetupDone && delayCheck(_setupTime, 5000 + _channelIndex * 500UL)) {
    _vldMeasurementSetupDone = true;
    if (vldProfile() != 0 && d201Caps().meas == D201_MEAS_CONFIG)
      sendVldMeasurementSetup();
  }

  if (_knxRockerReleaseAt != 0 && (int32_t)(millis() - _knxRockerReleaseAt) >= 0) {
    _knxRockerReleaseAt = 0;
    openknxEnoceanModule.sendRockerTelegram(_channelIndex, 0x00);
  }

  if (_rockerWippe == ROCKER_NONE || _rockerLong)
    return;

  if (!rockerNeedsLongPress(rockerFunction(_rockerWippe, _channelIndex)))
    return;

  if (delayCheck(_rockerPressTime, ParamENO_CHRockerLongPressWaitTime * 10)) {
    _rockerLong = true;
    rockerLongStart(_rockerWippe, _rockerUpper, _channelIndex);
  }
}

// Sendet "Taste gedrückt" und plant das Loslass-Telegramm nach releaseAfterMs (0 = Taste bleibt gedrückt, bis ein
// Loslassen explizit angefordert wird).
void EnoceanChannel::knxRockerPress(uint8_t db0, uint32_t releaseAfterMs) {
  _knxRockerLastDb0 = db0;
  openknxEnoceanModule.sendRockerTelegram(_channelIndex, db0);
  _knxRockerReleaseAt = millis() + releaseAfterMs;
  if (_knxRockerReleaseAt == 0)
    _knxRockerReleaseAt = 1; // 0 ist "kein Loslassen ausstehend"
}

// Sendet ein Lerntelegramm mit der Sender-ID dieses Kanals (Base-ID + Kanalindex), damit ein EnOcean-Aktor im
// Lernmodus den Kanal einlernen kann (ETS-Button / Konsole "eno learn N"). Taster in Richtung KNX -> EnOcean:
// Tastendruck oben (AO), danach Loslassen. D2-01-Aktor: UTE-Teach-In-Anfrage mit dem EEP des Kanals.
// A5-38-08-Aktor: 4BS-Lerntelegramm (Variante 3) mit EEP A5-38-08, Hersteller Eltako (0x00D) bzw. Multi-User (0x7FF).
// Liefert false, wenn der Kanal nicht sendet.
bool EnoceanChannel::sendLearnTelegram() {
  uint16_t vld = vldProfile();
  if (vld != 0)
    return openknxEnoceanModule.sendUTETeachIn(_channelIndex, 0x01, (uint8_t)(vld - D2_01_00));

  if (actuatorType() == ACT_SHUTTER) {
    // Eltako Jalousieaktor: Lerntelegramm laut "Inhalte der Eltako-Funktelegramme" (FUNC 3F, TYPE 7F, Eltako)
    const uint8_t data[4] = {0xFF, 0xF8, 0x0D, 0x80};
    logInfoP("Send learn telegram A5-3F-7F (Eltako shutter)");
    return openknxEnoceanModule.send4BSTelegram(_channelIndex, data, "A5-3F-7F teach-in");
  }

  if (actuatorIsEltako() && ParamENO_CHProductselType == Eltako_Aktor && ParamENO_CHProductselTypeAkt == Eltako_Aktor_FSUD) {
    // FSUD-230V weicht ab: Lerntelegramm laut "Inhalte der Eltako-Funktelegramme"
    const uint8_t data[4] = {0x02, 0x00, 0x00, 0x00};
    logInfoP("Send learn telegram (Eltako FSUD-230V)");
    return openknxEnoceanModule.send4BSTelegram(_channelIndex, data, "FSUD teach-in");
  }

  if (actuatorType() != ACT_NONE) {
    // DB3 FUNC 0x38 / TYPE (Bit 6-5), DB2 TYPE (Bit 4-0) / Hersteller (Bit 10-8), DB1 Hersteller (Bit 7-0), DB0 LRN-Typ 1
    const uint8_t data[4] = {0xE0, (uint8_t)(actuatorIsEltako() ? 0x40 : 0x47), (uint8_t)(actuatorIsEltako() ? 0x0D : 0xFF), 0x80};
    logInfoP("Send learn telegram A5-38-08 (%s)", actuatorIsEltako() ? "Eltako" : "EEP");
    return openknxEnoceanModule.send4BSTelegram(_channelIndex, data, "A5-38-08 teach-in");
  }

  if (ParamENO_CHProductsel != EEP_Profil || ParamENO_CHProfilSelection != u8RORG_Rocker || ParamENO_CHDirectionKnxEnocean != 1)
    return false;

  logInfoP("Send learn telegram (rocker AO)");
  knxRockerPress(0x30, ROCKER_TAP_MS);
  return true;
}

// Taster-Emulation KNX -> EnOcean für das Eingangs-KO KO_0, Funktion per ETS (ProfilRockerFunktionKNX):
// 1 = Schalten (DPT 1.001): EIN = Taste oben (AO), AUS = Taste unten (AI), jeweils kurz getippt.
// 2 = Dimmen (DPT 3.007): Start = Taste halten (heller oben / dunkler unten), Stopp = loslassen.
// 3 = Jalousie (DPT 3.008): Fahren = kurz tippen (AB unten / AUF oben), Stopp = erneut in letzter Richtung tippen.
void EnoceanChannel::processKnxRocker(GroupObject &ko) {
  switch (ParamENO_CHProfilRockerFunktionKNX) {
  case 1: { // Schalten
    bool on = ko.value(Dpt(1, 1));
    logDebugP("KNX -> EnOcean: Schalten %u", on);
    knxRockerPress(on ? 0x30 : 0x10, ROCKER_TAP_MS);
    break;
  }
  case 2: { // Dimmen
    bool up = ko.value(Dpt(3, 7, 0));
    uint8_t step = ko.value(Dpt(3, 7, 1));
    logDebugP("KNX -> EnOcean: Dimmen %s, Step %u", up ? "heller" : "dunkler", step);
    if (step == 0)
      _knxRockerReleaseAt = millis(); // Stopp: sofort loslassen (in loop())
    else
      knxRockerPress(up ? 0x30 : 0x10, ROCKER_HOLD_MAX_MS);
    break;
  }
  case 3: { // Jalousie: DPT 3.008 Richtungsbit 1 = AB
    bool down = ko.value(Dpt(3, 8, 0));
    uint8_t step = ko.value(Dpt(3, 8, 1));
    logDebugP("KNX -> EnOcean: Jalousie %s, Step %u", down ? "AB" : "AUF", step);
    if (step == 0) {
      if (_knxRockerLastDb0 != 0)
        knxRockerPress(_knxRockerLastDb0, ROCKER_TAP_MS); // Stopp: in letzter Fahrtrichtung tippen
    } else
      knxRockerPress(down ? 0x10 : 0x30, ROCKER_TAP_MS);
    break;
  }
  }
}

// Verarbeitet ein Rocker-Telegramm (F6-02-xx) dieses Kanals: Drücken merkt sich Wippe/Seite und löst Schalten/Szene
// sofort aus, Loslassen wertet kurzen bzw. langen Tastendruck aus (langer Druck wird in loop() erkannt).
void EnoceanChannel::handleRocker(PACKET_SERIAL_TYPE_ *pPacket) {
  uint8_t db0 = pPacket->u8DataBuffer[1];
  bool pressed = db0 & 0x10; // EB: Energy Bow

  if (pressed) {
    bool upper = false;
    uint8_t wippe = rockerDecode(db0, &upper, _channelIndex);
    logDebugP("Rocker pressed: DB0 %02X, Wippe %u, %s", db0, wippe, upper ? "oben" : "unten");
    if (wippe == ROCKER_NONE)
      return;

    _rockerWippe = wippe;
    _rockerUpper = upper;
    _rockerLong = false;
    _rockerPressTime = millis();
    rockerPress(wippe, upper, _channelIndex);
    return;
  }

  // losgelassen
  logDebugP("Rocker released");
  if (_rockerWippe == ROCKER_NONE)
    return;

  if (_rockerLong)
    rockerLongEnd(_rockerWippe, _channelIndex);
  else
    rockerShort(_rockerWippe, _rockerUpper, _channelIndex);

  _rockerWippe = ROCKER_NONE;
}

// Prüft anhand der EnOcean-ID und des RORG-Typs, ob ein empfangenes Paket zu diesem Kanal gehört,
// und leitet es je nach Profil (1BS/RPS/4BS/VLD) an den passenden Telegramm-Handler weiter.
bool EnoceanChannel::check_Eno_ID(PACKET_SERIAL_TYPE_ *pPacket) {
  // init parameter
  unionMSG.msg_sent_after_receive = 0;

  // Get rid of messages we can't handle
  if (0 == ParamENO_CHProductsel) {
    return false;
  }

  // Get rid of messages not intended for us: die Sender-ID (Position abhängig vom RORG, siehe
  // EnoceanModule::extractSenderId) muss mit der parametrierten ENOCEAN-ID übereinstimmen - für EEP-Profile
  // wie für Hersteller-Produkte.
  uint8_t senderId[4];
  if (!EnoceanModule::extractSenderId(pPacket, senderId) || !matchesDeviceId(senderId)) {
    return false;
  }

  logDebugP("ENO-ID:");
  logHexDebugP(senderId, 4);
  deviceSeen();

  // Bidirektionale Geräte (EEP A5-20-01/04/06 oder Eltako FKS-SV) brauchen eine Antwort im Empfangsfenster
  if (hvacProfile() != HVAC_NONE) {
    handleHvac(pPacket);
    return true;
  }

  // Aktor mit Zentralbefehl A5-38-08 (EEP-Profil oder Eltako-Produkt): Bestätigungstelegramme als Status auswerten
  if (actuatorType() != ACT_NONE) {
    handleActuatorStatus(pPacket);
    return true;
  }

  // Prüft wie der CH konfiguriert ist. Über EEP-Profil oder über Hersteller
  switch (ParamENO_CHProductsel) {
  case CH_inaktive: // Kein Profil oder Hersteller gewählt
    return false;
    break;

  case EEP_Profil: // EEP Profile

    // UTE-Antwort eines VLD-Aktors auf die Teach-In-Anfrage ("eno learn"): DB6 Bit 3-0 Kommando 1, Bit 5-4 Ergebnis
    if (pPacket->u8DataBuffer[0] == u8RORG_UTE && ParamENO_CHProfilSelection == u8RORG_VLD) {
      static const char *uteResult[] = {"rejected", "teach-in accepted", "deletion accepted", "EEP not supported"};
      if ((pPacket->u8DataBuffer[1] & 0x0F) == 0x01)
        logInfoP("UTE response: %s", uteResult[(pPacket->u8DataBuffer[1] >> 4) & 0x03]);
      return true;
    }

    // D2-01-Aktor: Status- und Messwert-Antworten
    if (pPacket->u8DataBuffer[0] == u8RORG_VLD && vldProfile() != 0) {
      handleD201(pPacket);
      return true;
    }

    // Get rid of messages we can't handle
    if (pPacket->u8DataBuffer[0] != ParamENO_CHProfilSelection && ParamENO_CHProfilSelection != u8RORG_Rocker) {
      logDebugP("wrong Profil");
      return false;
    }

    switch (ParamENO_CHProfilSelection) {
    case u8RORG_1BS:
      // Verarbeitet ein empfangenes 1BS-EnOcean-Telegramm (D5-00-01 Kontakt) und schreibt den Open/Close-Zustand auf
      // das KNX-Objekt
      handle_1BS(pPacket, _channelIndex);
      return true;
      break;

    case u8RORG_RPS:
      handle_RPS(pPacket, _channelIndex);
      return true;
      break;

    case u8RORG_4BS:
      unionMSG.msg_sent_after_receive = handle_4BS(pPacket, _channelIndex);
      return true;

      break;

    case u8RORG_VLD:
      handle_VLD(pPacket, _channelIndex);
      return true;
      break;

    case u8RORG_Rocker:
      // Taster: nur RPS-Telegramme, nur Richtung EnOcean -> KNX (0); KNX -> EnOcean siehe handleKnxEvent()
      if (pPacket->u8DataBuffer[0] != u8RORG_RPS || ParamENO_CHDirectionKnxEnocean != 0)
        return false;
      handleRocker(pPacket);
      return true;
    } // ENDE switch (ParamENO_CHProfilSelection)
    break; // ENDE EEP-Profil

  case Hersteller_Eltako: // Hersteller Eltako
    handleProductEltako(pPacket, _channelIndex);
    return true;
    break; // ENDE Hersteller ELTAKO

  default:
    break;
  } // ENDE SWITCH(ParamENO_CHProductsel)

  return false;
}

// Vergleicht die EnOcean-ID dieses Kanals mit der übergebenen ID (unabhängig vom gewählten Profil), um
// kanalübergreifend feststellen zu können, ob eine empfangene EnOcean-ID bereits bekannt ist.
// ID-Quelle: die ETS-Parameter, wenn dort eine ID steht; sonst im Teach-In-Modus die im Gateway eingelernte ID
// (Brücke bis zum nächsten ETS-Download, siehe "Ergebnis übernehmen").
bool EnoceanChannel::matchesDeviceId(const uint8_t id[4]) const {
  uint8_t configuredId[4];
  return deviceId(configuredId) && memcmp(configuredId, id, 4) == 0;
}

// Liefert die EnOcean-ID des Geräts dieses Kanals (Quelle wie bei matchesDeviceId()), false wenn keine bekannt ist.
bool EnoceanChannel::deviceId(uint8_t out8Id[4]) const {
  if (ParamENO_CHProductsel == CH_inaktive)
    return false;

  out8Id[0] = (ParamENO_CHId0 << 4) | ParamENO_CHId1;
  out8Id[1] = (ParamENO_CHId2 << 4) | ParamENO_CHId3;
  out8Id[2] = (ParamENO_CHId4 << 4) | ParamENO_CHId5;
  out8Id[3] = (ParamENO_CHId6 << 4) | ParamENO_CHId7;

  if (out8Id[0] | out8Id[1] | out8Id[2] | out8Id[3])
    return true;
  return ParamENO_CHSetupMode == ENO_SETUP_TEACHIN && openknxEnoceanModule.learnedId(_channelIndex, out8Id);
}

// Bidirektionaler Heizkörper-Stellantrieb dieses Kanals (HVAC_*): EEP A5-20-01/04/06, Eltako FKS-SV (A5-20-01) oder FKS-B (A5-20-04).
uint8_t EnoceanChannel::hvacProfile() const {
  switch (ParamENO_CHProductsel) {
  case EEP_Profil:
    if (ParamENO_CHProfilSelection != u8RORG_4BS || ParamENO_CHProfilSelection4BS != A5_20)
      return HVAC_NONE;
    switch (ParamENO_CHProfil4BS20) {
    case A5_20_01:
      return HVAC_A52001;
    case A5_20_04:
      return HVAC_A52004;
    case A5_20_06:
      return HVAC_A52006;
    default:
      return HVAC_NONE;
    }
  case Hersteller_Eltako:
    if (ParamENO_CHProductselType == Eltako_Heizung) {
      if (ParamENO_CHProductselTypeHei == Eltako_Heizung_FKS_SV)
        return HVAC_A52001;
      if (ParamENO_CHProductselTypeHei == Eltako_Heizung_FKS_B)
        return HVAC_A52004;
    }
    return HVAC_NONE;
  default:
    return HVAC_NONE;
  }
}

static const char *hvacName(uint8_t profile) {
  switch (profile) {
  case HVAC_A52001:
    return "A5-20-01";
  case HVAC_A52004:
    return "A5-20-04";
  case HVAC_A52006:
    return "A5-20-06";
  default:
    return "HVAC";
  }
}

// Telegramm des Antriebs: KOs aktualisieren und die Antwort vorbereiten (Teach-In-Antwort oder Sollwert).
void EnoceanChannel::handleHvac(PACKET_SERIAL_TYPE_ *pPacket) {
  if (pPacket->u8DataBuffer[0] != u8RORG_4BS) {
    logDebugP("%s: R-ORG %02X ignored", hvacName(hvacProfile()), pPacket->u8DataBuffer[0]);
    return;
  }

  _hvacRxTime = millis();
  EnoceanModule::extractSenderId(pPacket, _hvacDest);

  uint8_t result = 0;
  switch (hvacProfile()) {
  case HVAC_A52001:
    result = handle4BS_A5_20_01(pPacket, _channelIndex);
    break;
  case HVAC_A52004:
    result = handle4BS_A5_20_04(pPacket, _channelIndex);
    break;
  case HVAC_A52006:
    result = handle4BS_A5_20_06(pPacket, _channelIndex);
    break;
  default:
    return;
  }

  if (result == TEACHIN_A52001 || result == TEACHIN_A52004 || result == TEACHIN_A52006)
    prepareHvacTeachResponse(pPacket);
  else if (result != 0)
    prepareHvacReply(pPacket);
}

// Das Gateway hat im Teach-In-Fenster ein Lerntelegramm diesem Kanal zugeordnet. Bidirektionale Geräte erwarten
// darauf eine Teach-In-Antwort, sonst brechen sie das Einlernen ab. Sie wird hier sofort gesendet (der Aufrufer
// verarbeitet das Telegramm danach nicht weiter, der Empfangspuffer darf also überschrieben werden).
void EnoceanChannel::teachInReceived(PACKET_SERIAL_TYPE_ *pPacket) {
  if (hvacProfile() == HVAC_NONE) {
    logInfoP("Teach-in: channel has no bidirectional profile, no teach-in response");
    return;
  }
  handleHvac(pPacket);
  sendPendingHvacReply();
}

// Sendet eine vorbereitete Antwort an den Antrieb. Teach-In-Antworten und Antworten an A5-20-04 (laut EEP Unicast)
// gehen adressiert an den Antrieb, die übrigen als Broadcast.
void EnoceanChannel::sendPendingHvacReply() {
  if (!_hvacReplyPending)
    return;
  _hvacReplyPending = false;

  const char *name = hvacName(hvacProfile());
  bool addressed = _hvacReplyIsTeach || hvacProfile() == HVAC_A52004;
  bool ok = openknxEnoceanModule.send4BSTelegram(_channelIndex, _hvacReply, _hvacReplyIsTeach ? "HVAC teach-in response" : "HVAC reply",
                                                 addressed ? _hvacDest : nullptr);
  logInfoP("%s %s %02X %02X %02X %02X sent %u ms after receive%s", name, _hvacReplyIsTeach ? "teach-in response" : "reply", _hvacReply[0],
           _hvacReply[1], _hvacReply[2], _hvacReply[3], (uint32_t)(millis() - _hvacRxTime), ok ? "" : " (FAILED)");
  if (ok && !_hvacReplyIsTeach)
    _hvacOneShot = 0;
}

// Teach-In-Antwort (4BS, Variante 3): EEP und Hersteller wie in der Anfrage, DB0 = LRN-Typ 1, EEP unterstützt,
// Einlernen erfolgreich, Antwort, LRN-Bit 0. Sender-ID ist die Kanal-ID (Base-ID + Kanalindex).
void EnoceanChannel::prepareHvacTeachResponse(PACKET_SERIAL_TYPE_ *pPacket) {
  _hvacReply[0] = pPacket->u8DataBuffer[1];
  _hvacReply[1] = pPacket->u8DataBuffer[2];
  _hvacReply[2] = pPacket->u8DataBuffer[3];
  _hvacReply[3] = 0xF0;
  _hvacReplyIsTeach = true;
  _hvacReplyPending = true;
  logInfoP("%s teach-in request, sending response", hvacName(hvacProfile()));
}

// Antwort auf ein Datentelegramm (Belegung siehe handle4BS_A5_20_xx in 4BS_Telegram.h). Ohne Sollwert von KNX (z.B.
// nach einem Neustart) wird nicht geantwortet, der Antrieb behält dann sein eigenes Verhalten.
void EnoceanChannel::prepareHvacReply(PACKET_SERIAL_TYPE_ *pPacket) {
  const uint8_t *d = pPacket->u8DataBuffer; // d[1] = DB3 ... d[4] = DB0
  uint8_t profile = hvacProfile();

  // Am Gerät verstellte Solltemperatur übernehmen, damit die Antwort sie nicht gleich wieder zurücksetzt
  if (profile == HVAC_A52004 && (d[4] & 0x02)) {
    _hvacRoomTemp = d[2];
    _hvacRoomTempValid = true;
  }
  if (profile == HVAC_A52006 && _hvacSetpointValid && ParamENO_CHA52006SPS && (d[2] & 0x80) && (d[2] & 0x7F) <= 80)
    _hvacSetpoint = d[2] & 0x7F;

  if (!_hvacSetpointValid) {
    logDebugP("%s: no setpoint from KNX yet, no reply", hvacName(profile));
    return;
  }

  _hvacReply[0] = _hvacSetpoint;
  switch (profile) {
  case HVAC_A52001:
    // DB2 ist in Richtung 2 invertiert (255..0 = 0..40 °C), die gemessene Temperatur (DB1) in Richtung 1 linear
    _hvacReply[1] = _hvacRoomTempValid ? _hvacRoomTemp : (uint8_t)(255 - d[3]);
    _hvacReply[2] = _hvacFlags | _hvacOneShot | (ParamENO_CHA52001SPS ? A52001_DB1_SPS : 0);
    _hvacReply[3] = 0x08;
    break;

  case HVAC_A52004: {
    // DB2 Solltemperatur nur für das Display (Standard 20 °C), DB1 Aufwachzyklus (KO oder ETS), Messung immer aktiv
    uint8_t wuc = (_hvacInterval != HVAC_INTERVAL_ETS) ? _hvacInterval : ParamENO_CHA52004WCU;
    _hvacReply[1] = _hvacRoomTempValid ? _hvacRoomTemp : 128;
    _hvacReply[2] = wuc & 0x3F;
    _hvacReply[3] = ((ParamENO_CHA52004DSO & 0x03) << 4) | 0x08 | _hvacFlags | _hvacOneShot;
    break;
  }

  case HVAC_A52006: {
    // DB2 0 = interner Fühler des Antriebs, DB1 Funkintervall (KO oder ETS)
    uint8_t rfc = (_hvacInterval != HVAC_INTERVAL_ETS) ? _hvacInterval : ParamENO_CHA52006RFC;
    _hvacReply[1] = _hvacRoomTempValid ? _hvacRoomTemp : 0;
    _hvacReply[2] = _hvacOneShot | ((rfc & 0x07) << 4) | _hvacFlags | (ParamENO_CHA52006SPS ? A52006_DB1_SPS : 0) |
                    (ParamENO_CHA52006TSL ? A52006_DB1_TSL : 0);
    _hvacReply[3] = 0x08;
    break;
  }

  default:
    return;
  }
  _hvacReplyIsTeach = false;
  _hvacReplyPending = true;
}

// Temperatur in °C auf einen linearen Wertebereich raw 0..rawMax = tMin..tMax umrechnen (begrenzt).
static uint8_t hvacTempToRaw(double temp, double tMin, double tMax, uint8_t rawMax) {
  if (temp < tMin)
    temp = tMin;
  if (temp > tMax)
    temp = tMax;
  return (uint8_t)round((temp - tMin) * rawMax / (tMax - tMin));
}

// Setzt oder löscht Bits in einem Flag-Byte.
static void hvacSetBits(uint8_t &flags, uint8_t bits, bool on) {
  if (on)
    flags |= bits;
  else
    flags &= ~bits;
}

// KNX -> Antrieb: Werte nur merken, gesendet werden sie mit der nächsten Antwort (prepareHvacReply).
// KO_0 Sollwert (Temperatur DPT 9.001 oder Ventilstellung DPT 5.001, je nach ETS) bei allen Profilen, außerdem:
// A5-20-01: KO_1 Sommerbetrieb, KO_2 Referenzfahrt, KO_3 Raumtemperatur, KO_4 Service (1 = Ventil schließen,
//           2 = Ventil öffnen, 3 = Hub einstellen).
// A5-20-04: KO_1 Solltemperatur für das Display, KO_2 Aufwachzyklus (0..63), KO_3 Tastensperre, KO_4 Service
//           (1 = Ventil öffnen, 2 = Initialisierung, 3 = Ventil schließen).
// A5-20-06: KO_1 Sommerbetrieb, KO_2 Funkintervall (0..7), KO_3 Raumtemperatur, KO_4 Standby, KO_5 Referenzfahrt.
// Liefert false, wenn der Kanal kein bidirektionales Profil hat.
bool EnoceanChannel::processKnxHvac(int koIndex, GroupObject &ko) {
  uint8_t profile = hvacProfile();
  if (profile == HVAC_NONE)
    return false;
  const char *name = hvacName(profile);

  if (koIndex == ENO_KoGO_BASE__1) {
    bool tempMode = (profile == HVAC_A52001 && ParamENO_CHA52001SPS) || (profile == HVAC_A52006 && ParamENO_CHA52006SPS);
    if (tempMode) {
      double temp = ko.value(Dpt(9, 1));
      _hvacSetpoint = hvacTempToRaw(temp, 0.0, 40.0, profile == HVAC_A52001 ? 255 : 80);
      logInfoP("%s setpoint %.1f °C (sent with next reply)", name, temp);
    } else {
      _hvacSetpoint = (uint8_t)ko.value(Dpt(5, 1)); // 0..100 %
      logInfoP("%s valve position %u %% (sent with next reply)", name, _hvacSetpoint);
    }
    _hvacSetpointValid = true;
    return true;
  }

  switch (profile) {
  case HVAC_A52001:
    switch (koIndex) {
    case ENO_KoGO_BASE__2:
      hvacSetBits(_hvacFlags, A52001_DB1_SB, ko.value(Dpt(1, 1)));
      logInfoP("%s summer mode %u", name, (_hvacFlags & A52001_DB1_SB) != 0);
      break;
    case ENO_KoGO_BASE__3:
      if ((bool)ko.value(Dpt(1, 1))) {
        _hvacOneShot |= A52001_DB1_RIN;
        logInfoP("%s reference run requested", name);
      }
      break;
    case ENO_KoGO_BASE__4: {
      double temp = ko.value(Dpt(9, 1));
      _hvacRoomTemp = 255 - hvacTempToRaw(temp, 0.0, 40.0, 255); // Richtung 2: 255..0 = 0..40 °C
      _hvacRoomTempValid = true;
      logDebugP("%s room temperature %.1f °C", name, temp);
      break;
    }
    case ENO_KoGO_BASE__5: {
      uint8_t cmd = (uint8_t)ko.value(Dpt(5, 10));
      static const uint8_t service[] = {0, A52001_DB1_VC, A52001_DB1_VO, A52001_DB1_LFS};
      _hvacOneShot = (_hvacOneShot & A52001_DB1_RIN) | (cmd < 4 ? service[cmd] : 0);
      logInfoP("%s service command %u", name, cmd);
      break;
    }
    default:
      break;
    }
    break;

  case HVAC_A52004:
    switch (koIndex) {
    case ENO_KoGO_BASE__2: {
      double temp = ko.value(Dpt(9, 1));
      _hvacRoomTemp = hvacTempToRaw(temp, 10.0, 30.0, 255);
      _hvacRoomTempValid = true;
      logInfoP("%s display setpoint %.1f °C", name, temp);
      break;
    }
    case ENO_KoGO_BASE__3: {
      uint8_t wuc = (uint8_t)ko.value(Dpt(5, 10));
      _hvacInterval = wuc > 63 ? 63 : wuc;
      logInfoP("%s wake-up cycle %u", name, _hvacInterval);
      break;
    }
    case ENO_KoGO_BASE__4:
      hvacSetBits(_hvacFlags, A52004_DB0_BLC, ko.value(Dpt(1, 1)));
      logInfoP("%s button lock %u", name, (_hvacFlags & A52004_DB0_BLC) != 0);
      break;
    case ENO_KoGO_BASE__5: {
      uint8_t cmd = (uint8_t)ko.value(Dpt(5, 10));
      _hvacOneShot = cmd < 4 ? cmd : 0;
      logInfoP("%s service command %u", name, cmd);
      break;
    }
    default:
      break;
    }
    break;

  case HVAC_A52006:
    switch (koIndex) {
    case ENO_KoGO_BASE__2:
      hvacSetBits(_hvacFlags, A52006_DB1_SB, ko.value(Dpt(1, 1)));
      logInfoP("%s summer mode %u", name, (_hvacFlags & A52006_DB1_SB) != 0);
      break;
    case ENO_KoGO_BASE__3: {
      uint8_t rfc = (uint8_t)ko.value(Dpt(5, 10));
      _hvacInterval = rfc > 7 ? 7 : rfc;
      logInfoP("%s radio interval %u", name, _hvacInterval);
      break;
    }
    case ENO_KoGO_BASE__4: {
      double temp = ko.value(Dpt(9, 1));
      _hvacRoomTemp = hvacTempToRaw(temp, 0.0, 40.0, 160);
      if (_hvacRoomTemp == 0)
        _hvacRoomTemp = 1; // 0 hieße "interner Fühler"
      _hvacRoomTempValid = true;
      logDebugP("%s room temperature %.1f °C", name, temp);
      break;
    }
    case ENO_KoGO_BASE__5:
      hvacSetBits(_hvacFlags, A52006_DB1_SBY, ko.value(Dpt(1, 1)));
      logInfoP("%s standby %u", name, (_hvacFlags & A52006_DB1_SBY) != 0);
      break;
    case ENO_KoGO_BASE__6:
      if ((bool)ko.value(Dpt(1, 1))) {
        _hvacOneShot |= A52006_DB1_REF;
        logInfoP("%s reference run requested", name);
      }
      break;
    default:
      break;
    }
    break;

  default:
    break;
  }
  return true;
}

// Eigenschaften der D2-01-Typen 0x00..0x13 laut EEP 2.6.7 (Tabelle "D2-01"): Anzahl Ausgänge, Dimmer, Messung.
// 0x14 (8 Kanäle) passt nicht in die 10 KOs eines Kanals und wird nicht unterstützt.
static const D201Caps d201CapsTable[] = {
    {1, false, D201_MEAS_ENERGY}, // 00
    {1, false, D201_MEAS_NONE},   // 01
    {1, true, D201_MEAS_ENERGY},  // 02
    {1, true, D201_MEAS_NONE},    // 03
    {1, true, D201_MEAS_POWER},   // 04
    {1, true, D201_MEAS_CONFIG},  // 05
    {1, false, D201_MEAS_ENERGY}, // 06
    {1, false, D201_MEAS_NONE},   // 07
    {1, false, D201_MEAS_CONFIG}, // 08
    {1, true, D201_MEAS_CONFIG},  // 09
    {1, false, D201_MEAS_NONE},   // 0A
    {1, false, D201_MEAS_CONFIG}, // 0B
    {1, false, D201_MEAS_CONFIG}, // 0C
    {1, false, D201_MEAS_NONE},   // 0D
    {1, false, D201_MEAS_CONFIG}, // 0E
    {1, false, D201_MEAS_NONE},   // 0F
    {2, false, D201_MEAS_NONE},   // 10 (Energiemessung des Geräts wird nicht ausgewertet)
    {2, false, D201_MEAS_NONE},   // 11
    {2, false, D201_MEAS_NONE},   // 12
    {4, false, D201_MEAS_NONE},   // 13
};

// D2-01-Aktor dieses Kanals: liefert das EEP (D2_01_00..D2_01_13) oder 0, wenn der Kanal keiner ist.
uint16_t EnoceanChannel::vldProfile() const {
  if (ParamENO_CHProductsel != EEP_Profil || ParamENO_CHProfilSelection != u8RORG_VLD || ParamENO_CHProfilSelectionVLD != D2_01)
    return 0;
  uint16_t profile = ParamENO_CHProfilVLD01;
  return (profile >= D2_01_00 && profile <= D2_01_13) ? profile : 0;
}

// Eigenschaften des D2-01-Typs dieses Kanals (nur aufrufen, wenn vldProfile() != 0).
const D201Caps &EnoceanChannel::d201Caps() const { return d201CapsTable[vldProfile() - D2_01_00]; }

// D2-01 CMD 0x01 (Actuator Set Output): DB1 Dimmwert 0 (sofort) + I/O-Kanal (0x1E = alle), DB0 Ausgangswert
// 0 = AUS, 1..100 % (Schaltaktor: EIN).
void EnoceanChannel::sendVldSetOutput(uint8_t ioChannel, uint8_t percent) {
  if (percent > 100)
    percent = 100;
  const uint8_t data[3] = {VLD_CMD_ID_01, (uint8_t)(ioChannel & 0x1F), percent};
  logInfoP("D2-01 set output %02X: %u %%", ioChannel, percent);
  openknxEnoceanModule.sendVLDTelegram(_channelIndex, data, 3, "D2-01 set output");
}

// D2-01 CMD 0x05 (Actuator Set Measurement) für Typen mit konfigurierbarer Messung, je ein Telegramm für Energie und
// Leistung: DB4 Bit 7 automatisch melden, Bit 5 Energie/Leistung, Bit 4-0 I/O-Kanal (alle); DB3 Bit 7-4 Delta
// (Bit 3-0), Bit 2-0 Einheit; DB2 Delta (Bit 11-4); DB1 max. Zeit zwischen zwei Meldungen (10 s); DB0 min. Zeit (1 s).
void EnoceanChannel::sendVldMeasurementSetup() {
  uint8_t mit = ParamENO_CHD2010EMIT;
  uint8_t mat = ParamENO_CHD2010EMAT;
  if (mit == 0)
    mit = 1;
  if (mat * 10 < mit)
    mat = (mit + 9) / 10; // max. Zeit muss >= min. Zeit sein
  bool report = ParamENO_CHD2010ERM;

  for (uint8_t power = 0; power < 2; power++) {
    uint16_t delta = power ? ParamENO_CHD2010EdeltaPw : ParamENO_CHD2010EdeltaEwh;
    if (delta > 4095)
      delta = 4095;
    uint8_t unit = power ? (ParamENO_CHD2010EUNP ? 4 : 3) : (ParamENO_CHD2010EUNE ? 2 : 1); // kW/W bzw. kWh/Wh
    const uint8_t data[6] = {VLD_CMD_ID_05, (uint8_t)((report ? 0x80 : 0) | (power ? 0x20 : 0) | 0x1E),
                             (uint8_t)(((delta & 0x0F) << 4) | unit), (uint8_t)(delta >> 4), mat, mit};
    logInfoP("D2-01 measurement setup %s: unit %u, delta %u, min %u s, max %u s, auto %u", power ? "power" : "energy", unit, delta,
             mit, mat * 10, report);
    openknxEnoceanModule.sendVLDTelegram(_channelIndex, data, 6, "D2-01 measurement setup");
  }
}

// KNX -> D2-01-Aktor (Belegung siehe EnoceanModule.templ.xml):
// 1 Kanal:     KO_1 Schalten, KO_2 Messwert abfragen (0 = Energie, 1 = Leistung), KO_3 Status abfragen,
//              KO_4 Dimmwert (nur Dimmer).
// 2/4 Kanäle:  KO_1..n Schalten Kanal 1..n, KO_2n+1 Status abfragen.
// Die Antworten des Aktors (CMD 0x04 Status, CMD 0x07 Messwert) wertet handleD201() aus.
void EnoceanChannel::processKnxVld(int koIndex, GroupObject &ko) {
  if (vldProfile() == 0)
    return;
  const D201Caps &caps = d201Caps();
  bool statusQuery = false;

  if (caps.channels == 1) {
    switch (koIndex) {
    case ENO_KoGO_BASE__1:
      sendVldSetOutput(0x1E, (bool)ko.value(Dpt(1, 1)) ? 100 : 0);
      break;
    case ENO_KoGO_BASE__2: {
      if (caps.meas == D201_MEAS_NONE)
        break;
      // CMD 0x06 (Actuator Measurement Query): DB0 Bit 5 Energie/Leistung, Bit 4-0 I/O-Kanal (alle)
      bool power = ko.value(Dpt(1, 1));
      const uint8_t data[2] = {VLD_CMD_ID_06, (uint8_t)((power ? 0x20 : 0) | 0x1E)};
      logInfoP("D2-01 query %s", power ? "power" : "energy");
      openknxEnoceanModule.sendVLDTelegram(_channelIndex, data, 2, "D2-01 measurement query");
      break;
    }
    case ENO_KoGO_BASE__3:
      statusQuery = true;
      break;
    case ENO_KoGO_BASE__4:
      if (caps.dim)
        sendVldSetOutput(0x1E, (uint8_t)round((double)ko.value(Dpt(5, 1))));
      break;
    }
  } else if (koIndex < caps.channels) {
    sendVldSetOutput((uint8_t)koIndex, (bool)ko.value(Dpt(1, 1)) ? 100 : 0);
  } else if (koIndex == 2 * caps.channels) {
    statusQuery = true;
  }

  if (statusQuery) {
    // CMD 0x03 (Actuator Status Query): DB0 Bit 4-0 I/O-Kanal (alle), Antwort je Kanal mit CMD 0x04
    const uint8_t data[2] = {VLD_CMD_ID_03, 0x1E};
    logInfoP("D2-01 status query");
    openknxEnoceanModule.sendVLDTelegram(_channelIndex, data, 2, "D2-01 status query");
  }
}

// Telegramm eines D2-01-Aktors (d[1] = erstes Datenbyte):
// CMD 0x04 Actuator Status Response: DB2 Bit 3-0 CMD (Bit 7-6 Stromausfall-Flags), DB1 Bit 4-0 I/O-Kanal,
//   DB0 Bit 6-0 Ausgangswert (0 = AUS, 1..100 %). 1 Kanal: KO_5 Status, Dimmer zusätzlich KO_10 Dimmwert;
//   2/4 Kanäle: KO_n+1.. Status je Kanal.
// CMD 0x07 Actuator Measurement Response: DB4 Bit 7-5 Einheit, DB3..DB0 Messwert (MSB zuerst) -> KO_6..9.
void EnoceanChannel::handleD201(PACKET_SERIAL_TYPE_ *pPacket) {
  const uint8_t *d = pPacket->u8DataBuffer;
  const D201Caps &caps = d201Caps();

  switch (d[1] & 0x0F) {
  case VLD_CMD_ID_04: {
    uint8_t io = d[2] & 0x1F;
    uint8_t value = d[3] & 0x7F;
    if (value > 100)
      value = 100; // 0x7F: Ausgangswert nicht gültig
    logDebugP("D2-01 status: channel %u, value %u", io, value);
    if (caps.channels == 1) {
      KoENO_GO_BASE__5.value(value > 0, Dpt(1, 1));
      if (caps.dim)
        KoENO_GO_BASE__10.value(value, Dpt(5, 1));
    } else if (io < caps.channels) {
      knx.getGroupObject(ENO_KoCalcNumber(caps.channels + io)).value(value > 0, Dpt(1, 1));
    }
    break;
  }
  case VLD_CMD_ID_07: {
    if (caps.channels != 1 || caps.meas == D201_MEAS_NONE)
      break;
    uint8_t unit = d[2] >> 5;
    uint32_t value = ((uint32_t)d[3] << 24) | ((uint32_t)d[4] << 16) | ((uint32_t)d[5] << 8) | d[6];
    logDebugP("D2-01 measurement: unit %u, value %u", unit, value);
    switch (unit) {
    case 0x01: // Wh
      KoENO_GO_BASE__6.value(value, Dpt(13, 1));
      break;
    case 0x02: // kWh
      KoENO_GO_BASE__7.value(value, Dpt(13, 1));
      break;
    case 0x03: // W
      KoENO_GO_BASE__8.value(value, Dpt(14, 1));
      break;
    case 0x04: // kW
      KoENO_GO_BASE__9.value(value, Dpt(9, 1));
      break;
    default: // 0x00 Ws wird nicht ausgegeben
      break;
    }
    break;
  }
  default:
    logDebugP("D2-01: CMD %02X ignored", d[1] & 0x0F);
    break;
  }
}

// Aktor mit Zentralbefehl A5-38-08 dieses Kanals (ACT_*): per EEP-Profil (4BS A5-38) oder als Eltako-Produkt
// (Kategorie Aktoren). Beide Wege nutzen dieselben Funktionen, Unterschiede nur über actuatorIsEltako().
uint8_t EnoceanChannel::actuatorType() const {
  switch (ParamENO_CHProductsel) {
  case EEP_Profil:
    if (ParamENO_CHProfilSelection != u8RORG_4BS || ParamENO_CHProfilSelection4BS != A5_38)
      return ACT_NONE;
    switch (ParamENO_CHProfil4BS38) {
    case A5_38_08_SWITCH:
      return ACT_SWITCH;
    case A5_38_08_DIM:
      return ACT_DIM;
    default:
      return ACT_NONE;
    }
  case Hersteller_Eltako:
    if (ParamENO_CHProductselType != Eltako_Aktor)
      return ACT_NONE;
    switch (ParamENO_CHProductselTypeAkt) {
    case Eltako_Aktor_FSR:
      return ACT_SWITCH;
    case Eltako_Aktor_FUD:
    case Eltako_Aktor_FSUD:
      return ACT_DIM;
    case Eltako_Aktor_FSB:
      return ACT_SHUTTER;
    default:
      return ACT_NONE;
    }
  default:
    return ACT_NONE;
  }
}

bool EnoceanChannel::actuatorIsEltako() const { return ParamENO_CHProductsel == Hersteller_Eltako; }

// Zentralbefehl Schalten (CMD 0x01): DB2/DB1 Zeit 0 (kein Zeitablauf), DB0 Bit 3 Datentelegramm, Bit 0 EIN/AUS.
void EnoceanChannel::sendActuatorSwitch(bool on) {
  const uint8_t data[4] = {0x01, 0x00, 0x00, (uint8_t)(0x08 | (on ? 0x01 : 0x00))};
  logInfoP("A5-38-08 switch %u", on);
  openknxEnoceanModule.send4BSTelegram(_channelIndex, data, "A5-38-08 switch");
}

// Zentralbefehl Dimmen (CMD 0x02): DB2 Dimmwert 0..100 %, DB1 Rampe (EEP: Sekunden bis 100 %, Eltako: Geschwindigkeit,
// 0 = Drehschalter am Aktor), DB0 Bit 3 Datentelegramm, Bit 0 EIN/AUS. DB0 Bit 2 heißt beim EEP "Wert relativ
// (0..100 %)", bei Eltako "Dimmwert blockieren" und bleibt dort deshalb 0.
void EnoceanChannel::sendActuatorDim(uint8_t percent) {
  if (percent > 100)
    percent = 100;
  if (percent)
    _actLastDim = percent;
  const uint8_t data[4] = {0x02, percent, ParamENO_CHA53808Ramp,
                           (uint8_t)(0x08 | (actuatorIsEltako() ? 0x00 : 0x04) | (percent ? 0x01 : 0x00))};
  logInfoP("A5-38-08 dim %u %%", percent);
  openknxEnoceanModule.send4BSTelegram(_channelIndex, data, "A5-38-08 dim");
}

// Status auf KO_3 (Schalten) und beim Dimmaktor auf KO_4 (Dimmwert) ausgeben.
void EnoceanChannel::actuatorStatus(bool on, uint8_t percent) {
  if (on && percent)
    _actLastDim = percent;
  KoENO_GO_BASE__3.value(on, Dpt(1, 1));
  if (actuatorType() == ACT_DIM)
    KoENO_GO_BASE__4.value((uint8_t)(on ? percent : 0), Dpt(5, 1));
}

// KNX -> Aktor: KO_1 Schalten (Dimmaktor: Einschaltwert per ETS, 0 = letzter Wert), KO_2 Dimmwert (nur Dimmaktor).
// Ohne bekannte Aktor-ID kommt keine Bestätigung an, dann wird der Status direkt aus dem gesendeten Befehl gesetzt.
// Liefert false, wenn der Kanal kein A5-38-08-Aktor ist.
bool EnoceanChannel::processKnxActuator(int koIndex, GroupObject &ko) {
  uint8_t type = actuatorType();
  if (type == ACT_NONE)
    return false;
  if (type == ACT_SHUTTER) {
    processKnxShutter(koIndex, ko);
    return true;
  }

  bool on;
  uint8_t percent;
  if (koIndex == ENO_KoGO_BASE__1) {
    on = ko.value(Dpt(1, 1));
    if (type == ACT_SWITCH) {
      percent = on ? 100 : 0;
      sendActuatorSwitch(on);
    } else {
      percent = on ? (ParamENO_CHA53808OnValue ? ParamENO_CHA53808OnValue : _actLastDim) : 0;
      sendActuatorDim(percent);
    }
  } else if (koIndex == ENO_KoGO_BASE__2 && type == ACT_DIM) {
    percent = (uint8_t)round((double)ko.value(Dpt(5, 1)));
    on = percent > 0;
    sendActuatorDim(percent);
  } else
    return true;

  uint8_t id[4];
  if (!deviceId(id))
    actuatorStatus(on, percent);
  return true;
}

// Bestätigungstelegramme des Aktors: RPS (z.B. Eltako FSR: 0x70 = EIN, 0x50 = AUS) oder 4BS im Format des
// Zentralbefehls (DB3 Kommando 1 Schalten / 2 Dimmen, DB2 Dimmwert in %, DB0 Bit 0 EIN/AUS, Bit 3 = 1 Datentelegramm).
void EnoceanChannel::handleActuatorStatus(PACKET_SERIAL_TYPE_ *pPacket) {
  if (actuatorType() == ACT_SHUTTER) {
    handleShutterStatus(pPacket);
    return;
  }
  const uint8_t *d = pPacket->u8DataBuffer;
  if (d[0] == u8RORG_RPS && (d[1] == 0x70 || d[1] == 0x50)) {
    bool on = d[1] == 0x70;
    logDebugP("A5-38-08 status (RPS): %u", on);
    actuatorStatus(on, on ? _actLastDim : 0);
    return;
  }
  if (d[0] == u8RORG_4BS && (d[4] & 0x08) && (d[1] == 0x01 || d[1] == 0x02)) {
    bool on = d[4] & 0x01;
    uint8_t percent = d[1] == 0x02 ? (d[2] > 100 ? 100 : d[2]) : (on ? _actLastDim : 0);
    logDebugP("A5-38-08 status (4BS): %u, %u %%", on, percent);
    actuatorStatus(on, percent);
    return;
  }
  logDebugP("A5-38-08: telegram R-ORG %02X / %02X ignored", d[0], d[1]);
}

// Eltako Jalousie-/Rollladenaktor (FSB14/61/71, FJ62), Fahrbefehl laut "Inhalte der Eltako-Funktelegramme"
// (FUNC 3F, TYPE 7F): DB3/DB2 Laufzeit in 100 ms, DB1 Kommando (0 = Stopp, 1 = Auf, 2 = Ab), DB0 Bit 3 Datentelegramm,
// Bit 1 = Laufzeit in 100 ms (statt DB2 in Sekunden). Die am Aktor eingestellte Laufzeit wird dabei ignoriert.
void EnoceanChannel::sendShutterCommand(uint8_t command, uint16_t time100ms) {
  const uint8_t data[4] = {(uint8_t)(time100ms >> 8), (uint8_t)time100ms, command, 0x0A};
  logInfoP("Shutter %s, %u.%u s", command == 1 ? "up" : (command == 2 ? "down" : "stop"), time100ms / 10, time100ms % 10);
  openknxEnoceanModule.send4BSTelegram(_channelIndex, data, "Eltako shutter");
}

// Status Position (KO_4, nur wenn bekannt) und Status Fahrt (KO_5) ausgeben.
void EnoceanChannel::shutterStatus(uint8_t pos, bool moving) {
  _shutterPos = pos;
  if (pos != SHUTTER_POS_UNKNOWN)
    KoENO_GO_BASE__4.value(pos, Dpt(5, 1));
  KoENO_GO_BASE__5.value(moving, Dpt(1, 1));
}

// KNX -> Jalousieaktor: KO_1 Auf/Ab (DPT 1.008, 1 = Ab), KO_2 Stopp, KO_3 Position anfahren (DPT 5.001, 0 % = oben).
// Fahrten in eine Endlage bekommen 10 % Reserve auf die Fahrzeit, damit die Endlage sicher erreicht wird. Eine
// Zwischenposition geht nur bei bekannter Position. Ohne bekannte Aktor-ID kommt keine Bestätigung an, dann wird die
// Position direkt aus dem Befehl gesetzt.
void EnoceanChannel::processKnxShutter(int koIndex, GroupObject &ko) {
  uint16_t full = ParamENO_CHShutterTime * 10; // 100 ms
  uint8_t target;

  switch (koIndex) {
  case ENO_KoGO_BASE__1:
    target = (bool)ko.value(Dpt(1, 8)) ? 100 : 0;
    break;
  case ENO_KoGO_BASE__2:
    sendShutterCommand(0, 0);
    return;
  case ENO_KoGO_BASE__3:
    target = (uint8_t)round((double)ko.value(Dpt(5, 1)));
    if (target > 100)
      target = 100;
    break;
  default:
    return;
  }

  if (target == 0 || target == 100) {
    sendShutterCommand(target ? 2 : 1, full + full / 10);
  } else if (_shutterPos == SHUTTER_POS_UNKNOWN) {
    logInfoP("Shutter: position unknown, move to an end position first");
    return;
  } else if (target != _shutterPos) {
    uint8_t diff = target > _shutterPos ? target - _shutterPos : _shutterPos - target;
    uint16_t time = (uint32_t)diff * full / 100;
    sendShutterCommand(target > _shutterPos ? 2 : 1, time ? time : 1);
  } else
    return;

  uint8_t id[4];
  if (!deviceId(id))
    shutterStatus(target, false);
}

// Bestätigungen des Jalousieaktors: RPS 0x70 = Endlage oben, 0x50 = Endlage unten, 0x01 = Start auf, 0x02 = Start ab;
// 4BS beim Stopp vor der Endlage: DB3/DB2 gefahrene Zeit in 100 ms, DB1 1 = aufgefahren / 2 = abgefahren.
void EnoceanChannel::handleShutterStatus(PACKET_SERIAL_TYPE_ *pPacket) {
  const uint8_t *d = pPacket->u8DataBuffer;

  if (d[0] == u8RORG_RPS) {
    switch (d[1]) {
    case 0x70:
      logDebugP("Shutter: end position up");
      shutterStatus(0, false);
      return;
    case 0x50:
      logDebugP("Shutter: end position down");
      shutterStatus(100, false);
      return;
    case 0x01:
    case 0x02:
      logDebugP("Shutter: moving %s", d[1] == 0x01 ? "up" : "down");
      shutterStatus(_shutterPos, true);
      return;
    }
  } else if (d[0] == u8RORG_4BS && (d[4] & 0x08) && (d[3] == 0x01 || d[3] == 0x02)) {
    uint16_t time = ((uint16_t)d[1] << 8) | d[2];
    uint8_t pos = _shutterPos;
    uint16_t full = ParamENO_CHShutterTime * 10;
    if (pos != SHUTTER_POS_UNKNOWN && full) {
      int16_t moved = (int16_t)((uint32_t)time * 100 / full);
      int16_t p = d[3] == 0x02 ? pos + moved : pos - moved;
      pos = p < 0 ? 0 : (p > 100 ? 100 : p);
    }
    logDebugP("Shutter: stopped after %u.%u s %s, position %u", time / 10, time % 10, d[3] == 0x01 ? "up" : "down", pos);
    shutterStatus(pos, false);
    return;
  }
  logDebugP("Shutter: telegram R-ORG %02X / %02X ignored", d[0], d[1]);
}
