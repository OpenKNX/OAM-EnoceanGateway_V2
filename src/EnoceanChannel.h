#pragma once
#include "OpenKNX.h"


#define CH_inaktive        0
#define EEP_Profil         1
#define Hersteller_Eltako  2

// ETS-Parameter "Einrichtung des Geräts" (CHSetupMode)
#define ENO_SETUP_MANUAL   0
#define ENO_SETUP_TEACHIN  1

// Bidirektionale Heizkörper-Stellantriebe (EnoceanChannel::hvacProfile)
#define HVAC_NONE   0
#define HVAC_A52001 1
#define HVAC_A52004 2
#define HVAC_A52006 3
// _hvacInterval: Aufwachzyklus/Funkintervall aus der ETS verwenden (kein Wert per KNX)
#define HVAC_INTERVAL_ETS 0xFF




// Full definition lives in EnoceanModule.h; only a pointer is needed here,
// so a forward declaration avoids a circular include (EnoceanModule.h
// already includes EnoceanChannel.h).
struct PACKET_SERIAL_TYPE_;

// Hier definiert (statt in EnoceanModule.h), da diese Datei EnoceanModule.h wegen des zirkulären Includes nicht einbinden kann.
#define RockerIdle 0x01

// Taster-Emulation KNX -> EnOcean: Dauer eines kurzen Tastendrucks und maximale Haltedauer beim Dimmen (ms)
#define ROCKER_TAP_MS 150
#define ROCKER_HOLD_MAX_MS 10000

union EnoceanHandle {
  uint8_t msg_sent_after_receive;           // Für MVA-005  A5-20-06
  uint8_t rockerState_pressed = RockerIdle; // ROCKER
};

// Definition liegt in EnoceanChannel.cpp, um Mehrfachdefinitionen (ODR-Verletzung) zu vermeiden,
// da diese Header-Datei in mehrere .cpp-Dateien eingebunden wird.
extern EnoceanHandle unionMSG;

class EnoceanChannel : public OpenKNX::Channel {
private:
  // Zustand des Tasters (Kanal-Profil Rocker): gedrückte logische Wippe (0xFF = keine), Seite, Zeitpunkt, langer Druck
  uint8_t _rockerWippe = 0xFF;
  bool _rockerUpper = false;
  bool _rockerLong = false;
  uint32_t _rockerPressTime = 0;

  // Taster-Emulation KNX -> EnOcean: Zeitpunkt für das Loslass-Telegramm (0 = keines ausstehend) und zuletzt
  // gesendeter Tastencode (für Jalousie-Stopp)
  uint32_t _knxRockerReleaseAt = 0;
  uint8_t _knxRockerLastDb0 = 0;

  // Ausfallüberwachung: Zeitpunkt des letzten Telegramms des Geräts und ob der Ausfall bereits gemeldet wurde
  uint32_t _lastSeen = 0;
  bool _failed = false;

  // Bidirektionale Heizungsprofile A5-20-01/04/06: Werte von KNX, die mit der nächsten Antwort an das Gerät gehen.
  // Der Antrieb hört nur kurz nach seinem eigenen Telegramm zu, daher wird nie direkt bei KNX-Empfang gesendet.
  // Rohwerte jeweils in der Codierung des Profils (siehe handle4BS_A5_20_xx in 4BS_Telegram.h).
  uint8_t _hvacSetpoint = 0;    // DB3: Ventilstellung 0..100 % oder Solltemperatur
  uint8_t _hvacRoomTemp = 0;    // DB2: Raumtemperatur (01, 06) bzw. Display-Solltemperatur (04)
  uint8_t _hvacFlags = 0;       // dauerhafte Bits (Sommerbetrieb, Standby, Tastensperre)
  uint8_t _hvacOneShot = 0;     // Befehle, die nur einmal gesendet werden (Referenzfahrt, Service)
  uint8_t _hvacInterval = HVAC_INTERVAL_ETS; // Aufwachzyklus (04) bzw. Funkintervall (06) per KNX
  bool _hvacSetpointValid = false; // ohne Sollwert von KNX wird nicht geantwortet
  bool _hvacRoomTempValid = false; // sonst Standardwert je Profil (01: gemessene Temperatur, 04: 20 °C, 06: interner Fühler)
  uint8_t _hvacReply[4] = {0, 0, 0, 0}; // vorbereitete Antwort (DB3..DB0), gesendet in loop()
  bool _hvacReplyPending = false;
  bool _hvacReplyIsTeach = false;
  uint8_t _hvacDest[4] = {0, 0, 0, 0}; // ID des Antriebs (Ziel adressierter Antworten)
  uint32_t _hvacRxTime = 0;            // Empfangszeit, für die Laufzeit der Antwort im Log

  void handleRocker(PACKET_SERIAL_TYPE_ *pPacket);
  void knxRockerPress(uint8_t db0, uint32_t releaseAfterMs);
  void deviceSeen();
  void checkFailure();
  uint8_t hvacProfile() const;
  void handleHvac(PACKET_SERIAL_TYPE_ *pPacket);
  void prepareHvacTeachResponse(PACKET_SERIAL_TYPE_ *pPacket);
  void prepareHvacReply(PACKET_SERIAL_TYPE_ *pPacket);
  void sendPendingHvacReply();

public:
  EnoceanChannel(uint8_t index);
  const std::string name() override;
  void setup() override;
  void loop() override;
  bool check_Eno_ID(PACKET_SERIAL_TYPE_ *pPacket);
  bool matchesDeviceId(const uint8_t id[4]) const;
  void processKnxRocker(GroupObject &ko);
  bool sendLearnTelegram();
  bool processKnxHvac(int koIndex, GroupObject &ko);
  void teachInReceived(PACKET_SERIAL_TYPE_ *pPacket);
};