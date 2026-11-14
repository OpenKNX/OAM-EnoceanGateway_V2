#pragma once

#include "EnoceanModule.h"
#include "EnoceanProfils.h"
#include "EnoceanStructs.h"
#include "knxprod.h"
#include "WindowContact.h"
#include "AlarmOutput.h"

/*******************************************************************************************
 *  Rocker / Taster (Kanal-Profil "Rocker", EnOcean -> KNX), Telegramme F6-02-xx
 *
 *  DB0: Bit 7-5 R1 (0 = AI, 1 = AO, 2 = BI, 3 = BO), Bit 4 EB (1 = gedrückt, 0 = losgelassen),
 *       Bit 3-1 R2 (zweite Taste), Bit 0 SA (zweite Taste gültig).
 *  "O" = Wippe oben, "I" = Wippe unten.
 *
 *  KO-Belegung (ETS / KO-Tabelle), alle KOs einer Wippe stehen zusammen:
 *    Bedienung Wippe:  A KO_0 Schalten / AUF-AB / Szene, KO_1 Dimmen / Lamelle-Stopp
 *                      B KO_4 / KO_5, AB KO_8 / KO_9
 *    Einzeltasten:     A oben KO_0 (kurz) / KO_1 (lang, Dimmen, Stopp), A unten KO_2 / KO_3
 *                      B oben KO_4 / KO_5, B unten KO_6 / KO_7, AB oben KO_8 / KO_9, AB unten KO_10 / KO_11
 *    (KO_n = GO_BASE__(n+1); KO_0 ist nur bei Taster-Emulation KNX -> EnOcean der Eingang)
 *******************************************************************************************/

#define ROCKER_WIPPE_A 0
#define ROCKER_WIPPE_B 1
#define ROCKER_WIPPE_AB 2
#define ROCKER_NONE 0xFF

// Funktionen laut ETS-Parameter RockerFunktionA/B/C
#define ROCKER_FUNC_SWITCH_ON_OFF 1
#define ROCKER_FUNC_SWITCH_OFF_ON 2
#define ROCKER_FUNC_DIM_UP_DOWN 3
#define ROCKER_FUNC_DIM_DOWN_UP 4
#define ROCKER_FUNC_BLIND_UP_DOWN 5
#define ROCKER_FUNC_BLIND_DOWN_UP 6
#define ROCKER_FUNC_SCENE 7

// Tastenlayout: Wippen1..Wippen3 wie ETS-Parameter RockerProfil (1 Wippe, 2 Wippen, 1 + 2 Wippen) plus Einzeltaste
#define ROCKER_LAYOUT_1KEY 0

// Eltako-Taster (Hersteller Eltako, Kategorie Taster): der Produktwert legt das Tastenlayout fest, laut "Inhalte der
// Eltako-Funktelegramme":
//    1..19: eine Taste (F6-01-01): Drücken 0x10, Loslassen 0x00
//   20..49: eine Wippe bzw. 2 Tasten (F6-02-01): oben 0x70, unten 0x50
//   50.. : Doppelwippe bzw. 4 Tasten (F6-02-01): rechts 0x70 / 0x50 (Wippe B), links 0x30 / 0x10 (Wippe A),
//          beide Wippen gleichzeitig = Wippe AB.
// 6- und 8-fach-Sender senden mit zwei IDs, je ID ein Kanal: ihre Tastengruppen sind eigene Produkte (40..49 die
// 2er-Gruppe der 6-fach-Sender, ab 70 je 4 Tasten). Sie haben keine Wippen, nur Einzeltasten, und kein gleichzeitiges
// Drücken (Wippe AB).
#define ELTAKO_TASTER_1KEY_MAX 19
#define ELTAKO_TASTER_1ROCKER_MAX 49
#define ELTAKO_TASTER_2KEYS_MIN 40
#define ELTAKO_TASTER_4KEYS_MIN 70

// Funktionen einer Einzeltaste (ETS-Parameter RockerKey..Funktion)
#define ROCKER_KEY_TOGGLE 1
#define ROCKER_KEY_PUSH 2
#define ROCKER_KEY_SCENE 3
#define ROCKER_KEY_DIM 4   // kurz Umschalten, lang Dimmen (Richtung wechselt)
#define ROCKER_KEY_BLIND 5 // lang Fahren (Richtung wechselt), kurz Stopp/Lamelle

// Langer Tastendruck einer Einzeltaste bei Umschalten / Gedrückt / Szene (ETS-Parameter RockerKey..Long), auf dem
// zweiten KO
#define ROCKER_KEY_LONG_NONE 0
#define ROCKER_KEY_LONG_TOGGLE 1
#define ROCKER_KEY_LONG_ON 2
#define ROCKER_KEY_LONG_OFF 3
#define ROCKER_KEY_LONG_SCENE 4

// Einstellungen einer Einzeltaste
struct RockerKeyConfig {
  uint8_t function;
  bool sendRelease;     // beim Loslassen senden (nur Gedrückt)
  bool invert;          // Gedrückt invertiert: Drücken = 0, Loslassen = 1
  uint8_t longFunction; // langer Tastendruck bei Umschalten / Gedrückt / Szene
  uint8_t longScene;    // Szene (1..64) beim langen Tastendruck
};

// Liefert das Tastenlayout des Kanals: EEP-Profil per Parameter "Schaltertyp" (Wippen4 = 1 Taste, F6-01-01),
// Eltako-Taster aus dem Produkt.
inline uint8_t rockerLayout(uint8_t _channelIndex) {
  if (ParamENO_CHProductsel != Hersteller_Eltako)
    return ParamENO_CHRockerProfil == Wippen4 ? ROCKER_LAYOUT_1KEY : ParamENO_CHRockerProfil;

  uint8_t product = ParamENO_CHProductselTypeTas;
  if (product <= ELTAKO_TASTER_1KEY_MAX)
    return ROCKER_LAYOUT_1KEY;
  if (product <= ELTAKO_TASTER_1ROCKER_MAX)
    return Wippen1;
  if (product >= ELTAKO_TASTER_4KEYS_MIN)
    return Wippen2;
  return Wippen3;
}

// true bei Sendern ohne Wippen (EEP-Schaltertyp 1 Taste, Eltako-Einzeltaste, 6- und 8-fach-Sender): immer
// Einzeltasten.
inline bool rockerKeysOnly(uint8_t _channelIndex) {
  if (ParamENO_CHProductsel != Hersteller_Eltako)
    return ParamENO_CHRockerProfil == Wippen4;
  uint8_t product = ParamENO_CHProductselTypeTas;
  return product <= ELTAKO_TASTER_1KEY_MAX || (product >= ELTAKO_TASTER_2KEYS_MIN && product <= ELTAKO_TASTER_1ROCKER_MAX) ||
         product >= ELTAKO_TASTER_4KEYS_MIN;
}

// Startet einen Dimmvorgang (DPT 3.007, Stepcode 1 = 100 %) in die angegebene Richtung (true = heller).
inline void SendDPT3_007(bool dir, GroupObject &ko) {
  logDebug("RPS", "DIM: %u", dir);
  ko.valueNoSend((uint8_t)1, Dpt(3, 7, 1));
  ko.value(dir, Dpt(3, 7, 0));
}

// Beendet einen laufenden Dimmvorgang (DPT 3.007 Stop).
inline void stopDim(GroupObject &ko) {
  logDebug("RPS", "Stop DIM");
  ko.valueNoSend((uint8_t)0, Dpt(3, 7, 1));
  ko.value(false, Dpt(3, 7, 0));
}

// Liefert die per ETS parametrierte Funktion der logischen Wippe.
inline uint8_t rockerFunction(uint8_t wippe, uint8_t _channelIndex) {
  switch (wippe) {
  case ROCKER_WIPPE_A:
    return ParamENO_CHRockerFunktionA;
  case ROCKER_WIPPE_B:
    return ParamENO_CHRockerFunktionB;
  default:
    return ParamENO_CHRockerFunktionC;
  }
}

// Liefert die per ETS parametrierte Szenennummer (1..64) für Wippe oben (Szene a) bzw. unten (Szene b).
inline uint8_t rockerScene(uint8_t wippe, bool upper, uint8_t _channelIndex) {
  switch (wippe) {
  case ROCKER_WIPPE_A:
    return upper ? ParamENO_CHRockerASzeneA : ParamENO_CHRockerASzeneB;
  case ROCKER_WIPPE_B:
    return upper ? ParamENO_CHRockerBSzeneA : ParamENO_CHRockerBSzeneB;
  default:
    return upper ? ParamENO_CHRockerCSzeneA : ParamENO_CHRockerCSzeneB;
  }
}

// Erstes KO der logischen Wippe (Schalten / AUF-AB / Szene): A KO_0, B KO_4, AB KO_8.
inline GroupObject &rockerKoMain(uint8_t wippe, uint8_t _channelIndex) {
  switch (wippe) {
  case ROCKER_WIPPE_A:
    return KoENO_GO_BASE__1;
  case ROCKER_WIPPE_B:
    return KoENO_GO_BASE__5;
  default:
    return KoENO_GO_BASE__9;
  }
}

// Zweites KO der logischen Wippe (Dimmen / Lamelle-Stopp): A KO_1, B KO_5, AB KO_9.
inline GroupObject &rockerKoSecond(uint8_t wippe, uint8_t _channelIndex) {
  switch (wippe) {
  case ROCKER_WIPPE_A:
    return KoENO_GO_BASE__2;
  case ROCKER_WIPPE_B:
    return KoENO_GO_BASE__6;
  default:
    return KoENO_GO_BASE__10;
  }
}

// true, wenn die Wippen-Seite die "positive" Aktion auslöst (EIN / heller / AUF). Bei den Funktionen
// "AUS/EIN", "Dunkler/Heller" und "AB/AUF" ist die Zuordnung oben/unten vertauscht.
inline bool rockerPositive(uint8_t function, bool upper) {
  bool inverted = function == ROCKER_FUNC_SWITCH_OFF_ON || function == ROCKER_FUNC_DIM_DOWN_UP || function == ROCKER_FUNC_BLIND_DOWN_UP;
  return upper != inverted;
}

// true, wenn die Funktion zwischen kurzem und langem Tastendruck unterscheidet (Dimmen, Jalousie). Schalten und
// Szene werden sofort beim Drücken ausgelöst.
inline bool rockerNeedsLongPress(uint8_t function) {
  return function >= ROCKER_FUNC_DIM_UP_DOWN && function <= ROCKER_FUNC_BLIND_DOWN_UP;
}

// Taste gedrückt: Schalten und Szene sofort senden.
inline void rockerPress(uint8_t wippe, bool upper, uint8_t _channelIndex) {
  uint8_t function = rockerFunction(wippe, _channelIndex);
  bool positive = rockerPositive(function, upper);

  switch (function) {
  case ROCKER_FUNC_SWITCH_ON_OFF:
  case ROCKER_FUNC_SWITCH_OFF_ON:
    logDebug("RPS", "Wippe %u: Schalten %u", wippe, positive);
    rockerKoMain(wippe, _channelIndex).value(positive, Dpt(1, 1));
    break;
  case ROCKER_FUNC_SCENE: {
    uint8_t scene = rockerScene(wippe, upper, _channelIndex);
    logDebug("RPS", "Wippe %u: Szene %u", wippe, scene);
    rockerKoMain(wippe, _channelIndex).value((uint8_t)(scene - 1), Dpt(17, 1));
    break;
  }
  }
}

// Kurzer Tastendruck (losgelassen vor Ablauf der Zeit für langen Tastendruck): Dimmen -> EIN/AUS,
// Jalousie -> Lamelle/Stopp.
inline void rockerShort(uint8_t wippe, bool upper, uint8_t _channelIndex) {
  uint8_t function = rockerFunction(wippe, _channelIndex);
  bool positive = rockerPositive(function, upper);

  switch (function) {
  case ROCKER_FUNC_DIM_UP_DOWN:
  case ROCKER_FUNC_DIM_DOWN_UP:
    logDebug("RPS", "Wippe %u: kurz Schalten %u", wippe, positive);
    rockerKoMain(wippe, _channelIndex).value(positive, Dpt(1, 1));
    break;
  case ROCKER_FUNC_BLIND_UP_DOWN:
  case ROCKER_FUNC_BLIND_DOWN_UP:
    // DPT 1.009 Lamelle/Stopp: 0 = AUF, 1 = AB
    logDebug("RPS", "Wippe %u: Lamelle/Stopp %s", wippe, positive ? "AUF" : "AB");
    rockerKoSecond(wippe, _channelIndex).value(!positive, Dpt(1, 9));
    break;
  }
}

// Langer Tastendruck erkannt (Taste noch gedrückt): Dimmen starten bzw. Jalousie fahren.
inline void rockerLongStart(uint8_t wippe, bool upper, uint8_t _channelIndex) {
  uint8_t function = rockerFunction(wippe, _channelIndex);
  bool positive = rockerPositive(function, upper);

  switch (function) {
  case ROCKER_FUNC_DIM_UP_DOWN:
  case ROCKER_FUNC_DIM_DOWN_UP:
    logDebug("RPS", "Wippe %u: Dimmen %s", wippe, positive ? "heller" : "dunkler");
    SendDPT3_007(positive, rockerKoSecond(wippe, _channelIndex));
    break;
  case ROCKER_FUNC_BLIND_UP_DOWN:
  case ROCKER_FUNC_BLIND_DOWN_UP:
    // DPT 1.008 AUF/AB: 0 = AUF, 1 = AB
    logDebug("RPS", "Wippe %u: Jalousie %s", wippe, positive ? "AUF" : "AB");
    rockerKoMain(wippe, _channelIndex).value(!positive, Dpt(1, 8));
    break;
  }
}

// Losgelassen nach langem Tastendruck: Dimmen stoppen (Jalousie fährt weiter, Stopp per kurzem Tastendruck).
inline void rockerLongEnd(uint8_t wippe, uint8_t _channelIndex) {
  uint8_t function = rockerFunction(wippe, _channelIndex);
  if (function == ROCKER_FUNC_DIM_UP_DOWN || function == ROCKER_FUNC_DIM_DOWN_UP)
    stopDim(rockerKoSecond(wippe, _channelIndex));
}

// Ermittelt aus DB0 eines "gedrückt"-Telegramms die logische Wippe (abhängig vom Schaltertyp) und ob oben gedrückt
// wurde. Liefert ROCKER_NONE, wenn die Tastenkombination für den Schaltertyp nicht belegt ist.
inline uint8_t rockerDecode(uint8_t db0, bool *upper, uint8_t _channelIndex) {
  uint8_t r1 = (db0 >> 5) & 0x07;
  uint8_t r2 = (db0 >> 1) & 0x07;
  bool secondAction = db0 & 0x01;

  *upper = r1 & 0x01; // AO / BO
  bool rockerB = r1 & 0x02;
  // zweite Taste auf der jeweils anderen Wippe -> beide Wippen gleichzeitig (Wippe AB)
  bool both = secondAction && ((r2 & 0x02) != (r1 & 0x02));

  switch (rockerLayout(_channelIndex)) {
  case Wippen1: // 1 Wippe: jede Taste / Kombination wirkt auf Wippe A
    return ROCKER_WIPPE_A;
  case Wippen2: // 2 Wippen
    if (both)
      return ROCKER_NONE;
    return rockerB ? ROCKER_WIPPE_B : ROCKER_WIPPE_A;
  default: // 1 + 2 Wippen
    if (both)
      return ROCKER_WIPPE_AB;
    return rockerB ? ROCKER_WIPPE_B : ROCKER_WIPPE_A;
  }
}

// Führt die Funktion einer Einzeltaste aus: Umschalten bei jedem Drücken, Gedrückt (Drücken = 1, Loslassen = 0 nur
// wenn gewünscht, invertiert umgekehrt) oder Szene (scene = 1..64) beim Drücken.
inline void rockerKeyAction(GroupObject &ko, const RockerKeyConfig &cfg, uint8_t scene, bool pressed) {
  switch (cfg.function) {
  case ROCKER_KEY_TOGGLE:
    if (pressed) {
      bool on = !(bool)ko.value(Dpt(1, 1));
      logDebug("RPS", "Taste: Umschalten %u", on);
      ko.value(on, Dpt(1, 1));
    }
    break;
  case ROCKER_KEY_PUSH:
    logDebug("RPS", "Taste: %s", pressed ? "gedrückt" : "losgelassen");
    if (pressed || cfg.sendRelease)
      ko.value(pressed != cfg.invert, Dpt(1, 1));
    break;
  case ROCKER_KEY_SCENE:
    if (pressed) {
      logDebug("RPS", "Taste: Szene %u", scene);
      ko.value((uint8_t)(scene - 1), Dpt(17, 1));
    }
    break;
  }
}

// true, wenn die Tasten oben und unten der logischen Wippe getrennt bedient werden (ETS: Bedienung = Einzeltasten,
// bei Eltako-Sendern ohne Wippen immer).
inline bool rockerKeyMode(uint8_t wippe, uint8_t _channelIndex) {
  if (rockerKeysOnly(_channelIndex))
    return true;
  switch (wippe) {
  case ROCKER_WIPPE_A:
    return ParamENO_CHRockerModeA;
  case ROCKER_WIPPE_B:
    return ParamENO_CHRockerModeB;
  default:
    return ParamENO_CHRockerModeC;
  }
}

// Einstellungen der Taste oben bzw. unten einer Wippe mit getrennten Tasten.
inline RockerKeyConfig rockerKeyConfig(uint8_t wippe, bool upper, uint8_t _channelIndex) {
  switch (wippe) {
  case ROCKER_WIPPE_A:
    if (upper)
      return {ParamENO_CHRockerKeyAOFunktion, ParamENO_CHRockerKeyAORelease, ParamENO_CHRockerKeyAOInvert, ParamENO_CHRockerKeyAOLong,
              ParamENO_CHRockerKeyAOLongScene};
    return {ParamENO_CHRockerKeyAUFunktion, ParamENO_CHRockerKeyAURelease, ParamENO_CHRockerKeyAUInvert, ParamENO_CHRockerKeyAULong,
            ParamENO_CHRockerKeyAULongScene};
  case ROCKER_WIPPE_B:
    if (upper)
      return {ParamENO_CHRockerKeyBOFunktion, ParamENO_CHRockerKeyBORelease, ParamENO_CHRockerKeyBOInvert, ParamENO_CHRockerKeyBOLong,
              ParamENO_CHRockerKeyBOLongScene};
    return {ParamENO_CHRockerKeyBUFunktion, ParamENO_CHRockerKeyBURelease, ParamENO_CHRockerKeyBUInvert, ParamENO_CHRockerKeyBULong,
            ParamENO_CHRockerKeyBULongScene};
  default:
    if (upper)
      return {ParamENO_CHRockerKeyCOFunktion, ParamENO_CHRockerKeyCORelease, ParamENO_CHRockerKeyCOInvert, ParamENO_CHRockerKeyCOLong,
              ParamENO_CHRockerKeyCOLongScene};
    return {ParamENO_CHRockerKeyCUFunktion, ParamENO_CHRockerKeyCURelease, ParamENO_CHRockerKeyCUInvert, ParamENO_CHRockerKeyCULong,
            ParamENO_CHRockerKeyCULongScene};
  }
}

// Erstes KO einer Einzeltaste: A oben KO_0, A unten KO_2, B oben KO_4, B unten KO_6, AB oben KO_8, AB unten KO_10.
inline GroupObject &rockerKeyKo(uint8_t wippe, bool upper, uint8_t _channelIndex) {
  switch (wippe) {
  case ROCKER_WIPPE_A:
    return upper ? KoENO_GO_BASE__1 : KoENO_GO_BASE__3;
  case ROCKER_WIPPE_B:
    return upper ? KoENO_GO_BASE__5 : KoENO_GO_BASE__7;
  default:
    return upper ? KoENO_GO_BASE__9 : KoENO_GO_BASE__11;
  }
}

// Zweites KO einer Einzeltaste (langer Tastendruck, Dimmen, Stopp/Lamelle), direkt hinter dem ersten.
inline GroupObject &rockerKeyKo2(uint8_t wippe, bool upper, uint8_t _channelIndex) {
  switch (wippe) {
  case ROCKER_WIPPE_A:
    return upper ? KoENO_GO_BASE__2 : KoENO_GO_BASE__4;
  case ROCKER_WIPPE_B:
    return upper ? KoENO_GO_BASE__6 : KoENO_GO_BASE__8;
  default:
    return upper ? KoENO_GO_BASE__10 : KoENO_GO_BASE__12;
  }
}

// Bit der Einzeltaste im Richtungsspeicher (Dimmen: zuletzt heller, Jalousie: zuletzt AB).
inline uint8_t rockerKeyBit(uint8_t wippe, bool upper) { return 1 << (wippe * 2 + (upper ? 0 : 1)); }

// true, wenn die Taste zwischen kurzem und langem Tastendruck unterscheidet. Dann wird der kurze Druck erst beim
// Loslassen ausgeführt, sonst sofort beim Drücken.
inline bool rockerKeyNeedsLongPress(const RockerKeyConfig &cfg) {
  if (cfg.function == ROCKER_KEY_DIM || cfg.function == ROCKER_KEY_BLIND)
    return true;
  return cfg.longFunction != ROCKER_KEY_LONG_NONE;
}

// Einzeltaste gedrückt: ohne Unterscheidung kurz / lang sofort ausführen.
inline void rockerKeyPress(uint8_t wippe, bool upper, uint8_t _channelIndex) {
  RockerKeyConfig cfg = rockerKeyConfig(wippe, upper, _channelIndex);
  logDebug("RPS", "Wippe %u: Taste %s gedrückt", wippe, upper ? "oben" : "unten");
  if (!rockerKeyNeedsLongPress(cfg))
    rockerKeyAction(rockerKeyKo(wippe, upper, _channelIndex), cfg, rockerScene(wippe, upper, _channelIndex), true);
}

// Langer Tastendruck einer Einzeltaste erkannt (Taste noch gedrückt): Dimmen bzw. Fahren mit wechselnder Richtung
// (Dimmen bei ausgeschaltetem Licht immer heller) oder die Funktion für langen Tastendruck auf dem zweiten KO.
inline void rockerKeyLongStart(uint8_t wippe, bool upper, uint8_t &directions, uint8_t _channelIndex) {
  RockerKeyConfig cfg = rockerKeyConfig(wippe, upper, _channelIndex);
  GroupObject &ko2 = rockerKeyKo2(wippe, upper, _channelIndex);
  uint8_t bit = rockerKeyBit(wippe, upper);

  switch (cfg.function) {
  case ROCKER_KEY_DIM: {
    bool up = !(bool)rockerKeyKo(wippe, upper, _channelIndex).value(Dpt(1, 1)) || !(directions & bit);
    directions = up ? (directions | bit) : (directions & ~bit);
    logDebug("RPS", "Wippe %u: Taste lang, Dimmen %s", wippe, up ? "heller" : "dunkler");
    SendDPT3_007(up, ko2);
    break;
  }
  case ROCKER_KEY_BLIND: {
    bool down = !(directions & bit);
    directions = down ? (directions | bit) : (directions & ~bit);
    // DPT 1.008 AUF/AB: 0 = AUF, 1 = AB
    logDebug("RPS", "Wippe %u: Taste lang, Jalousie %s", wippe, down ? "AB" : "AUF");
    rockerKeyKo(wippe, upper, _channelIndex).value(down, Dpt(1, 8));
    break;
  }
  default:
    switch (cfg.longFunction) {
    case ROCKER_KEY_LONG_TOGGLE:
      ko2.value(!(bool)ko2.value(Dpt(1, 1)), Dpt(1, 1));
      break;
    case ROCKER_KEY_LONG_ON:
      ko2.value(true, Dpt(1, 1));
      break;
    case ROCKER_KEY_LONG_OFF:
      ko2.value(false, Dpt(1, 1));
      break;
    case ROCKER_KEY_LONG_SCENE:
      ko2.value((uint8_t)(cfg.longScene - 1), Dpt(17, 1));
      break;
    }
    logDebug("RPS", "Wippe %u: Taste lang, Funktion %u", wippe, cfg.longFunction);
    break;
  }
}

// Einzeltaste losgelassen: nach langem Druck Dimmen stoppen (Jalousie fährt weiter), nach kurzem Druck die kurze
// Funktion ausführen (Dimmen: Umschalten, Jalousie: Stopp/Lamelle in letzter Fahrtrichtung).
inline void rockerKeyRelease(uint8_t wippe, bool upper, bool wasLong, uint8_t directions, uint8_t _channelIndex) {
  RockerKeyConfig cfg = rockerKeyConfig(wippe, upper, _channelIndex);
  GroupObject &ko = rockerKeyKo(wippe, upper, _channelIndex);
  logDebug("RPS", "Wippe %u: Taste %s losgelassen", wippe, upper ? "oben" : "unten");

  if (wasLong) {
    if (cfg.function == ROCKER_KEY_DIM)
      stopDim(rockerKeyKo2(wippe, upper, _channelIndex));
    return;
  }

  if (!rockerKeyNeedsLongPress(cfg)) {
    rockerKeyAction(ko, cfg, rockerScene(wippe, upper, _channelIndex), false);
    return;
  }

  switch (cfg.function) {
  case ROCKER_KEY_DIM:
    ko.value(!(bool)ko.value(Dpt(1, 1)), Dpt(1, 1));
    break;
  case ROCKER_KEY_BLIND:
    // DPT 1.007 Schritt: 0 = AUF, 1 = AB
    rockerKeyKo2(wippe, upper, _channelIndex).value((bool)(directions & rockerKeyBit(wippe, upper)), Dpt(1, 7));
    break;
  default: // Umschalten / Gedrückt / Szene mit langem Tastendruck: kurzer Druck erst jetzt
    rockerKeyAction(ko, cfg, rockerScene(wippe, upper, _channelIndex), true);
    // Gedrückt: Drücken und Loslassen fallen zusammen, ergibt einen kurzen Impuls (wenn Loslassen gesendet wird)
    if (cfg.function == ROCKER_KEY_PUSH)
      rockerKeyAction(ko, cfg, 0, false);
    break;
  }
}

// Verarbeitet ein F6-02-03-Telegramm (zwei Wippen an einem Kontakt) und schaltet die beiden zugehörigen Gruppenobjekte
// je nach Tastencode.
inline void handle_F6_02_03(uint8_t value, uint8_t _channelIndex) {
  logDebug("RPS", "Profil: F6-02-03");
  switch (value) {
  case 0x30: // A0 Set
    KoENO_GO_BASE__1.value(true, Dpt(1, 1));
    logDebug("RPS", "A0 (0x30)");
    break;
  case 0x10: // A0 release
    KoENO_GO_BASE__1.value(false, Dpt(1, 1));
    logDebug("RPS", "A1 (0x10)");
    break;
  case 0x70: // B0 Set
    KoENO_GO_BASE__2.value(true, Dpt(1, 1));
    logDebug("RPS", "B0 (0x70)");
    break;
  case 0x50: // B0 relesae
    KoENO_GO_BASE__2.value(false, Dpt(1, 1));
    logDebug("RPS", "B1 (0x50)");
    break;
  default:
    break;
  }
}

// Verarbeitet die F6-05-xx-Telegramme (Rauch-/Wind-/Wassermelder bzw. Energiestatus) und schreibt den passenden
// Alarm-/Statuswert.
// KO-Belegung laut ETS: KO_3 (GO_BASE__4) = Alarm, KO_4 (GO_BASE__5) = Energy Low.
// Wasser (F6-05-01): Data 0x11, ob Wasser da ist steht im Statusbyte (NU-Bit, z.B. Eltako FWS81: 0x30 = Wasser,
// 0x20 = kein Wasser).
inline void handle_F6_05_0x(PACKET_SERIAL_TYPE_ *f_Pkt_st, uint8_t _channelIndex) {
  uint8_t value = f_Pkt_st->u8DataBuffer[1];
  uint8_t status = f_Pkt_st->u8DataBuffer[6];
  logDebug("RPS", "F6-05: Data %02X, Status %02X", value, status);

  switch (value) {
  case 0x00: // SMOKE/WIND Alarm OFF
    sendAlarm(KoENO_GO_BASE__4, false, _channelIndex);
    break;
  case 0x10: // SMOKE/WIND Alarm ON
    sendAlarm(KoENO_GO_BASE__4, true, _channelIndex);
    break;
  case 0x11: // Water detected / kein Wasser (NU-Bit)
    sendAlarm(KoENO_GO_BASE__4, status & 0x10, _channelIndex);
    break;
  case 0x30: // Energy LOW
    KoENO_GO_BASE__5.value(true, DPT_Bool);
    break;
  default:
    break;
  }
}

inline void handle_F6_10_00(PACKET_SERIAL_TYPE_ *f_Pkt_st, uint8_t _channelIndex);

// Zentrale Dispatch-Funktion für empfangene RPS-Telegramme dieses Kanals: wertet das konfigurierte EEP-Profil aus und
// schaltet die zugehörigen Gruppenobjekte bzw. ruft die passende Behandlungsroutine auf.
inline void handle_RPS(PACKET_SERIAL_TYPE_ *f_Pkt_st, uint8_t _channelIndex) {
  RPS_F6_10_01_TYPE *lRpsTlg2_p;

#ifndef EnOceanTEST
  // switch (knx.paramWord(firstParameter + ENO_CHProfilSelectionRPS))
  switch (ParamENO_CHProfilSelectionRPS)
#else
  switch (profil)
#endif
  {
  case F6_02_01:
    logDebug("RPS", "Profil: F6-02-01");
    break;
  case F6_02_02:
    logDebug("RPS", "Profil: F6-02-02");
  case F6_02_03:
    handle_F6_02_03(f_Pkt_st->u8DataBuffer[1], _channelIndex);
    break;
  case F6_03_01:
    logDebug("RPS", "Profil: F6-03-01");
    break;
  case F6_03_02:
    logDebug("RPS", "Profil: F6-03-02");
    break;
  //**************************************************************
  // ----------------- Profil: F6-05-00   ------------------------
  //**************************************************************
  case F6_04_01: // Hotelkartenschalter: 0x70 = Karte gesteckt, 0x00 = Karte gezogen
  case F6_04_02: // ERP2: DB0 Bit 2 = Karte gesteckt
  {
    uint8_t value = f_Pkt_st->u8DataBuffer[1];
    bool inserted = ParamENO_CHProfilSelectionRPS == F6_04_01 ? value == 0x70 : (value & 0x04);
    logDebug("RPS", "Key card: %s (%02X)", inserted ? "gesteckt" : "gezogen", value);
    KoENO_GO_BASE__1.value(inserted, Dpt(1, 1));
    break;
  }
  case F6_05_00:
    logDebug("RPS", "Profil: F6-05-00");
    handle_F6_05_0x(f_Pkt_st, _channelIndex);
    break;
  //**************************************************************
  // ----------------- Profil: F6-05-01 --------------------------
  //**************************************************************
  case F6_05_01:
    logDebug("RPS", "Profil: F6-05-01");
    handle_F6_05_0x(f_Pkt_st, _channelIndex);
    break;
    //**************************************************************
  // ----------------- Profil: F6-05-02   ------------------------
  //**************************************************************
  case F6_05_02:
    logDebug("RPS", "Profil: F6-05-02");
    handle_F6_05_0x(f_Pkt_st, _channelIndex);
    break;
  //**************************************************************
  // ----------------- Profil: F6-10-00 --------------------------
  //**************************************************************
  case F6_10_00:
    handle_F6_10_00(f_Pkt_st, _channelIndex);
    break;
  //**************************************************************
  // ----------------- Profil: F6-10-01 --------------------------
  //**************************************************************
  case F6_10_01:
    logDebug("RPS", "Profil: F6-10-01");

    lRpsTlg2_p = (RPS_F6_10_01_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);

    // STATE = DB_BIT 5-4, gleiche Kodierung wie die unteren 2 Bit von F6-10-00
    switch (lRpsTlg2_p->u8RpsTelData.STATE) {
    case 0b11: // geschlossen (Hebel unten)
      setWindowHandleKOs(false, false, _channelIndex);
      logDebug("RPS", "geschlossen");
      break;
    case 0b01: // gekippt (Hebel oben)
      setWindowHandleKOs(false, true, _channelIndex);
      logDebug("RPS", "gekippt");
      break;
    case 0b00: // offen (Hebel waagerecht)
    case 0b10:
      setWindowHandleKOs(true, false, _channelIndex);
      logDebug("RPS", "offen");
      break;
    }
    break;
  default:
    logDebug("RPS", "Profil: ERROR");
    break;
  }
}

// Verarbeitet ein F6-10-00-Telegramm (Fenstergriff, z.B. Eltako FFG7B, bzw. Fenster-/Tastkontakt FTKE/FFTE).
// STATE = DB_BIT 7-4: 0b1111 = geschlossen (Griff unten), 0b1101 = gekippt (Griff oben),
// 0b1100 / 0b1110 = offen (Griff waagerecht). Kontakte senden nur 0xF0 (geschlossen) und 0xE0 (offen).
// KO-Belegung wie A5-14-09: KO2 = offen, KO3 = gekippt, KO4 = offen/geschlossen (Wert für geschlossen per ETS).
inline void handle_F6_10_00(PACKET_SERIAL_TYPE_ *f_Pkt_st, uint8_t _channelIndex) {
  RPS_F6_10_00_TYPE *lRpsTlg_p;

  logDebug("RPS", "Profil: F6-10-00");

  lRpsTlg_p = (RPS_F6_10_00_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);

  logDebug("RPS", "STATE: %u, NA: %u", lRpsTlg_p->u8RpsTelData.STATE, lRpsTlg_p->u8RpsTelData.NA);

  switch (lRpsTlg_p->u8RpsTelData.STATE) {
  case 0b1111: // geschlossen (Hebel unten)
    setWindowHandleKOs(false, false, _channelIndex);
    logDebug("RPS", "geschlossen");
    break;
  case 0b1101: // gekippt (Hebel oben)
    setWindowHandleKOs(false, true, _channelIndex);
    logDebug("RPS", "gekippt");
    break;
  case 0b1100: // offen (Hebel waagerecht)
  case 0b1110:
    setWindowHandleKOs(true, false, _channelIndex);
    logDebug("RPS", "offen");
    break;
  default:
    logDebug("RPS", "RPS ERROR");
    break;
  }
}
