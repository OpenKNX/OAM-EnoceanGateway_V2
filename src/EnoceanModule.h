#pragma once
#include "EnoceanChannel.h"
#include "OpenKNX.h"
#ifdef ARDUINO_ARCH_RP2040
#ifndef OPENKNX_USB_EXCHANGE_IGNORE
#include "UsbExchangeModule.h"
#endif
#endif

#define ENOCEAN_OK 0
#define ENOCEAN_OUT_OF_RANGE 21
#define ENOCEAN_NOT_VALID_CHKSUM 7
#define ENOCEAN_NO_RX_TEL 6
#define ENOCEAN_NEW_RX_BYTE 3

/*
    EEP type
*/

// Receive
#define u8RORG_1BS 0xD5    // 213
#define u8RORG_RPS 0xF6    // 246
#define u8RORG_VLD 0xD2    // 210
#define u8RORG_4BS 0xA5    // 165
#define u8RORG_UTE 0xD4    // Universal Teach-In (VLD-Geräte)
// Verschlüsselte Telegramme; nach dem Entschlüsseln durch den TCM bleibt der R-ORG, die Daten sind entschlüsselt
#define u8RORG_SEC 0x30
#define u8RORG_SEC_CDM 0x31
#define u8RORG_SEC_ENCAPS 0x32
#define u8RORG_SEC_CDM_ENCAPS 0x33

inline bool isSecureRorg(uint8_t rorg) { return rorg >= u8RORG_SEC && rorg <= u8RORG_SEC_CDM_ENCAPS; }
#define u8RORG_Rocker 0xFA // 250

// Send
#define u8RORG_COMMON_COMMAND 0x05
#define u8KNX_RORG_Rocker 1
#define u8KNX_RORG_VLD 2
#define u8KNX_RORG_4BS 3

/*
    Packet type
*/

#define u8RADIO_ERP1 0x01
#define u8RESPONSE 0x02
#define u8EVENT 0x04
#define u8COMMAND_ACCEPTED 0x0C

// EVENT Codes
#define u8CO_READY 0x04
#define u8CO_DUTYCYCLE_LIMIT 0x06
#define u8CO_EVENT_SECUREDEVICES 0x05
#define u8CO_TRANSMIT_FAILED 0x07
#define u8CO_TX_DONE 0x08
#define u8CO_LRN_MODE_DISABLED 0x09

// COMANDS
#define u8CO_RD_VERSION 0x03
#define u8CO_WR_IDBASE 0x07
#define u8CO_RD_IDBASE 0x08
#define u8CO_WR_REPEATER 0x09
#define u8CO_RD_REPEATER 0x0A // 10
#define u8CO_WR_LEARNMODE 0x17 // 23
#define u8CO_RD_LEARNMODE 0x18 // 24
#define u8CO_WR_SECUREDEVICE_DEL 0x1A

/*
    Teach-In über die ETS (Buttons + Script, siehe EnoceanModule.script.js)
*/

// FunctionProperty-Adresse für die ETS-Buttons (Objekt 160, Property 7; Logik nutzt 160/4, Meter 160/2, Network 160/5)
#define ENO_FP_OBJECT 160
#define ENO_FP_PROPERTY 7

// Befehle (data[0])
#define ENO_FP_CMD_STATUS 1   // data[1] = Kanal 1..30
#define ENO_FP_CMD_START 2    // data[1] = Kanal 1..30
#define ENO_FP_CMD_DELETE 3   // data[1] = Kanal 1..30
#define ENO_FP_CMD_BASEID 4   // Base-ID, Schreibzyklen, EURID
#define ENO_FP_CMD_VERSION 5  // App- und API-Version des Transceivers
#define ENO_FP_CMD_LEARN 6    // data[1] = Kanal 1..30: Lerntelegramm an einen Aktor senden

// Ergebniscodes (resultData[0])
#define ENO_FP_OK 0
#define ENO_FP_ERR_NOT_CONFIGURED 1
#define ENO_FP_ERR_ALREADY_LEARNED 2
#define ENO_FP_ERR_BUSY 3
#define ENO_FP_ERR_CHANNEL 4
#define ENO_FP_ERR_TRANSCEIVER 5
#define ENO_FP_ERR_COMMAND 6
#define ENO_FP_ERR_PROFILE 7 // Kanal sendet nicht (kein Profil mit Richtung KNX -> EnOcean)

// Teach-In-Status eines Kanals
#define ENO_TEACH_FREE 0
#define ENO_TEACH_RUNNING 1
#define ENO_TEACH_LEARNED 2
#define ENO_TEACH_ERROR 3

// Flags einer eingelernten ID
#define ENO_TEACH_FLAG_SECURE 0x01

// Version des Flash-Formats der eingelernten IDs (bei Formatänderung erhöhen)
#define ENO_FLASH_VERSION 1

// Eingelernte ID eines Kanals (im OpenKNX-Flash gespeichert)
struct EnoceanTeachEntry
{
    uint8_t valid;  // 1 = eingelernt
    uint8_t flags;  // ENO_TEACH_FLAG_*
    uint8_t id[4];  // Sender-ID
    uint8_t eep[3]; // R-ORG, FUNC, TYPE (0 = unbekannt)
};

#define RPS_BUTTON_CHANNEL_AI 0
#define RPS_BUTTON_CHANNEL_AO 1
#define RPS_BUTTON_CHANNEL_BI 2
#define RPS_BUTTON_CHANNEL_BO 3

#define RPS_BUTTON_2NDACT_NO 0
#define RPS_BUTTON_2NDACT_VALID 1

#define VLD_CMD_ID_01 0x01
#define VLD_CMD_ID_02 0x02
#define VLD_CMD_ID_03 0x03
#define VLD_CMD_ID_04 0x04
#define VLD_CMD_ID_05 0x05
#define VLD_CMD_ID_06 0x06

#define RPS_Func_10 0xA

#define BASEID_BYTES 4
#define DATBUF_SZ 100

// Max. Wartezeit auf eine RESPONSE / COMMAND_ACCEPTED des Transceivers (ms), ESP3-Spec Kap. 1.10
#define ENOCEAN_RESPONSE_TIMEOUT 500
// Obergrenze, um die nach einem COMMAND_ACCEPTED zusätzlich auf die RESPONSE gewartet wird (ms)
#define ENOCEAN_ACCEPTED_TIMEOUT_MAX 2000
// Max. Pause zwischen zwei Bytes eines ESP3-Pakets, danach Neusynchronisation (ms). Bei 57600 Baud dauert ein Byte
// ca. 0,17 ms, ein komplettes Paket < 10 ms.
#define ENOCEAN_INTERBYTE_TIMEOUT 100

#define SER_SYNCH_CODE 0x55
#define SER_HEADER_NR_BYTES 4

//Rocker States
#define ButtonStateO 0
#define ButtonStateI 1

#define AI_pressed 0x10
#define AO_pressed 0x30
#define BI_pressed 0x50
#define BO_pressed 0x70
#define CI_pressed 0x15
#define CO_pressed 0x37
#define Contact_pressed 0xF0

#define AI_release 0x00
#define AO_release 0x20
#define BI_release 0x40
#define BO_release 0x60
#define CI_release 0x05
#define CO_release 0x07
#define Contact_release 0xE0

#define ROCKER_INACTIVE 0x00
#define ROCKER_AI 0x01
#define ROCKER_AO 0x02
#define ROCKER_BI 0x03
#define ROCKER_BO 0x04
#define ROCKER_CI 0x05
#define ROCKER_CO 0x06

//! uart_getPacket state machine states.
enum STATES_GET_PACKET_
{
    //! Waiting for the synchronisation byte 0x55
    GET_SYNC_STATE = 0,
    //! Copying the 4 after sync byte: raw data length (2 bytes), optional data length (1), type (1).
    GET_HEADER_STATE,
    //! Checking the header CRC8 checksum. Resynchronisation test is also done here
    CHECK_CRC8H_STATE,
    //! Copying the data and optional data bytes to the paquet buffer
    GET_DATA_STATE,
    //! Checking the info CRC8 checksum.
    CHECK_CRC8D_STATE,
};

//! Packet structure (ESP3)
struct PACKET_SERIAL_TYPE_
{
    // Amount of raw data bytes to be received. The most significant byte is sent/received first
    uint16_t u16DataLength;
    // Amount of optional data bytes to be received
    uint8_t u8OptionLength;
    // Packetype code
    uint8_t u8Type;
    // Data buffer: raw data + optional bytes
    uint8_t *u8DataBuffer;
};

class EnoceanModule : public OpenKNX::Module
{
private:
    uint32_t _timer1 = 0;
    uint32_t _timer2 = 0;
    uint8_t _currentChannel = 0;
    Stream *_serial;
    EnoceanChannel *_channels[ENO_ChannelCount];
    OpenKNX::Flash::Driver *_dummyStorage = nullptr;

    void begin();
    void setupCustomFlash();
    void setupChannels();
    void initSerial(Stream &serial);
    void task();
    bool readBaseId(uint8_t *fui8_BaseID_p);
    void setBaseId(const uint8_t *fui8_BaseID_p);
    void getConfiguredBaseId(uint8_t out8BaseId[BASEID_BYTES]);
    bool checkBaseID();
    uint8_t waitForResponse(uint32_t timeoutMs);
    static bool isValidBaseId(const uint8_t baseId[BASEID_BYTES]);
    bool activateLearnMode(uint8_t channel, uint8_t enable, uint32_t timeoutMs = 0);
    uint8_t startTeachIn(uint8_t channel);
    void stopTeachIn();
    bool tryTeachIn(PACKET_SERIAL_TYPE_ *f_Pkt_st);
    void storeTeachIn(const uint8_t id[4], uint8_t rorg, uint8_t func, uint8_t type, uint8_t flags);
    void finishTeachIn();
    bool isIdAssigned(const uint8_t id[4]);
    void deleteTeachIn(uint8_t channel);
    void deleteSecureDevice(const uint8_t id[4]);
    uint32_t teachWindowMs();
    bool readLearnMode(uint8_t *fu8_Enable_p, uint8_t *fu8_Channel_p);
    void handleTeachCommand(uint8_t value);
    void setIsTeachChannel(uint8_t channel);
    void disableLearnMode();
    void reportTeachChannel();
    void handleEvent(PACKET_SERIAL_TYPE_ *f_Pkt_st);
    void readVersion();
    void logReceivedTelegram(PACKET_SERIAL_TYPE_ *f_Pkt_st);
    void readRepeaterFunc();
    void setRepeaterFunc(uint8_t mode, uint8_t level);
    void getEnOceanMSG(uint8_t u8RetVal, PACKET_SERIAL_TYPE_ *f_Pkt_st);
    void checkAndReportUnknownId(PACKET_SERIAL_TYPE_ *f_Pkt_st);
    void handleKnxEvent(uint8_t _channelIndex, int koIndex, GroupObject &ko);
    bool sendChannelTelegram(uint8_t channelIndex, uint8_t rorg, const uint8_t *data, uint8_t dataLength, uint8_t status, const char *description, const uint8_t *destId = nullptr);
    void setStatusActors(uint8_t *mySenderId, uint8_t idExtra, bool state);
    void getStatusActors(uint8_t *mySenderId, uint8_t idExtra);
    void setActorsMeasurment(uint8_t *mySenderId, uint8_t idExtra, uint8_t *inputs);
    void getActorsMeasurmentValue(uint8_t *mySenderId, uint8_t idExtra, uint8_t *inputs, bool unit);
    uint8_t uart_getPacket(PACKET_SERIAL_TYPE_ *pPacket, uint16_t u16BufferLength);
    uint8_t uart_sendPacket(PACKET_SERIAL_TYPE_ *pPacket);
    bool sendRadioPacket(PACKET_SERIAL_TYPE_ *pPacket, const char *description);
#ifdef ARDUINO_ARCH_RP2040
#ifndef OPENKNX_USB_EXCHANGE_IGNORE
    void registerUsbExchangeCallbacks();
#endif
#endif

    bool isInited = false;
    // true, sobald _channels angelegt sind (nur dann dürfen Funktelegramme an die Kanäle verteilt werden)
    bool _channelsReady = false;

    uint8_t u8datBuf[DATBUF_SZ];
    uint8_t u8CRC;
    uint8_t u8RxByte;
    uint8_t u8RetVal;

    STATES_GET_PACKET_ u8State;
    PACKET_SERIAL_TYPE_ m_Pkt_st;

    static uint8_t u8CRC8Table[256];

    uint8_t lui8_BaseID_p[BASEID_BYTES];
    // verbleibende Base-ID-Schreibzyklen laut Transceiver (0xFF = unbegrenzt)
    uint8_t _baseIdRemainingWrites = 0xFF;

    uint32_t _lastPollingTime = 0;

    // Konsole "eno monitor on": jedes empfangene Funktelegramm loggen (auch mit ETS-Konfiguration)
    bool _monitor = false;

    // Transceiver-Info (beim Start gelesen, für "eno version" und die ETS)
    bool _versionValid = false;
    uint8_t _eurid[4] = {0, 0, 0, 0};
    uint8_t _appVersion[4] = {0, 0, 0, 0};
    uint8_t _apiVersion[4] = {0, 0, 0, 0};

    // Teach-In: eingelernte IDs je Kanal (Flash), laufendes Teach-In-Fenster und letzter Fehler
    EnoceanTeachEntry _teach[ENO_ChannelCount] = {};
    uint8_t _teachChannel = 0; // 1..30, 0 = kein Teach-In aktiv
    uint32_t _teachStart = 0;
    uint8_t _teachErrorChannel = 0;
    uint8_t _teachErrorCause = 0; // Ursache aus CO_EVENT_SECUREDEVICES

public:
    EnoceanModule();
    void loop(bool configured) override;
    void setup(bool configured) override;
#ifdef OPENKNX_DUALCORE
    void loop1(bool configured) override;
    void setup1(bool configured) override;
#endif
    const std::string name() override;
    const std::string version() override;
    void processInputKo(GroupObject &ko) override;
    bool processCommand(const std::string cmd, bool diagnoseKo);
    void showHelp() override;
    uint16_t flashSize() override;
    void writeFlash() override;
    void readFlash(const uint8_t *data, const uint16_t size) override;
    bool processFunctionProperty(uint8_t objectIndex, uint8_t propertyId, uint8_t length, uint8_t *data, uint8_t *resultData, uint8_t &resultLength) override;
    static bool extractSenderId(const PACKET_SERIAL_TYPE_ *f_Pkt_st, uint8_t out8SenderId[4]);
    bool sendRockerTelegram(uint8_t channelIndex, uint8_t db0);
    bool send4BSTelegram(uint8_t channelIndex, const uint8_t data[4], const char *description, const uint8_t *destId = nullptr);
    bool learnedId(uint8_t channelIndex, uint8_t out8Id[4]);
};

extern EnoceanModule openknxEnoceanModule;