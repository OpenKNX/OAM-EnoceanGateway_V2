#pragma once

#include "EnoceanModule.h"
#include "EnoceanProfils.h"
#include "EnoceanStructs.h"
#include "knxprod.h"

// Vorwärtsdeklaration: Definition steht weiter unten in dieser Datei, wird aber schon innerhalb von handle_4BS
// aufgerufen.
inline void handle4BS_A5_04_02(PACKET_SERIAL_TYPE_ *f_Pkt_st, uint8_t _channelIndex);
inline void handle4BS_A5_04_03(PACKET_SERIAL_TYPE_ *f_Pkt_st, uint8_t _channelIndex);
inline void handle4BS_A5_06_01_V2(PACKET_SERIAL_TYPE_ *f_Pkt_st, uint8_t _channelIndex);
inline void handle4BS_A5_07_01(PACKET_SERIAL_TYPE_ *f_Pkt_st, uint8_t _channelIndex);
inline void handle4BS_A5_08_01(PACKET_SERIAL_TYPE_ *f_Pkt_st, uint8_t _channelIndex);
inline void handle4BS_A5_09_04(PACKET_SERIAL_TYPE_ *f_Pkt_st, uint8_t _channelIndex);
inline void handle4BS_A5_09_05(PACKET_SERIAL_TYPE_ *f_Pkt_st, uint8_t _channelIndex);
inline void handle4BS_A5_09_0C(PACKET_SERIAL_TYPE_ *f_Pkt_st, uint8_t _channelIndex);
inline void handle4BS_A5_14_09(PACKET_SERIAL_TYPE_ *f_Pkt_st, uint8_t _channelIndex);
inline void handle4BS_A5_30_03(PACKET_SERIAL_TYPE_ *f_Pkt_st, uint8_t _channelIndex);
inline void handle4BS_D5_00_01_V2(PACKET_SERIAL_TYPE_ *f_Pkt_st, uint8_t _channelIndex);
inline uint8_t handle4BS_A5_20_01(PACKET_SERIAL_TYPE_ *f_Pkt_st, uint8_t _channelIndex);
inline uint8_t handle4BS_A5_20_04(PACKET_SERIAL_TYPE_ *f_Pkt_st, uint8_t _channelIndex);
inline uint8_t handle4BS_A5_20_06(PACKET_SERIAL_TYPE_ *f_Pkt_st, uint8_t _channelIndex);


inline uint8_t handle_4BS(PACKET_SERIAL_TYPE_ *f_Pkt_st, uint8_t _channelIndex) {
  uint32_t value_4Byte;
  uint16_t lux;
  float luxfloat;
  float temp;
  float hum;
  bool bvalue;

  FOURBS_A5_02_TYPE *fourBsA5_02_Tlg_p;
  FOURBS_A5_02_2030TYPE *fourBsA5_02_2030_Tlg_p;
  FOURBS_A5_04_TYPE *fourBsA5_04_Tlg_p;
  FOURBS_A5_04_03_TYPE *fourBsA5_04_03_Tlg_p;
  FOURBS_A5_06_01_TYPE *fourBsA5_06_01_Tlg_p;
  FOURBS_A5_06_01_V2_TYPE *fourBsA5_06_01_V2_Tlg_p;
  FOURBS_A5_06_01_TYPE *fourBsA5_06_02_Tlg_p;
  FOURBS_A5_06_03_TYPE *fourBsA5_06_03_Tlg_p;
  FOURBS_A5_07_01_TYPE *fourBsTlg2_p;
  FOURBS_A5_07_02_TYPE *fourBsTlg_p;
  FOURBS_A5_07_03_TYPE *fourBsTlg3_p;
  FOURBS_A5_08_TYPE *fourBsA5_08_Tlg_p;
  FOURBS_A5_12_01_TYPE *fourBsA5_12_01_Tlg_p;
  FOURBS_A5_14_01_06_TYPE *fourBsA5_14_01_06_Tlg_p;
  FOURBS_A5_14_07_08_TYPE *fourBsA5_14_07_08_Tlg_p;
  FOURBS_A5_14_09_0A_TYPE *fourBsA5_14_09_0A_Tlg_p;

  // Special Profil for a 4BS MSG from a 1BS Device

  logDebug("4BS", "Profil: 4BS - %u", ParamENO_CHProfilSelection4BS);

  // #ifndef EnOceanTEST
  switch (ParamENO_CHProfilSelection4BS)
  // #else
  //       switch (profil)
  // #endif
  {
  case A5_02:
    logDebug("4BS", "A5-02-");
    // switch (knx.paramWord(firstParameter + ENO_CHProfil4BS02))
    switch (ParamENO_CHProfil4BS02) {
    //**************************************************************
    // ----------------- Profil: A5-02-01 --------------------------
    //**************************************************************
    case A5_02_01:
      fourBsA5_02_Tlg_p = (FOURBS_A5_02_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
      temp = (float)(fourBsA5_02_Tlg_p->Temp / -6.375);
      KoENO_GO_BASE__2.value(temp, Dpt(9, 1));
      logDebug("4BS", "01 Temp: %.1f", temp);
      break;
    //**************************************************************
    // ----------------- Profil: A5-02-02 --------------------------
    //**************************************************************
    case A5_02_02:
      fourBsA5_02_Tlg_p = (FOURBS_A5_02_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
      temp = (float)(fourBsA5_02_Tlg_p->Temp / -6.375) + 10.0;
      KoENO_GO_BASE__2.value(temp, Dpt(9, 1));
      logDebug("4BS", "02 Temp: %.1f", temp);
      break;
    //**************************************************************
    // ----------------- Profil: A5-02-03 --------------------------
    //**************************************************************
    case A5_02_03:
      fourBsA5_02_Tlg_p = (FOURBS_A5_02_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
      temp = (float)(fourBsA5_02_Tlg_p->Temp / -6.375) + 20.0;
      KoENO_GO_BASE__2.value(temp, Dpt(9, 1));
      logDebug("4BS", "03 Temp: %.1f", temp);
      break;
    //**************************************************************
    // ----------------- Profil: A5-02-04 --------------------------
    //**************************************************************
    case A5_02_04:
      fourBsA5_02_Tlg_p = (FOURBS_A5_02_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
      temp = (float)(fourBsA5_02_Tlg_p->Temp / -6.375) + 30.0;
      KoENO_GO_BASE__2.value(temp, Dpt(9, 1));
      logDebug("4BS", "04 Temp: %.1f", temp);
      break;
    //**************************************************************
    // ----------------- Profil: A5-02-05 --------------------------
    //**************************************************************
    case A5_02_05:
      fourBsA5_02_Tlg_p = (FOURBS_A5_02_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
      temp = (float)(fourBsA5_02_Tlg_p->Temp / -6.375) + 40.0;
      KoENO_GO_BASE__2.value(temp, Dpt(9, 1));
      logDebug("4BS", "05 Temp: %.1f", temp);
      break;
    //**************************************************************
    // ----------------- Profil: A5-02-06 --------------------------
    //**************************************************************
    case A5_02_06:
      fourBsA5_02_Tlg_p = (FOURBS_A5_02_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
      temp = (float)(fourBsA5_02_Tlg_p->Temp / -6.375) + 50.0;
      KoENO_GO_BASE__2.value(temp, Dpt(9, 1));
      logDebug("4BS", "06 Temp: %.1f", temp);
      break;
    //**************************************************************
    // ----------------- Profil: A5-02-07 --------------------------
    //**************************************************************
    case A5_02_07:
      fourBsA5_02_Tlg_p = (FOURBS_A5_02_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
      temp = (float)(fourBsA5_02_Tlg_p->Temp / -6.375) + 60.0;
      KoENO_GO_BASE__1.value(temp, Dpt(9, 1));
      logDebug("4BS", "07 Temp: %.1f", temp);
      break;
    //**************************************************************
    // ----------------- Profil: A5-02-08 --------------------------
    //**************************************************************
    case A5_02_08:
      fourBsA5_02_Tlg_p = (FOURBS_A5_02_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
      temp = (float)(fourBsA5_02_Tlg_p->Temp / -6.375) + 70.0;
      KoENO_GO_BASE__2.value(temp, Dpt(9, 1));
      logDebug("4BS", "08 Temp: %.1f", temp);
      break;
    //**************************************************************
    // ----------------- Profil: A5-02-09 --------------------------
    //**************************************************************
    case A5_02_09:
      fourBsA5_02_Tlg_p = (FOURBS_A5_02_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
      temp = (float)(fourBsA5_02_Tlg_p->Temp / -6.375) + 80.0;
      KoENO_GO_BASE__2.value(temp, Dpt(9, 1));
      logDebug("4BS", "09 Temp: %.1f", temp);
      break;
    //**************************************************************
    // ----------------- Profil: A5-02-0A --------------------------
    //**************************************************************
    case A5_02_0A:
      fourBsA5_02_Tlg_p = (FOURBS_A5_02_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
      temp = (float)(fourBsA5_02_Tlg_p->Temp / -6.375) + 90.0;
      KoENO_GO_BASE__2.value(temp, Dpt(9, 1));
      logDebug("4BS", "0A Temp: %.1f", temp);
      break;
    //**************************************************************
    // ----------------- Profil: A5-02-0B --------------------------
    //**************************************************************
    case A5_02_0B:
      fourBsA5_02_Tlg_p = (FOURBS_A5_02_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
      temp = (float)(fourBsA5_02_Tlg_p->Temp / -6.375) + 100.0;
      KoENO_GO_BASE__2.value(temp, Dpt(9, 1));
      logDebug("4BS", "0B Temp: %.1f", temp);
      break;
    //**************************************************************
    // ----------------- Profil: A5-02-10 --------------------------
    //**************************************************************
    case A5_02_10:
      fourBsA5_02_Tlg_p = (FOURBS_A5_02_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
      temp = (float)(fourBsA5_02_Tlg_p->Temp / -3.1875) + 20.0;
      KoENO_GO_BASE__2.value(temp, Dpt(9, 1));
      logDebug("4BS", "10 Temp: %.1f", temp);
      break;
    //**************************************************************
    // ----------------- Profil: A5-02-11 --------------------------
    //**************************************************************
    case A5_02_11:
      fourBsA5_02_Tlg_p = (FOURBS_A5_02_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
      temp = (float)(fourBsA5_02_Tlg_p->Temp / -3.1875) + 30.0;
      KoENO_GO_BASE__2.value(temp, Dpt(9, 1));
      logDebug("4BS", "11 Temp: %.1f", temp);
      break;
    //**************************************************************
    // ----------------- Profil: A5-02-12 --------------------------
    //**************************************************************
    case A5_02_12:
      fourBsA5_02_Tlg_p = (FOURBS_A5_02_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
      temp = (float)(fourBsA5_02_Tlg_p->Temp / -3.1875) + 40.0;
      KoENO_GO_BASE__2.value(temp, Dpt(9, 1));
      logDebug("4BS", "12 Temp: %.1f", temp);
      break;
    //**************************************************************
    // ----------------- Profil: A5-02-13 --------------------------
    //**************************************************************
    case A5_02_13:
      fourBsA5_02_Tlg_p = (FOURBS_A5_02_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
      temp = (float)(fourBsA5_02_Tlg_p->Temp / -3.1875) + 50.0;
      KoENO_GO_BASE__2.value(temp, Dpt(9, 1));
      logDebug("4BS", "13 Temp: %.1f", temp);
      break;
    //**************************************************************
    // ----------------- Profil: A5-02-14 --------------------------
    //**************************************************************
    case A5_02_14:
      fourBsA5_02_Tlg_p = (FOURBS_A5_02_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
      temp = (float)(fourBsA5_02_Tlg_p->Temp / -3.1875) + 60.0;
      KoENO_GO_BASE__2.value(temp, Dpt(9, 1));
      logDebug("4BS", "14 Temp: %.1f", temp);
      break;
    //**************************************************************
    // ----------------- Profil: A5-02-15 --------------------------
    //**************************************************************
    case A5_02_15:
      fourBsA5_02_Tlg_p = (FOURBS_A5_02_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
      temp = (float)(fourBsA5_02_Tlg_p->Temp / -3.1875) + 70.0;
      KoENO_GO_BASE__2.value(temp, Dpt(9, 1));
      logDebug("4BS", "15 Temp: %.1f", temp);
      break;
    //**************************************************************
    // ----------------- Profil: A5-02-16 --------------------------
    //**************************************************************
    case A5_02_16:
      fourBsA5_02_Tlg_p = (FOURBS_A5_02_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
      temp = (float)(fourBsA5_02_Tlg_p->Temp / -3.1875) + 80.0;
      KoENO_GO_BASE__2.value(temp, Dpt(9, 1));
      logDebug("4BS", "16 Temp: %.1f", temp);
      break;
    //**************************************************************
    // ----------------- Profil: A5-02-17 --------------------------
    //**************************************************************
    case A5_02_17:
      fourBsA5_02_Tlg_p = (FOURBS_A5_02_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
      temp = (float)(fourBsA5_02_Tlg_p->Temp / -3.1875) + 90.0;
      KoENO_GO_BASE__2.value(temp, Dpt(9, 1));
      logDebug("4BS", "17 Temp: %.1f", temp);
      break;
    //**************************************************************
    // ----------------- Profil: A5-02-18 --------------------------
    //**************************************************************
    case A5_02_18:
      fourBsA5_02_Tlg_p = (FOURBS_A5_02_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
      temp = (float)(fourBsA5_02_Tlg_p->Temp / -3.1875) + 100.0;
      KoENO_GO_BASE__2.value(temp, Dpt(9, 1));
      logDebug("4BS", "18 Temp: %.1f", temp);
      break;
    //**************************************************************
    // ----------------- Profil: A5-02-19 --------------------------
    //**************************************************************
    case A5_02_19:
      fourBsA5_02_Tlg_p = (FOURBS_A5_02_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
      temp = (float)(fourBsA5_02_Tlg_p->Temp / -3.1875) + 110.0;
      KoENO_GO_BASE__2.value(temp, Dpt(9, 1));
      logDebug("4BS", "19 Temp: %.1f", temp);
      break;
    //**************************************************************
    // ----------------- Profil: A5-02-1A --------------------------
    //**************************************************************
    case A5_02_1A:
      fourBsA5_02_Tlg_p = (FOURBS_A5_02_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
      temp = (float)(fourBsA5_02_Tlg_p->Temp / -3.1875) + 120.0;
      KoENO_GO_BASE__2.value(temp, Dpt(9, 1));
      logDebug("4BS", "1A Temp: %.1f", temp);
      break;
    //**************************************************************
    // ----------------- Profil: A5-02-1B --------------------------
    //**************************************************************
    case A5_02_1B:
      fourBsA5_02_Tlg_p = (FOURBS_A5_02_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
      temp = (float)(fourBsA5_02_Tlg_p->Temp / -3.1875) + 130.0;
      KoENO_GO_BASE__2.value(temp, Dpt(9, 1));
      logDebug("4BS", "1B Temp: %.1f", temp);
      break;
    //**************************************************************
    // ----------------- Profil: A5-02-20 --------------------------
    //**************************************************************
    case A5_02_20:
      fourBsA5_02_2030_Tlg_p = (FOURBS_A5_02_2030TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
      temp = (float)(((uint16_t)fourBsA5_02_2030_Tlg_p->TempMSB << 8 | fourBsA5_02_2030_Tlg_p->TempLSB) / -20.0) + 41.2;
      KoENO_GO_BASE__2.value(temp, Dpt(9, 1));
      logDebug("4BS", "20 Temp: %.1f", temp);
      break;
    //**************************************************************
    // ----------------- Profil: A5-02-30 --------------------------
    //**************************************************************
    case A5_02_30:
      fourBsA5_02_2030_Tlg_p = (FOURBS_A5_02_2030TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
      temp = (float)(((uint16_t)fourBsA5_02_2030_Tlg_p->TempMSB << 8 | fourBsA5_02_2030_Tlg_p->TempLSB) / -10.0) + 62.3;
      KoENO_GO_BASE__2.value(temp, Dpt(9, 1));
      logDebug("4BS", "30 Temp: %.1f", temp);
      break;
    default:
      break;
    }
    break; // ENDE A5-02-XX

  case A5_04:
    logDebug("4BS", "A5-04-");
    // switch (knx.paramWord(firstParameter + ENO_CHProfil4BS04))
    switch (ParamENO_CHProfil4BS04) {
    //**************************************************************
    // ----------------- Profil: A5-04-01 --------------------------
    //**************************************************************
    case A5_04_01:
      fourBsA5_04_Tlg_p = (FOURBS_A5_04_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
      temp = (float)(fourBsA5_04_Tlg_p->Temp / 6.25);
      hum = (float)(fourBsA5_04_Tlg_p->Hum / 2.5);
      KoENO_GO_BASE__2.value(temp, Dpt(9, 1));
      KoENO_GO_BASE__3.value(hum, Dpt(9, 1));
      logDebug("4BS", "01 Temp: %.1f Hum: %.1f", temp, hum);
      break;
    //**************************************************************
    // ----------------- Profil: A5-04-02 --------------------------
    //**************************************************************
    case A5_04_02:
      handle4BS_A5_04_02(f_Pkt_st, _channelIndex);
      break;
    //**************************************************************
    // ----------------- Profil: A5-04-03 --------------------------
    //**************************************************************
    case A5_04_03:
      handle4BS_A5_04_03(f_Pkt_st, _channelIndex);
      break;
    default:
      break;
    }
    break; // ENDE A5-04-XX
  case A5_06:
    logDebug("4BS", "A5-06-");
    // switch (knx.paramWord(firstParameter + ENO_CHProfil4BS06))
    switch (ParamENO_CHProfil4BS06) {
    //**************************************************************
    // ----------------- Profil: A5-06-01 --------------------------
    //**************************************************************
    case A5_06_01:
      logDebug("4BS", "01");
      fourBsA5_06_01_Tlg_p = (FOURBS_A5_06_01_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
      switch (fourBsA5_06_01_Tlg_p->u84BsTelData.RS) {
      case 0: // Range 600 ... 60000 lux
        luxfloat = (float)(fourBsA5_06_01_Tlg_p->u8Illumination1 * 232.941176) + 600.0;
        break;
      case 1: // Range 300 ... 30000 lux
        luxfloat = (float)(fourBsA5_06_01_Tlg_p->u8Illumination2 * 116.470588) + 300.0;
        break;
      default:
        luxfloat = 0;
        break;
      }
      KoENO_GO_BASE__2.value(luxfloat, Dpt(9, 1));
      KoENO_GO_BASE__3.value(fourBsA5_06_01_Tlg_p->u8SupplyVoltage / 50.0, Dpt(9, 1));

      logDebug("4BS", "RS: %u LUX: %.1f Bat: %.1f", fourBsA5_06_01_Tlg_p->u84BsTelData.RS, luxfloat,
               fourBsA5_06_01_Tlg_p->u8SupplyVoltage / 50.0);
      break;
    //**************************************************************
    // ----------------- Profil: A5-06-01 Version 2 ----------------
    //**************************************************************
    case A5_06_01_V2:
      handle4BS_A5_06_01_V2(f_Pkt_st, _channelIndex);
      break;
    //**************************************************************
    // ----------------- Profil: A5-06-02 --------------------------
    //**************************************************************
    case A5_06_02:
      logDebug("4BS", "02");
      fourBsA5_06_02_Tlg_p = (FOURBS_A5_06_01_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
      switch (fourBsA5_06_02_Tlg_p->u84BsTelData.RS) {
      case 0: // Range 0 ... 1020 lux
        luxfloat = (float)(fourBsA5_06_02_Tlg_p->u8Illumination1 / 0.25);
        break;
      case 1: // Range 0 ... 510 lux
        luxfloat = (float)(fourBsA5_06_02_Tlg_p->u8Illumination2 / 0.5);
        break;
      default:
        luxfloat = 0;
        break;
      }
      KoENO_GO_BASE__2.value(luxfloat, Dpt(9, 1));
      KoENO_GO_BASE__3.value(fourBsA5_06_02_Tlg_p->u8SupplyVoltage / 50.0, Dpt(9, 1));

      logDebug("4BS", "LUX: %.1f Bat: %.1f", luxfloat, fourBsA5_06_02_Tlg_p->u8SupplyVoltage / 50.0);
      break;
    //**************************************************************
    // ----------------- Profil: A5-06-03 --------------------------
    //**************************************************************
    case A5_06_03:
      logDebug("4BS", "03");
      fourBsA5_06_03_Tlg_p = (FOURBS_A5_06_03_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);

      // ........Illumination.......................................
      lux = (uint16_t)fourBsA5_06_03_Tlg_p->u8LuxMSB << 2 | fourBsA5_06_03_Tlg_p->u84BsTelData.LUX;
      luxfloat = (float)lux * 1.0;

      KoENO_GO_BASE__2.value(luxfloat, Dpt(9, 1));
      KoENO_GO_BASE__3.value(fourBsA5_06_03_Tlg_p->u8SupplyVoltage / 51.0, Dpt(9, 1));

      logDebug("4BS", "LUX: %.1f Bat: %.1f", luxfloat, fourBsA5_06_03_Tlg_p->u8SupplyVoltage / 51.0);
      break;
    default:
      break;
    }
    break; // ENDE A5-06-XX
  case A5_07:
    logDebug("4BS", "A5-07-");
    // switch (knx.paramWord(firstParameter + ENO_CHProfil4BS07))
    switch (ParamENO_CHProfil4BS07) {
    //**************************************************************
    // ----------------- Profil: A5-07-01 --------------------------
    //**************************************************************
    case A5_07_01:
      handle4BS_A5_07_01(f_Pkt_st, _channelIndex);
      break; // ENDE A5-07-01
    //**************************************************************
    // ----------------- Profil: A5-07-02 --------------------------
    //**************************************************************
    case A5_07_02:
      fourBsTlg_p = (FOURBS_A5_07_02_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
      logDebug("4BS", "02");
      // .......PIR Status.........................................
      KoENO_GO_BASE__1.value(fourBsTlg_p->u84BsTelData.PIR, Dpt(1, 1));
      logDebug("4BS", "PIR: %u", fourBsTlg_p->u84BsTelData.PIR);
      // ...................  Supply Voltage ......................
      KoENO_GO_BASE__3.value(fourBsTlg_p->u8SupplyVoltage * 20.0, Dpt(9, 1));
      logDebug("4BS", "Supply Voltage: %.1f", fourBsTlg_p->u8SupplyVoltage / 50.0);
      break; // ENDE A5-07-02

    //**************************************************************
    // ----------------- Profil: A5-07-03 --------------------------
    //**************************************************************
    case A5_07_03:
      fourBsTlg3_p = (FOURBS_A5_07_03_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
      logDebug("4BS", "03");
      // .......PIR Status.........................................
      KoENO_GO_BASE__1.value(fourBsTlg3_p->u84BsTelData.PIR, Dpt(1, 1));
      logDebug("4BS", "PIR:%u", fourBsTlg3_p->u84BsTelData.PIR);
      // ........Illumination.......................................
      lux = (uint16_t)fourBsTlg3_p->u8LuxMSB << 2 | fourBsTlg3_p->u84BsTelData2.LUX;
      luxfloat = (float)lux * 1.0;
      KoENO_GO_BASE__2.value(luxfloat, Dpt(9, 1));
      logDebug("4BS", "LUX: %u", lux);
      // ...................  Supply Voltage .......................
      KoENO_GO_BASE__3.value(fourBsTlg3_p->u8SupplyVoltage * 20.0, Dpt(9, 1));
      logDebug("4BS", "Supply Voltage: %.1f", fourBsTlg3_p->u8SupplyVoltage / 50.0);
      break; // ENDE A5-07-03

    default:
      logDebug("4BS", "ERROR");
      break;
    }
    break; // ENDE A5-07-XX
  case A5_08:
    logDebug("4BS", "A5-08-");
    // switch (knx.paramWord(firstParameter + ENO_CHProfil4BS08))
    switch (ParamENO_CHProfil4BS08) {
    //**************************************************************
    // ----------------- Profil: A5-08-01 --------------------------
    //**************************************************************
    case A5_08_01:
      handle4BS_A5_08_01(f_Pkt_st, _channelIndex);
      break; // ENDE A5-08-01
    //**************************************************************
    // ----------------- Profil: A5-08-02 --------------------------
    //**************************************************************
    case A5_08_02:
      fourBsA5_08_Tlg_p = (FOURBS_A5_08_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
      logDebug("4BS", "02");
      // ...................  Supply Voltage .......................
      KoENO_GO_BASE__3.value(fourBsA5_08_Tlg_p->u8SupplyVoltage * 20.0, Dpt(9, 1));
      logDebug("4BS", "Supply Voltage: %.1f", fourBsA5_08_Tlg_p->u8SupplyVoltage / 50.0);
      // ........Illumination..............................................
      lux = (uint16_t)fourBsA5_08_Tlg_p->Ill * 4;
      luxfloat = (float)lux * 1.0;
      KoENO_GO_BASE__2.value(luxfloat, Dpt(9, 1));
      logDebug("4BS", "LUX: %u", lux);
      // ........Temperature..............................................
      KoENO_GO_BASE__7.value(fourBsA5_08_Tlg_p->TMP / 5.0, Dpt(9, 1));
      logDebug("4BS", "Temp: %.1f", fourBsA5_08_Tlg_p->TMP / 5.0);
      // .......PIR Status.........................................
      KoENO_GO_BASE__1.value(fourBsA5_08_Tlg_p->u84BsTelData.PIR, Dpt(1, 1));
      logDebug("4BS", "PIR:%u", fourBsA5_08_Tlg_p->u84BsTelData.PIR);
      // .......Occupancy Button .........................................
      KoENO_GO_BASE__6.value(fourBsA5_08_Tlg_p->u84BsTelData.OCC, Dpt(1, 1));
      logDebug("4BS", "OCC:%u", fourBsA5_08_Tlg_p->u84BsTelData.OCC);

      break; // ENDE A5-08-02

    //**************************************************************
    // ----------------- Profil: A5-08-03 --------------------------
    //**************************************************************
    case A5_08_03:
      fourBsA5_08_Tlg_p = (FOURBS_A5_08_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
      logDebug("4BS", "03");
      // ...................  Supply Voltage .......................
      KoENO_GO_BASE__3.value(fourBsA5_08_Tlg_p->u8SupplyVoltage * 20.0, Dpt(9, 1));
      logDebug("4BS", "Supply Voltage: %.1f", fourBsA5_08_Tlg_p->u8SupplyVoltage / 50.0);
      // ........Illumination..............................................
      lux = (uint16_t)fourBsA5_08_Tlg_p->Ill * 6;
      luxfloat = (float)lux * 1.0;
      KoENO_GO_BASE__2.value(luxfloat, Dpt(9, 1));
      logDebug("4BS", "LUX: %u", lux);
      // ........Temperature..............................................
      KoENO_GO_BASE__7.value((fourBsA5_08_Tlg_p->TMP / 3.1875) - 30, Dpt(9, 1));
      logDebug("4BS", "Temp: %.1f", (fourBsA5_08_Tlg_p->TMP / 3.1875) - 30);
      // .......PIR Status.........................................
      KoENO_GO_BASE__1.value(fourBsA5_08_Tlg_p->u84BsTelData.PIR, Dpt(1, 1));
      logDebug("4BS", "PIR:%u", fourBsA5_08_Tlg_p->u84BsTelData.PIR);
      // .......Occupancy Button .........................................
      KoENO_GO_BASE__6.value(fourBsA5_08_Tlg_p->u84BsTelData.OCC, Dpt(1, 1));
      logDebug("4BS", "OCC:%u", fourBsA5_08_Tlg_p->u84BsTelData.OCC);

      break; // ENDE A5-08-03
    }
    break; // ENDE A5-08-XX
  case A5_09:
    logDebug("4BS", "A5-09-");
    // switch (knx.paramWord(firstParameter + ENO_CHProfil4BS12))
    switch (ParamENO_CHProfil4BS09) {
    //**************************************************************
    // ----------------- Profil: A5-09-04 --------------------------
    //**************************************************************
    case A5_09_04:
    handle4BS_A5_09_04(f_Pkt_st, _channelIndex);
    break;
    case A5_09_05:
    handle4BS_A5_09_05(f_Pkt_st, _channelIndex);
    break;
    case A5_09_0C:
    handle4BS_A5_09_0C(f_Pkt_st, _channelIndex);
    break;
    }//ENDE Switch A5_09
    break; 
  case A5_12:
    logDebug("4BS", "A5-12-");
    // switch (knx.paramWord(firstParameter + ENO_CHProfil4BS12))
    switch (ParamENO_CHProfil4BS12) {
    //**************************************************************
    // ----------------- Profil: A5-12-01 --------------------------
    //**************************************************************
    case A5_12_01:
      fourBsA5_12_01_Tlg_p = (FOURBS_A5_12_01_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
      logDebug("4BS", "01");

      value_4Byte = ((uint32_t)fourBsA5_12_01_Tlg_p->Meterreading_MSB) << 16 |
                    fourBsA5_12_01_Tlg_p->Meterreading_MID << 8 | fourBsA5_12_01_Tlg_p->Meterreading_LSB;

      switch (fourBsA5_12_01_Tlg_p->u84BsTelData.DIV) {
      case 0:
        value_4Byte = value_4Byte / 1;
        break;
      case 1:
        value_4Byte = value_4Byte / 10;
        break;
      case 2:
        value_4Byte = value_4Byte / 100;
        break;
      case 3:
        value_4Byte = value_4Byte / 1000;
        break;

      default:
        break;
      }
      // check unit
      if (fourBsA5_12_01_Tlg_p->u84BsTelData.DT == 1) // WATT
      {
        KoENO_GO_BASE__8.value(value_4Byte, Dpt(14, 1)); // WATT
      } else                                             // KWh
      {
        KoENO_GO_BASE__7.value(value_4Byte, Dpt(13, 1)); // KWH
      }

      // Tarif
      KoENO_GO_BASE__10.value(fourBsA5_12_01_Tlg_p->u84BsTelData.TI, Dpt(5, 1));
      logDebug("4BS", "Energy: %u DT: %u DIV: %u Tarif: %u %u %u %u", value_4Byte,
               fourBsA5_12_01_Tlg_p->u84BsTelData.DT, fourBsA5_12_01_Tlg_p->u84BsTelData.DIV,
               fourBsA5_12_01_Tlg_p->u84BsTelData.TI, fourBsA5_12_01_Tlg_p->Meterreading_MSB,
               fourBsA5_12_01_Tlg_p->Meterreading_MID, fourBsA5_12_01_Tlg_p->Meterreading_LSB);
      break;
    }
    break; // ENDE A5-12-XX

  case A5_14:
    logDebug("4BS", "A5-14-");
    // switch (knx.paramWord(firstParameter + ENO_CHProfil4BS14))
    switch (ParamENO_CHProfil4BS14) {
    //**************************************************************
    // ----------------- Profil: A5-14-01 --------------------------
    //**************************************************************
    case A5_14_01:
      fourBsA5_14_01_06_Tlg_p = (FOURBS_A5_14_01_06_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
      logDebug("4BS", "01");
      // ...................  Supply Voltage .......................
      KoENO_GO_BASE__5.value(fourBsA5_14_01_06_Tlg_p->u8SupplyVoltage * 20.0, Dpt(9, 1));
      logDebug("4BS", "Supply Voltage: %.1f", fourBsA5_14_01_06_Tlg_p->u8SupplyVoltage / 50.0);
      // ........Contact..............................................
      if (fourBsA5_14_01_06_Tlg_p->u84BsTelData.CT == 0) // geschlossen
      {
        if (((ParamENO_CHWindowcloseValue) >> ENO_CHWindowcloseValueShift) & 1)
          bvalue = true;
        else
          bvalue = false;
      } else // open
      {
        if (((ParamENO_CHWindowcloseValue) >> ENO_CHWindowcloseValueShift) & 1)
          bvalue = false;
        else
          bvalue = true;
      }
      KoENO_GO_BASE__4.value(bvalue, Dpt(1, 1));
      logDebug("4BS", "detected: State: %u", fourBsA5_14_01_06_Tlg_p->u84BsTelData.CT);
      break;
    //**************************************************************
    // ----------------- Profil: A5-14-02 --------------------------
    //**************************************************************
    case A5_14_02:
      fourBsA5_14_01_06_Tlg_p = (FOURBS_A5_14_01_06_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
      logDebug("4BS", "02");
      // ...................  Supply Voltage .......................
      KoENO_GO_BASE__5.value(fourBsA5_14_01_06_Tlg_p->u8SupplyVoltage * 20.0, Dpt(9, 1));
      logDebug("4BS", "Supply Voltage: %.1f", fourBsA5_14_01_06_Tlg_p->u8SupplyVoltage / 50.0);
      // ........Illumination.......................................
      lux = (uint16_t)fourBsA5_14_01_06_Tlg_p->u8Lux * 4;
      luxfloat = (float)lux * 1.0;
      KoENO_GO_BASE__2.value(luxfloat, Dpt(9, 1));
      logDebug("4BS", "LUX: %u", lux);
      // ........Contact..............................................
      if (fourBsA5_14_01_06_Tlg_p->u84BsTelData.CT == 0) // geschlossen
      {
        if (((ParamENO_CHWindowcloseValue) >> ENO_CHWindowcloseValueShift) & 1)
          bvalue = true;
        else
          bvalue = false;
      } else // open
      {
        if (((ParamENO_CHWindowcloseValue) >> ENO_CHWindowcloseValueShift) & 1)
          bvalue = false;
        else
          bvalue = true;
      }
      KoENO_GO_BASE__4.value(bvalue, Dpt(1, 1));
      logDebug("4BS", "detected: State: %u", fourBsA5_14_01_06_Tlg_p->u84BsTelData.CT);
      break;
    //**************************************************************
    // ----------------- Profil: A5-14-03 --------------------------
    //**************************************************************
    case A5_14_03:
      fourBsA5_14_01_06_Tlg_p = (FOURBS_A5_14_01_06_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
      logDebug("4BS", "03");
      // ...................  Supply Voltage .......................
      KoENO_GO_BASE__5.value(fourBsA5_14_01_06_Tlg_p->u8SupplyVoltage * 20.0, Dpt(9, 1));
      logDebug("4BS", "Supply Voltage: %.1f", fourBsA5_14_01_06_Tlg_p->u8SupplyVoltage / 50.0);
      // ........Contact..............................................
      if (fourBsA5_14_01_06_Tlg_p->u84BsTelData.CT == 0) // geschlossen
      {
        if (((ParamENO_CHWindowcloseValue) >> ENO_CHWindowcloseValueShift) & 1)
          bvalue = true;
        else
          bvalue = false;
      } else // open
      {
        if (((ParamENO_CHWindowcloseValue) >> ENO_CHWindowcloseValueShift) & 1)
          bvalue = false;
        else
          bvalue = true;
      }
      KoENO_GO_BASE__4.value(bvalue, Dpt(1, 1));
      logDebug("4BS", "detected: State: %u", fourBsA5_14_01_06_Tlg_p->u84BsTelData.CT);
      // ........Vibration Det.........................................
      KoENO_GO_BASE__1.value(fourBsA5_14_01_06_Tlg_p->u84BsTelData.VIB, Dpt(1, 1));
      logDebug("4BS", "Vibration det: %u", fourBsA5_14_01_06_Tlg_p->u84BsTelData.VIB);
      break;
    //**************************************************************
    // ----------------- Profil: A5-14-04 --------------------------
    //**************************************************************
    case A5_14_04:
      fourBsA5_14_01_06_Tlg_p = (FOURBS_A5_14_01_06_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
      logDebug("4BS", "04");
      // ...................  Supply Voltage .......................
      KoENO_GO_BASE__5.value(fourBsA5_14_01_06_Tlg_p->u8SupplyVoltage * 20.0, Dpt(9, 1));
      logDebug("4BS", "Supply Voltage: %.1f", fourBsA5_14_01_06_Tlg_p->u8SupplyVoltage / 50.0);
      // ........Illumination.......................................
      lux = (uint16_t)fourBsA5_14_01_06_Tlg_p->u8Lux * 4;
      luxfloat = (float)lux * 1.0;
      KoENO_GO_BASE__2.value(luxfloat, Dpt(9, 1));
      logDebug("4BS", "LUX: %u", lux);
      // ........Contact..............................................
      if (fourBsA5_14_01_06_Tlg_p->u84BsTelData.CT == 0) // geschlossen
      {
        if (((ParamENO_CHWindowcloseValue) >> ENO_CHWindowcloseValueShift) & 1)
          bvalue = true;
        else
          bvalue = false;
      } else // open
      {
        if (((ParamENO_CHWindowcloseValue) >> ENO_CHWindowcloseValueShift) & 1)
          bvalue = false;
        else
          bvalue = true;
      }
      KoENO_GO_BASE__4.value(bvalue, Dpt(1, 1));
      logDebug("4BS", "detected: State: %u", fourBsA5_14_01_06_Tlg_p->u84BsTelData.CT);
      // ........Vibration Det.........................................
      KoENO_GO_BASE__1.value(fourBsA5_14_01_06_Tlg_p->u84BsTelData.VIB, Dpt(1, 1));
      logDebug("4BS", "Vibration det: %u", fourBsA5_14_01_06_Tlg_p->u84BsTelData.VIB);
      break;
    //**************************************************************
    // ----------------- Profil: A5-14-05 --------------------------
    //**************************************************************
    case A5_14_05:
      fourBsA5_14_01_06_Tlg_p = (FOURBS_A5_14_01_06_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
      logDebug("4BS", "05");
      // ...................  Supply Voltage .......................
      KoENO_GO_BASE__5.value(fourBsA5_14_01_06_Tlg_p->u8SupplyVoltage * 20.0, Dpt(9, 1));
      logDebug("4BS", "Supply Voltage: %.1f", fourBsA5_14_01_06_Tlg_p->u8SupplyVoltage / 50.0);
      // ........Vibration Det.........................................
      KoENO_GO_BASE__1.value(fourBsA5_14_01_06_Tlg_p->u84BsTelData.VIB, Dpt(1, 1));
      logDebug("4BS", "Vibration det: %u", fourBsA5_14_01_06_Tlg_p->u84BsTelData.VIB);
      break;
    //**************************************************************
    // ----------------- Profil: A5-14-06 --------------------------
    //**************************************************************
    case A5_14_06:
      fourBsA5_14_01_06_Tlg_p = (FOURBS_A5_14_01_06_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
      logDebug("4BS", "06");
      // ...................  Supply Voltage .......................
      KoENO_GO_BASE__5.value(fourBsA5_14_01_06_Tlg_p->u8SupplyVoltage * 20.0, Dpt(9, 1));
      logDebug("4BS", "Supply Voltage: %.1f", fourBsA5_14_01_06_Tlg_p->u8SupplyVoltage / 50.0);
      // ........Illumination.......................................
      lux = (uint16_t)fourBsA5_14_01_06_Tlg_p->u8Lux * 4;
      luxfloat = (float)lux * 1.0;
      KoENO_GO_BASE__2.value(luxfloat, Dpt(9, 1));
      logDebug("4BS", "LUX: %u", lux);
      // ........Vibration Det.........................................
      KoENO_GO_BASE__1.value(fourBsA5_14_01_06_Tlg_p->u84BsTelData.VIB, Dpt(1, 1));
      logDebug("4BS", "Vibration det: %u", fourBsA5_14_01_06_Tlg_p->u84BsTelData.VIB);
      break;

    //**************************************************************
    // ----------------- Profil: A5-14-07 --------------------------
    //**************************************************************
    case A5_14_07:
      fourBsA5_14_07_08_Tlg_p = (FOURBS_A5_14_07_08_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
      logDebug("4BS", "07");
      // ...................  Supply Voltage .......................
      KoENO_GO_BASE__5.value(fourBsA5_14_07_08_Tlg_p->u8SupplyVoltage * 20.0, Dpt(9, 1));
      logDebug("4BS", "Supply Voltage: %.1f", fourBsA5_14_07_08_Tlg_p->u8SupplyVoltage / 50.0);
      // ........DOOR Contact..............................................
      KoENO_GO_BASE__2.value(fourBsA5_14_07_08_Tlg_p->u84BsTelData.DCT, Dpt(1, 1));
      logDebug("4BS", "Door State: %u", fourBsA5_14_07_08_Tlg_p->u84BsTelData.DCT);
      // ........Lock Contact.........................................
      KoENO_GO_BASE__4.value(fourBsA5_14_07_08_Tlg_p->u84BsTelData.LCT, Dpt(1, 1));
      logDebug("4BS", "Lock State: %u", fourBsA5_14_07_08_Tlg_p->u84BsTelData.LCT);
      break;
    //**************************************************************
    // ----------------- Profil: A5-14-08 --------------------------
    //**************************************************************
    case A5_14_08:
      fourBsA5_14_07_08_Tlg_p = (FOURBS_A5_14_07_08_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
      logDebug("4BS", "08");
      // ...................  Supply Voltage .......................
      KoENO_GO_BASE__5.value(fourBsA5_14_07_08_Tlg_p->u8SupplyVoltage * 20.0, Dpt(9, 1));
      logDebug("4BS", "Supply Voltage: %.1f", fourBsA5_14_07_08_Tlg_p->u8SupplyVoltage / 50.0);
      // ........Vibration Det.........................................
      KoENO_GO_BASE__1.value(fourBsA5_14_07_08_Tlg_p->u84BsTelData.VIB, Dpt(1, 1));
      logDebug("4BS", "Vibration det: %u", fourBsA5_14_07_08_Tlg_p->u84BsTelData.VIB);
      // ........DOOR Contact..............................................
      KoENO_GO_BASE__2.value(fourBsA5_14_07_08_Tlg_p->u84BsTelData.DCT, Dpt(1, 1));
      logDebug("4BS", "Door State: %u", fourBsA5_14_07_08_Tlg_p->u84BsTelData.DCT);
      // ........Lock Contact.........................................
      KoENO_GO_BASE__4.value(fourBsA5_14_07_08_Tlg_p->u84BsTelData.LCT, Dpt(1, 1));
      logDebug("4BS", "Lock State: %u", fourBsA5_14_07_08_Tlg_p->u84BsTelData.LCT);
      break;
    //**************************************************************
    // ----------------- Profil: A5-14-09 --------------------------
    //**************************************************************
    case A5_14_09:
      handle4BS_A5_14_09(f_Pkt_st, _channelIndex);
      break;

      //**************************************************************
      // ----------------- Profil: A5-14-0A --------------------------
      //**************************************************************
    case A5_14_0A:
      fourBsA5_14_09_0A_Tlg_p = (FOURBS_A5_14_09_0A_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
      logDebug("4BS", "0A");
      // ...................  Supply Voltage .......................
      KoENO_GO_BASE__5.value(fourBsA5_14_09_0A_Tlg_p->u8SupplyVoltage * 20.0, Dpt(9, 1));
      logDebug("4BS", "Supply Voltage: %.1f", fourBsA5_14_09_0A_Tlg_p->u8SupplyVoltage / 50.0);
      // ...................  Fenster Status .......................
      switch (fourBsA5_14_09_0A_Tlg_p->u84BsTelData.CT) {
      case 0x00: // close
        KoENO_GO_BASE__2.value(false, Dpt(1, 1));
        KoENO_GO_BASE__3.value(false, Dpt(1, 1));
        if (ParamENO_CHWindowcloseValue)
          bvalue = true;
        else
          bvalue = false;
        KoENO_GO_BASE__4.value(bvalue, Dpt(1, 1));

        logDebug("4BS", "State: close");
        break;
      case 0x01: // tilt
        KoENO_GO_BASE__2.value(false, Dpt(1, 1));
        KoENO_GO_BASE__3.value(true, Dpt(1, 1));
        if (ParamENO_CHWindowcloseValue)
          bvalue = false;
        else
          bvalue = true;
        KoENO_GO_BASE__4.value(bvalue, Dpt(1, 1));

        logDebug("4BS", "State: gekippt");
        break;
      case 0x03: // open
        KoENO_GO_BASE__2.value(true, Dpt(1, 1));
        KoENO_GO_BASE__3.value(false, Dpt(1, 1));
        if (ParamENO_CHWindowcloseValue)
          bvalue = false;
        else
          bvalue = true;
        KoENO_GO_BASE__4.value(bvalue, Dpt(1, 1));
        logDebug("4BS", "State: open");
        break;

      default:
        break;
      }
      // ...................  Vibration detected .......................
      KoENO_GO_BASE__1.value(fourBsA5_14_09_0A_Tlg_p->u84BsTelData.VIB, Dpt(1, 1));
      logDebug("4BS", "Vibr: %u", fourBsA5_14_09_0A_Tlg_p->u84BsTelData.VIB);

      break; // A5_14_0A

    default:
      logDebug("4BS", "ERROR");
      break;
    }
    break; // ENDE A5-14-XX
  case A5_20:
    // Heizkörper-Stellantriebe (bidirektional): Kanäle mit diesen Profilen werden in EnoceanChannel::handleHvac()
    // verarbeitet, das auch die Antwort an den Antrieb sendet
    switch (ParamENO_CHProfil4BS20) {
    case A5_20_01:
      return handle4BS_A5_20_01(f_Pkt_st, _channelIndex);
    case A5_20_04:
      return handle4BS_A5_20_04(f_Pkt_st, _channelIndex);
    case A5_20_06:
      return handle4BS_A5_20_06(f_Pkt_st, _channelIndex);
    default:
      logDebug("4BS", "A5-20: profile %u not supported", ParamENO_CHProfil4BS20);
      return 0;
    }

  case A5_special:
    // switch (knx.paramWord(firstParameter + ENO_CHProfil4BSAdd))
    switch (ParamENO_CHProfil4BSAdd) {
    case D5_00_01_V2:
      handle4BS_D5_00_01_V2(f_Pkt_st, _channelIndex);
      break;

    default:
      logDebug("4BS", "ERROR");
      break;
    }

    break; // ENDE A5-Special

  //**************************************************************
  default:
    logDebug("4BS", "Profil: ERROR");
    break;
  }

  return 0;
}

/*************************************************************************************
 *
 *  Inline funktions
 *
 ***************************************************************************************/

// Spannungstelegramm (4BS) der Eltako-Fensterkontakte FFKB/FTK/FTKB, die ihren Kontaktzustand per 1BS (D5-00-01)
// senden: DB3 = Energiespeicher, DB2 = Batteriespannung, jeweils 0..255 = 0..5 V.
// KO_4 (GO_BASE__5) = Batteriespannung, KO_7 (GO_BASE__8) = Energiespeicher, beide in mV (DPT 9.020).
inline void handle4BS_D5_00_01_V2(PACKET_SERIAL_TYPE_ *f_Pkt_st, uint8_t _channelIndex) {
  ONEBS_TELEGRAM_TYPE_V2 *l1bsV2Tlg_p = (ONEBS_TELEGRAM_TYPE_V2 *)&(f_Pkt_st->u8DataBuffer[1]);

  logDebug("4BS", "D5-00-01 V2");
  KoENO_GO_BASE__8.value(l1bsV2Tlg_p->u8EnergyVoltage * 19.608, Dpt(9, 20));
  KoENO_GO_BASE__5.value(l1bsV2Tlg_p->u8BatteryVoltage * 19.608, Dpt(9, 20));

  logDebug("4BS", "Enr: %.1f V Bat: %.1f V", l1bsV2Tlg_p->u8EnergyVoltage / 51.0, l1bsV2Tlg_p->u8BatteryVoltage / 51.0);
}

// A5-20-01 Heizkörper-Stellantrieb (bidirektional, z.B. Eltako FKS-SV).
// Antrieb -> Gateway: DB3 Ventilstellung 0..100 %, DB2 Statusbits, DB1 Temperatur 0..255 = 0..40 °C, DB0 Bit 3 LRN.
// Gateway -> Antrieb (Antwort im Empfangsfenster, siehe EnoceanChannel::prepareHvacReply): DB3 Sollwert, DB2 Raumtemperatur (invertiert),
// DB1 Befehle (folgende Bits), DB0 = 0x08.
#define A52001_DB1_RIN 0x80 // Referenzfahrt (Run Init)
#define A52001_DB1_LFS 0x40 // Hub einstellen (Lift Set)
#define A52001_DB1_VO 0x20  // Ventil öffnen (Wartung)
#define A52001_DB1_VC 0x10  // Ventil schließen
#define A52001_DB1_SB 0x08  // Sommerbetrieb
#define A52001_DB1_SPS 0x04 // Sollwert ist eine Temperatur (sonst Ventilstellung)
// Befehle, die nur mit einer Antwort gesendet und danach zurückgesetzt werden
#define A52001_ONESHOT (A52001_DB1_RIN | A52001_DB1_LFS | A52001_DB1_VO | A52001_DB1_VC)

// Wertet ein Telegramm des Antriebs aus: KO_6 Temperatur, KO_7 Ventilstellung, KO_9 Statusbits.
// Liefert TEACHIN_A52001 für ein Lerntelegramm mit EEP (verlangt eine Teach-In-Antwort), msg_A5_20_01 für ein
// Datentelegramm (verlangt eine Antwort mit dem Sollwert), sonst 0.
inline uint8_t handle4BS_A5_20_01(PACKET_SERIAL_TYPE_ *f_Pkt_st, uint8_t _channelIndex) {
  const uint8_t *d = f_Pkt_st->u8DataBuffer; // d[1] = DB3 ... d[4] = DB0

  if (!(d[4] & 0x08)) {
    // Lerntelegramm: nur Variante 3 (LRN-Typ = 1, mit EEP) wird beantwortet
    if (d[4] & 0x80) {
      logDebug("4BS", "A5-20-01 teach-in request");
      return TEACHIN_A52001;
    }
    logDebug("4BS", "A5-20-01 teach-in without EEP ignored");
    return 0;
  }

  uint8_t pos = d[1] > 100 ? 100 : d[1];
  float temp = d[3] * 40.0f / 255.0f;
  KoENO_GO_BASE__8.value(pos, Dpt(5, 1));
  KoENO_GO_BASE__10.value(d[2], Dpt(5, 10));
  KoENO_GO_BASE__7.value(temp, Dpt(9, 1));
  logDebug("4BS", "A5-20-01 Pos: %u%% Temp: %.1f Status: %02X", pos, temp, d[2]);
  return msg_A5_20_01;
}

// A5-20-04 Heizkörper-Stellantrieb mit Vorlauf-/Raumtemperatur, Display und Tasten (bidirektional, z.B. Holter).
// Antrieb -> Gateway: DB3 Ventilstellung 0..100 %, DB2 Vorlauftemperatur 0..255 = 20..80 °C oder am Gerät
// eingestellte Solltemperatur 0..255 = 10..30 °C (DB0.1 TS), DB1 Raumtemperatur 0..255 = 10..30 °C oder Fehlercode
// (DB0.0 FL), DB0: MST Bit 7, STR Bit 6, LRN Bit 3, BLS Bit 2, TS Bit 1, FL Bit 0.
// Gateway -> Antrieb (adressiert): DB3 Ventilstellung, DB2 Solltemperatur für das Display 0..255 = 10..30 °C,
// DB1 MC Bit 6 + Aufwachzyklus Bit 5..0, DB0 Displaydrehung Bit 5..4, LRN Bit 3, Tastensperre Bit 2, Service Bit 1..0.
#define A52004_DB0_BLC 0x04 // Tasten am Gerät sperren
#define A52004_DB0_SER 0x03 // Service: 1 = Ventil öffnen, 2 = Initialisierung, 3 = Ventil schließen

// KO_5 Fehlercode, KO_6 Vorlauf- bzw. am Gerät eingestellte Solltemperatur, KO_7 Ventilstellung, KO_8 Raumtemperatur,
// KO_9 Statusbits (DB0). Rückgabe wie handle4BS_A5_20_01.
inline uint8_t handle4BS_A5_20_04(PACKET_SERIAL_TYPE_ *f_Pkt_st, uint8_t _channelIndex) {
  const uint8_t *d = f_Pkt_st->u8DataBuffer; // d[1] = DB3 ... d[4] = DB0

  if (!(d[4] & 0x08)) {
    if (d[4] & 0x80) {
      logDebug("4BS", "A5-20-04 teach-in request");
      return TEACHIN_A52004;
    }
    logDebug("4BS", "A5-20-04 teach-in without EEP ignored");
    return 0;
  }

  uint8_t pos = d[1] > 100 ? 100 : d[1];
  KoENO_GO_BASE__8.value(pos, Dpt(5, 1));

  float temp;
  if (d[4] & 0x02) {
    temp = 10.0f + d[2] * 20.0f / 255.0f; // am Gerät eingestellte Solltemperatur
    logDebug("4BS", "A5-20-04 local setpoint: %.1f", temp);
  } else {
    temp = 20.0f + d[2] * 60.0f / 255.0f; // Vorlauftemperatur
    logDebug("4BS", "A5-20-04 feed temperature: %.1f", temp);
  }
  KoENO_GO_BASE__7.value(temp, Dpt(9, 1));

  if (d[4] & 0x01) {
    KoENO_GO_BASE__6.value(d[3], Dpt(5, 10));
    logDebug("4BS", "A5-20-04 failure code: %u", d[3]);
  } else {
    float room = 10.0f + d[3] * 20.0f / 255.0f;
    KoENO_GO_BASE__9.value(room, Dpt(9, 1));
    logDebug("4BS", "A5-20-04 room temperature: %.1f", room);
  }

  KoENO_GO_BASE__10.value(d[4], Dpt(5, 10));
  logDebug("4BS", "A5-20-04 Pos: %u%% Status: %02X", pos, d[4]);
  return msg_A5_20_04;
}

// A5-20-06 Energieautarker Heizkörper-Stellantrieb mit lokaler Verstellung (bidirektional, z.B. Micropelt MVA-005).
// Antrieb -> Gateway: DB3 Ventilstellung 0..100 %, DB2 Bit 7 LOM / Bit 6..0 LO (LOM = 1: Solltemperatur inkl. Verstellung
// 0..80 = 0..40 °C, LOM = 0: Verstellung -5..+5 K als 7-Bit-Zweierkomplement), DB1 Temperatur 0,5 °C/Bit (Umgebung
// oder Vorlauf je nach DB0.7 TSL, 0xFF = Fühlerfehler), DB0: TSL, ENIE, ES, DWO, LRN, RCE, RSS, ACO (Bit 7..0).
// Gateway -> Antrieb: DB3 Sollwert (0..100 % oder 0..80 = 0..40 °C), DB2 Raumtemperatur 0..160 = 0..40 °C (0 = interner
// Fühler), DB1 REF Bit 7, RFC Bit 6..4, SB Bit 3, SPS Bit 2, TSL Bit 1, SBY Bit 0, DB0 = 0x08.
#define A52006_DB1_REF 0x80 // Referenzfahrt
#define A52006_DB1_SB 0x08  // Sommerbetrieb
#define A52006_DB1_SPS 0x04 // Sollwert ist eine Temperatur
#define A52006_DB1_TSL 0x02 // Vorlauftemperatur statt Umgebungstemperatur melden
#define A52006_DB1_SBY 0x01 // Standby

// KO_6 Temperatur, KO_7 Ventilstellung, KO_8 lokale Verstellung (LOM 1: Solltemperatur in °C, LOM 0: Offset in K),
// KO_9 Statusbits (DB0). Rückgabe wie handle4BS_A5_20_01.
inline uint8_t handle4BS_A5_20_06(PACKET_SERIAL_TYPE_ *f_Pkt_st, uint8_t _channelIndex) {
  const uint8_t *d = f_Pkt_st->u8DataBuffer; // d[1] = DB3 ... d[4] = DB0

  if (!(d[4] & 0x08)) {
    if (d[4] & 0x80) {
      logDebug("4BS", "A5-20-06 teach-in request");
      return TEACHIN_A52006;
    }
    logDebug("4BS", "A5-20-06 teach-in without EEP ignored");
    return 0;
  }

  uint8_t pos = d[1] > 100 ? 100 : d[1];
  KoENO_GO_BASE__8.value(pos, Dpt(5, 1));

  if (d[3] != 0xFF)
    KoENO_GO_BASE__7.value(d[3] / 2.0f, Dpt(9, 1));
  else
    logInfo("4BS", "A5-20-06 temperature sensor failure");

  uint8_t lo = d[2] & 0x7F;
  float local;
  if (d[2] & 0x80) {
    local = lo / 2.0f; // Solltemperatur inkl. lokaler Verstellung
  } else {
    local = (lo & 0x40) ? (int)lo - 128 : lo; // Offset in K
  }
  KoENO_GO_BASE__9.value(local, Dpt(9, 1));

  KoENO_GO_BASE__10.value(d[4], Dpt(5, 10));
  logDebug("4BS", "A5-20-06 Pos: %u%% Temp: %.1f %s: %.1f Status: %02X", pos, d[3] / 2.0f,
           (d[2] & 0x80) ? "Setpoint" : "Offset", local, d[4]);
  return msg_A5_20_06;
}

inline void handle4BS_A5_04_02(PACKET_SERIAL_TYPE_ *f_Pkt_st, uint8_t _channelIndex) {
  FOURBS_A5_04_TYPE *fourBsA5_04_Tlg_p;
  float temp;
  float hum;
  fourBsA5_04_Tlg_p = (FOURBS_A5_04_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
  temp = (float)(fourBsA5_04_Tlg_p->Temp / 3.125) - 20.0;
  hum = (float)(fourBsA5_04_Tlg_p->Hum / 2.5);
  KoENO_GO_BASE__2.value(temp, Dpt(9, 1));
  KoENO_GO_BASE__3.value(hum, Dpt(9, 1));
  logDebug("4BS", "02 Temp: %.1f Hum: %.1f", temp, hum);
}

inline void handle4BS_A5_04_03(PACKET_SERIAL_TYPE_ *f_Pkt_st, uint8_t _channelIndex) {
  FOURBS_A5_04_03_TYPE *fourBsA5_04_03_Tlg_p;
  float temp;
  float hum;
  fourBsA5_04_03_Tlg_p = (FOURBS_A5_04_03_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
  temp = (float)(((uint16_t)fourBsA5_04_03_Tlg_p->TempMSB << 8 | fourBsA5_04_03_Tlg_p->TempLSB) / 12.7875) - 20.0;
  hum = (float)(fourBsA5_04_03_Tlg_p->Hum / 2.55);
  KoENO_GO_BASE__2.value(temp, Dpt(9, 1));
  KoENO_GO_BASE__3.value(hum, Dpt(9, 1));
  logDebug("4BS", "03 Temp: %.1f Hum: %.1f", temp, hum);
}

inline void handle4BS_A5_06_01_V2(PACKET_SERIAL_TYPE_ *f_Pkt_st, uint8_t _channelIndex) {
  float luxfloat;
  FOURBS_A5_06_01_V2_TYPE *fourBsA5_06_01_V2_Tlg_p;

  logDebug("4BS", "01*");
  fourBsA5_06_01_V2_Tlg_p = (FOURBS_A5_06_01_V2_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);

  if (fourBsA5_06_01_V2_Tlg_p->u8byte0 == 0x09) {
    if (fourBsA5_06_01_V2_Tlg_p->u8Illumbyte2 == 0) // Helligkeit 0..100LUX
    {
      // Range 0 ... 50 lux
      luxfloat = (float)(fourBsA5_06_01_V2_Tlg_p->u8Illumbyte3);
    } else {
      // Range 300 ... 30000 lux
      luxfloat = (float)(fourBsA5_06_01_V2_Tlg_p->u8Illumbyte2 * 116.470588) + 300.0;
    }
  } else {
    luxfloat = 0;
  }
  KoENO_GO_BASE__2.value(luxfloat, Dpt(9, 1));

  logDebug("4BS", "LUX: %.1f", luxfloat);
}


inline void handle4BS_A5_07_01(PACKET_SERIAL_TYPE_ *f_Pkt_st, uint8_t _channelIndex){
      FOURBS_A5_07_01_TYPE *fourBsTlg2_p;
      fourBsTlg2_p = (FOURBS_A5_07_01_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
      // .......PIR Status.........................................
      if (fourBsTlg2_p->PIR < 128)
        KoENO_GO_BASE__1.value(false, Dpt(1, 1));
      else
        KoENO_GO_BASE__1.value(true, Dpt(1, 1));
      logDebug("4BS", "01 PIR: %u", fourBsTlg2_p->PIR);
}


inline void  handle4BS_A5_08_01(PACKET_SERIAL_TYPE_ *f_Pkt_st, uint8_t _channelIndex){
      FOURBS_A5_08_TYPE *fourBsA5_08_Tlg_p;
      uint16_t lux;
      float luxfloat;
      fourBsA5_08_Tlg_p = (FOURBS_A5_08_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
      logDebug("4BS", "01");
      // ...................  Supply Voltage .......................
      KoENO_GO_BASE__3.value(fourBsA5_08_Tlg_p->u8SupplyVoltage * 20.0, Dpt(9, 1));
      logDebug("4BS", "Supply Voltage: %.1f", fourBsA5_08_Tlg_p->u8SupplyVoltage / 50.0);
      // ........Illumination..............................................
      lux = (uint16_t)fourBsA5_08_Tlg_p->Ill * 2;
      luxfloat = (float)lux * 1.0;
      KoENO_GO_BASE__2.value(luxfloat, Dpt(9, 1));
      logDebug("4BS", "LUX: %u", lux);
      // ........Temperature..............................................
      KoENO_GO_BASE__7.value(fourBsA5_08_Tlg_p->TMP / 5.0, Dpt(9, 1));
      logDebug("4BS", "Temp: %.1f", fourBsA5_08_Tlg_p->TMP / 5.0);
      // .......PIR Status.........................................
      KoENO_GO_BASE__1.value(fourBsA5_08_Tlg_p->u84BsTelData.PIR, Dpt(1, 1));
      logDebug("4BS", "PIR:%u", fourBsA5_08_Tlg_p->u84BsTelData.PIR);
      // .......Occupancy Button .........................................
      KoENO_GO_BASE__6.value(fourBsA5_08_Tlg_p->u84BsTelData.OCC, Dpt(1, 1));
      logDebug("4BS", "OCC:%u", fourBsA5_08_Tlg_p->u84BsTelData.OCC);
}

inline void handle4BS_A5_09_04(PACKET_SERIAL_TYPE_ *f_Pkt_st, uint8_t _channelIndex) {
  FOURBS_A5_09_04_TYPE *fourBsA5_09_04_Tlg_p;
  float temp, hum;
  uint16_t CO2;
  fourBsA5_09_04_Tlg_p = (FOURBS_A5_09_04_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
  temp = (float)(fourBsA5_09_04_Tlg_p->TMP / 5.0);
  hum = (float)(fourBsA5_09_04_Tlg_p->Hum / 2.0);
  CO2 = fourBsA5_09_04_Tlg_p->CO2 * 10;
  KoENO_GO_BASE__1.value(CO2, Dpt(9, 8));
  KoENO_GO_BASE__2.value(temp, Dpt(9, 1));
  KoENO_GO_BASE__3.value(hum, Dpt(9, 1));
  logDebug("4BS", "Temp: %.1f Hum: %.1f  CO2: %d", temp, hum, CO2);
}

inline void handle4BS_A5_09_05(PACKET_SERIAL_TYPE_ *f_Pkt_st, uint8_t _channelIndex) {
  FOURBS_A5_09_05_TYPE *fourBsA5_09_05_Tlg_p;
  uint16_t voc;
  uint8_t voc_id;
  uint8_t scale;
  fourBsA5_09_05_Tlg_p = (FOURBS_A5_09_05_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
  scale = fourBsA5_09_05_Tlg_p->u84BsTelData.SCALE;
  voc = (uint16_t)(fourBsA5_09_05_Tlg_p->VOC_MSB << 8) | fourBsA5_09_05_Tlg_p->VOC_LSB; // ppb
  // Umrechnung auf ppm
  voc = voc / 1000;

  voc_id = fourBsA5_09_05_Tlg_p->VOC_ID;

  switch (scale) {
  case 0: // * 0,01
    voc = voc * 0.01;
    break;
  case 1: // * 0,1
    voc = voc * 0.1;
    break;
  case 2: // * 1.0
    voc = voc * 1.0;
    break;
  case 3: // * 10.0
    voc = voc * 10;
    break;
  }
  KoENO_GO_BASE__1.value(voc, Dpt(9, 8));
  KoENO_GO_BASE__8.value(voc_id, Dpt(5, 10));
  logDebug("4BS", "VOC: %d  VOC_ID: %d", voc, voc_id);
}

inline void handle4BS_A5_09_0C(PACKET_SERIAL_TYPE_ *f_Pkt_st, uint8_t _channelIndex) {
  FOURBS_A5_09_0C_TYPE *fourBsA5_09_0C_Tlg_p;
  uint16_t voc;
  uint8_t voc_id;
  uint8_t scale;
  bool voc_unit;
  fourBsA5_09_0C_Tlg_p = (FOURBS_A5_09_0C_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
  scale = fourBsA5_09_0C_Tlg_p->u84BsTelData.SCALE;
  voc_unit = fourBsA5_09_0C_Tlg_p->u84BsTelData.UNIT;
  voc_id = fourBsA5_09_0C_Tlg_p->VOC_ID;
  voc = (uint16_t)(fourBsA5_09_0C_Tlg_p->VOC_MSB << 8) | fourBsA5_09_0C_Tlg_p->VOC_LSB; // ppb

  if (voc_unit == 0) // ppb
  {
    // Umrechnung auf ppm
    voc = voc / 1000;
    switch (scale) {
    case 0: // * 0,01
      voc = voc * 0.01;
      break;
    case 1: // * 0,1
      voc = voc * 0.1;
      break;
    case 2: // * 1.0
      voc = voc * 1.0;
      break;
    case 3: // * 10.0
      voc = voc * 10;
      break;
    }
    KoENO_GO_BASE__1.value(voc, Dpt(9, 8));
  } else {
    switch (scale) {
    case 0: // * 0,01
      voc = voc * 0.01;
      break;
    case 1: // * 0,1
      voc = voc * 0.1;
      break;
    case 2: // * 1.0
      voc = voc * 1.0;
      break;
    case 3: // * 10.0
      voc = voc * 10;
      break;
    }
    KoENO_GO_BASE__10.value(voc, Dpt(9, 30));
  }

  KoENO_GO_BASE__8.value(voc_id, Dpt(5, 10));
  logDebug("4BS", "VOC: %d  VOC_ID: %d", voc, voc_id);
}

inline void handle4BS_A5_14_09(PACKET_SERIAL_TYPE_ *f_Pkt_st, uint8_t _channelIndex) {
  FOURBS_A5_14_09_0A_TYPE *fourBsA5_14_09_0A_Tlg_p;
  bool bvalue;
  fourBsA5_14_09_0A_Tlg_p = (FOURBS_A5_14_09_0A_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
  logDebug("4BS", "A5-14-09 ");
  //  ...................  Supply Voltage .......................
  KoENO_GO_BASE__5.value(fourBsA5_14_09_0A_Tlg_p->u8SupplyVoltage * 20.0, Dpt(9, 1));
  logDebug("4BS", "Supply Voltage: %.1f", fourBsA5_14_09_0A_Tlg_p->u8SupplyVoltage / 50.0);

  switch (fourBsA5_14_09_0A_Tlg_p->u84BsTelData.CT) {
  case 0x00: // close
    KoENO_GO_BASE__2.value(false, Dpt(1, 1));
    KoENO_GO_BASE__3.value(false, Dpt(1, 1));
    if (ParamENO_CHWindowcloseValue)
      bvalue = true;
    else
      bvalue = false;
    KoENO_GO_BASE__4.value(bvalue, Dpt(1, 1));

    logDebug("4BS", "State: close");
    break;
  case 0x01: // tilt
    KoENO_GO_BASE__2.value(false, Dpt(1, 1));
    KoENO_GO_BASE__3.value(true, Dpt(1, 1));
    if (ParamENO_CHWindowcloseValue)
      bvalue = false;
    else
      bvalue = true;
    KoENO_GO_BASE__4.value(bvalue, Dpt(1, 1));

    logDebug("4BS", "State: gekippt");
    break;
  case 0x03: // open
    KoENO_GO_BASE__2.value(true, Dpt(1, 1));
    KoENO_GO_BASE__3.value(false, Dpt(1, 1));
    if (ParamENO_CHWindowcloseValue)
      bvalue = false;
    else
      bvalue = true;
    KoENO_GO_BASE__4.value(bvalue, Dpt(1, 1));
    logDebug("4BS", "State: open");
    break;

  default:
    break;
  }
}

inline void handle4BS_A5_30_03(PACKET_SERIAL_TYPE_ *f_Pkt_st, uint8_t _channelIndex) {
  bool bvalue;
  FOURBS_A5_30_03_TYPE *fourBsA5_30_39_Tlg_p;
  fourBsA5_30_39_Tlg_p = (FOURBS_A5_30_03_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
  logDebug("4BS", "A5-30-03 ");
  //******************** Temperatur **************************************************/
  float temp = (float)(fourBsA5_30_39_Tlg_p->u8Temp * 0.156862745);
  KoENO_GO_BASE__2.value(temp, Dpt(9, 1));
  //******************** Alarm *******************************************************/
  if (fourBsA5_30_39_Tlg_p->u8Alarm == 0x0F)
    bvalue = true;
  else if (fourBsA5_30_39_Tlg_p->u8Alarm == 0x1F)
    bvalue = false;
  else
    bvalue = false;
  KoENO_GO_BASE__4.value(bvalue, Dpt(1, 5));

  logDebug("4BS", "Temp: %.1f", temp);
  logDebug("4BS", "Alarm: %.1f", bvalue);
}