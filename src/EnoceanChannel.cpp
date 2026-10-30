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
// Lernmodus den Kanal einlernen kann (ETS-Button / Konsole "eno learn N"). Derzeit für den Taster in Richtung
// KNX -> EnOcean: Tastendruck oben (AO), danach Loslassen. Liefert false, wenn der Kanal nicht sendet.
bool EnoceanChannel::sendLearnTelegram() {
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

  // Prüft wie der CH konfiguriert ist. Über EEP-Profil oder über Hersteller
  switch (ParamENO_CHProductsel) {
  case CH_inaktive: // Kein Profil oder Hersteller gewählt
    return false;
    break;

  case EEP_Profil: // EEP Profile

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
  if (ParamENO_CHProductsel == CH_inaktive)
    return false;

  uint8_t configuredId[4];
  configuredId[0] = (ParamENO_CHId0 << 4) | ParamENO_CHId1;
  configuredId[1] = (ParamENO_CHId2 << 4) | ParamENO_CHId3;
  configuredId[2] = (ParamENO_CHId4 << 4) | ParamENO_CHId5;
  configuredId[3] = (ParamENO_CHId6 << 4) | ParamENO_CHId7;

  bool paramIdSet = configuredId[0] | configuredId[1] | configuredId[2] | configuredId[3];
  if (!paramIdSet) {
    if (ParamENO_CHSetupMode != ENO_SETUP_TEACHIN || !openknxEnoceanModule.learnedId(_channelIndex, configuredId))
      return false;
  }

  return memcmp(configuredId, id, 4) == 0;
}

// Bidirektionaler Heizkörper-Stellantrieb dieses Kanals (HVAC_*): EEP A5-20-01/04/06 oder Eltako FKS-SV (A5-20-01).
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
    if (ParamENO_CHProductselType == Eltako_Heizung && ParamENO_CHProductselTypeHei == Eltako_Heizung_FKS_SV)
      return HVAC_A52001;
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
