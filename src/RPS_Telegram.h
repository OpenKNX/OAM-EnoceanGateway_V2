#pragma once

#include "EnoceanModule.h"
#include "EnoceanProfils.h"
#include "EnoceanStructs.h"
#include "knxprod.h"

/*******************************************************************************************
 *  Rocker / Taster (Kanal-Profil "Rocker", EnOcean -> KNX), Telegramme F6-02-xx
 *
 *  DB0: Bit 7-5 R1 (0 = AI, 1 = AO, 2 = BI, 3 = BO), Bit 4 EB (1 = gedrückt, 0 = losgelassen),
 *       Bit 3-1 R2 (zweite Taste), Bit 0 SA (zweite Taste gültig).
 *  "O" = Wippe oben, "I" = Wippe unten.
 *
 *  KO-Belegung (ETS / KO-Tabelle):
 *    Wippe A : KO_1 (GO_BASE__2) Schalten / AUF-AB / Szene,  KO_2 (GO_BASE__3) Dimmen / Lamelle-Stopp
 *    Wippe B : KO_3 (GO_BASE__4)                              KO_4 (GO_BASE__5)
 *    Wippe AB: KO_5 (GO_BASE__6)                              KO_6 (GO_BASE__7)
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

// Erstes KO der logischen Wippe (Schalten / AUF-AB / Szene).
inline GroupObject &rockerKoMain(uint8_t wippe, uint8_t _channelIndex) {
  switch (wippe) {
  case ROCKER_WIPPE_A:
    return KoENO_GO_BASE__2;
  case ROCKER_WIPPE_B:
    return KoENO_GO_BASE__4;
  default:
    return KoENO_GO_BASE__6;
  }
}

// Zweites KO der logischen Wippe (Dimmen / Lamelle-Stopp).
inline GroupObject &rockerKoSecond(uint8_t wippe, uint8_t _channelIndex) {
  switch (wippe) {
  case ROCKER_WIPPE_A:
    return KoENO_GO_BASE__3;
  case ROCKER_WIPPE_B:
    return KoENO_GO_BASE__5;
  default:
    return KoENO_GO_BASE__7;
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

  switch (ParamENO_CHRockerProfil) {
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

// Verarbeitet die F6-05-xx-Telegramme (Rauch-/Wassermelder bzw. Energiestatus) und schreibt den passenden
// Alarm-/Statuswert.
// KO-Belegung laut ETS: KO_3 (GO_BASE__4) = Alarm, KO_4 (GO_BASE__5) = Energy Low.
inline void handle_F6_05_0x(uint8_t value, uint8_t _channelIndex) {
  switch (value) {
  case 0x00: // SMOKE/WIND Alarm OFF
    KoENO_GO_BASE__4.value(false, DPT_Bool);
    break;
  case 0x10: // SMOKE/WIND Alarm ON
    KoENO_GO_BASE__4.value(true, DPT_Bool);
    break;
  case 0x11: // Water detected
    KoENO_GO_BASE__4.value(true, DPT_Bool);
    break;
  case 0x30: // Energy LOW
    KoENO_GO_BASE__5.value(true, DPT_Bool);
    break;
  default:
    break;
  }
}

// Schreibt den Fenstergriff-/Kontaktzustand auf die Gruppenobjekte (Belegung wie A5-14-09):
// KO2 = offen, KO3 = gekippt, KO4 = offen/geschlossen (Wert für "geschlossen" per ETS-Parameter).
inline void setWindowHandleKOs(bool open, bool tilted, uint8_t _channelIndex) {
  bool closed = !open && !tilted;
  KoENO_GO_BASE__2.value(open, Dpt(1, 1));
  KoENO_GO_BASE__3.value(tilted, Dpt(1, 1));
  KoENO_GO_BASE__4.value(closed ? ParamENO_CHWindowcloseValue : !ParamENO_CHWindowcloseValue, Dpt(1, 1));
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
  case F6_05_00:
    logDebug("RPS", "Profil: F6-05-00");
    handle_F6_05_0x(f_Pkt_st->u8DataBuffer[1], _channelIndex);
    break;
  //**************************************************************
  // ----------------- Profil: F6-05-01 --------------------------
  //**************************************************************
  case F6_05_01:
    logDebug("RPS", "Profil: F6-05-01");
    handle_F6_05_0x(f_Pkt_st->u8DataBuffer[1], _channelIndex);
    break;
    //**************************************************************
  // ----------------- Profil: F6-05-02   ------------------------
  //**************************************************************
  case F6_05_02:
    logDebug("RPS", "Profil: F6-05-02");
    handle_F6_05_0x(f_Pkt_st->u8DataBuffer[1], _channelIndex);
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
