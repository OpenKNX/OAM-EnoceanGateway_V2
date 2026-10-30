#pragma once

#include "EnoceanModule.h"
#include "EnoceanProfils.h"
#include "EnoceanStructs.h"
#include "knxprod.h"

inline void handle_VLD(PACKET_SERIAL_TYPE_ *f_Pkt_st, uint8_t _channelIndex)
{

      uint8_t mem;
      uint8_t mem2;
      uint8_t valueBat;
      uint16_t value;
      
      //float temp_s;
      float hum_s;
      float voc1_s;
      

      union float_value
      {
            float temp_s;
            float lux;
      }valuefloat;

      union flaot_value2
      {
            float voc2_s;
            float acc_value;
      }valuefloat2;
      

      union u32t_value
      {
            uint32_t counter;
            uint32_t lux;
      }valueU32;

      VLD_D2_01_TELEGRAM_CMD_04_TYPE *ActStatResp;
      VLD_D2_01_TELEGRAM_CMD_04_TYPE *ActStatResp_4_D2_01_0E;
      VLD_D2_01_TELEGRAM_CMD_07_TYPE *ActStatResp_7_D2_01_0E;
      VLD_D2_04_00_TELEGRAM *SenVal_D2_04;
      VLD_D2_03_0A_TELEGRAM *SenVal_D2_03_0A;
      VLD_D2_05_00_TELEGRAM_CMD_04_TYPE *ActStatResp_D2_05;
      // VLD_D2_14_00_TELEGRAM *SenVal_D2_14;
      VLD_D2_14_30_TELEGRAM *SenStat_D2_14_30;
      VLD_D2_14_41_TELEGRAM *SenStat_D2_14_41;
      VLD_D2_06_01_TELEGRAM *SenVal_D2_06_01;
      VLD_D2_06_50_Window_Status_0x01 *SenVal_D2_06_50_0x01;
      VLD_D2_06_50_Alarm_Status_0x02 *SenVal_D2_06_50_0x02;

      //switch (knx.paramWord(firstParameter + ENO_CHProfilSelectionVLD))
      switch (ParamENO_CHProfilSelectionVLD)
      {
      case D2_01:
            //**************************************************************
            // ----------------- Profil: D2-01-xx --------------------------
            //**************************************************************
            //switch (knx.paramWord(firstParameter + ENO_CHProfilVLD04))
            switch (ParamENO_CHProfilVLD04)
            {
            case D2_01_0E:
                  logDebug("VLD", "Profil: D2-01-0E");
                  switch (f_Pkt_st->u8DataBuffer[1])
                  {
                  case 4: // CMD = 4   -->  Status AKTOR Channels
                        logDebug("VLD", "CMD = 0x04");
                        mem = f_Pkt_st->u8DataBuffer[2];
                        f_Pkt_st->u8DataBuffer[2] = f_Pkt_st->u8DataBuffer[3];
                        f_Pkt_st->u8DataBuffer[3] = mem;

                        ActStatResp_4_D2_01_0E = (VLD_D2_01_TELEGRAM_CMD_04_TYPE *)&(f_Pkt_st->u8DataBuffer[2]);

                        if (ActStatResp_4_D2_01_0E->u8VldTelActResp2.IOChannel == 0) // Abfrage ob CH1
                        {
                              if (ActStatResp_4_D2_01_0E->u8VldTelActResp.outputValue == 0)
                                    KoENO_GO_BASE__5.value(false, Dpt(1, 1));
                              else
                                    KoENO_GO_BASE__5.value(true, Dpt(1, 1));
                              logDebug("VLD", "State: %u", ActStatResp_4_D2_01_0E->u8VldTelActResp.outputValue);
                        }
                        break;
                  case 7:
                        logDebug("VLD", "CMD = 0x07");
                        ActStatResp_7_D2_01_0E = (VLD_D2_01_TELEGRAM_CMD_07_TYPE *)&(f_Pkt_st->u8DataBuffer[2]);
                        uint32_t value = (f_Pkt_st->u8DataBuffer[3] << 24) | f_Pkt_st->u8DataBuffer[4] << 16 | f_Pkt_st->u8DataBuffer[5] << 8 | f_Pkt_st->u8DataBuffer[6];
                        switch (ActStatResp_7_D2_01_0E->u8VldTelActResp.UNIT)
                        {
                        case 0x00: // WS
                              /* code */
                              break;
                        case 0x01: // Wh
                              KoENO_GO_BASE__6.value(value, Dpt(13, 1));
                              break;
                        case 0x02: // KWh
                              KoENO_GO_BASE__7.value(value, Dpt(13, 1));
                              break;
                        case 0x03: // W
                              KoENO_GO_BASE__8.value(value, Dpt(14, 1));
                              break;
                        case 0x04: // KW
                              KoENO_GO_BASE__9.value(value, Dpt(9, 1));
                              break;
                        default:
                              break;
                        }
                        logDebug("VLD", "Unit: %u, Value: %u", ActStatResp_7_D2_01_0E->u8VldTelActResp.UNIT, value);
                        break;
                  } // Ende SWITCH (f_Pkt_st->u8DataBuffer[1])
                  break;
            case D2_01_12:
                  logDebug("VLD", "Profil: D2-01-12");
                  switch (f_Pkt_st->u8DataBuffer[1])
                  {
                  case 4: // CMD = 4   -->  Status AKTOR Channels
// swap two bytes
                        logDebug("VLD", "CMD = 0x04");
                        mem = f_Pkt_st->u8DataBuffer[2];
                        f_Pkt_st->u8DataBuffer[2] = f_Pkt_st->u8DataBuffer[3];
                        f_Pkt_st->u8DataBuffer[3] = mem;

                        ActStatResp = (VLD_D2_01_TELEGRAM_CMD_04_TYPE *)&(f_Pkt_st->u8DataBuffer[2]);

                        if (ActStatResp->u8VldTelActResp2.IOChannel == 0) // Abfrage ob CH1
                        {
                              if (ActStatResp->u8VldTelActResp.outputValue == 0)
                                    KoENO_GO_BASE__3.value(false, Dpt(1, 1));
                              else
                                    KoENO_GO_BASE__3.value(true, Dpt(1, 1));
                              logDebug("VLD", "CH1: %u", ActStatResp->u8VldTelActResp.outputValue);
                        }
                        else if (ActStatResp->u8VldTelActResp2.IOChannel == 1) // Abfrage ob CH2
                        {
                              if (ActStatResp->u8VldTelActResp.outputValue == 0)
                                    KoENO_GO_BASE__4.value(false, Dpt(1, 1));
                              else
                                    KoENO_GO_BASE__4.value(true, Dpt(1, 1));
                              logDebug("VLD", "CH2: %u", ActStatResp->u8VldTelActResp.outputValue);
                        }
                        break;
                  }
                  break; // ENDE CASE D2_01_12
            }
            break;

      case D2_03:
            //**************************************************************
            // ----------------- Profil: D2-03-xx --------------------------
            //**************************************************************
            //switch (knx.paramWord(firstParameter + ENO_CHProfilVLD03))
            switch (ParamENO_CHProfilVLD03)
            {
            case D2_03_0A:
                  SenVal_D2_03_0A = (VLD_D2_03_0A_TELEGRAM *)&(f_Pkt_st->u8DataBuffer[1]);
                  logDebug("VLD", "Profil: D2-03-0A");
                  // Batterie
                  KoENO_GO_BASE__5.value(SenVal_D2_03_0A->bat, Dpt(5, 1));
                  // Buttons
                  switch (SenVal_D2_03_0A->value)
                  {
                  case 1: // single click
                        KoENO_GO_BASE__7.value(true, Dpt(1, 1));
                        break;
                  case 2: // double click
                        KoENO_GO_BASE__8.value(true, Dpt(1, 1));
                        break;
                  case 3: // long click
                        KoENO_GO_BASE__9.value(true, Dpt(1, 1));
                        break;
                  case 4: // long click release
                        KoENO_GO_BASE__10.value(true, Dpt(1, 1));
                        break;

                  default:
                        break;
                  }
                  break;

                  break; // ENDE CASE D2_03_0A
            }
            break; // ENDE CASE D2_03

      case D2_04:
            //**************************************************************
            // ----------------- Profil: D2-04-xx --------------------------
            //**************************************************************

            SenVal_D2_04 = (VLD_D2_04_00_TELEGRAM *)&(f_Pkt_st->u8DataBuffer[1]);

            valuefloat.temp_s = (float)(SenVal_D2_04->temp / 5.0);
            hum_s = (float)(SenVal_D2_04->hum / 2.0);
            voc1_s = (float)(SenVal_D2_04->voc) * 7.84;
            valuefloat2.voc2_s = (float)(SenVal_D2_04->voc) * 19.6;

            //switch (knx.paramWord(firstParameter + ENO_CHProfilVLD04))
            switch (ParamENO_CHProfilVLD04)
            {
            case D2_04_00:
                  KoENO_GO_BASE__1.value(voc1_s, Dpt(9, 1));
                  KoENO_GO_BASE__2.value(valuefloat.temp_s, Dpt(9, 1));
                  KoENO_GO_BASE__3.value(hum_s, Dpt(9, 1));
                  KoENO_GO_BASE__4.value(SenVal_D2_04->u8VldTelSenSta.daynight, Dpt(1, 1));
                  KoENO_GO_BASE__5.value(SenVal_D2_04->u8VldTelSenSta.batt, Dpt(5, 1));
                  logDebug("VLD", "Profil: D2-04-00");
                  break;
            case D2_04_01:
                  KoENO_GO_BASE__1.value(voc1_s, Dpt(9, 1));
                  // KoENO_GO_BASE__2.value(temp_s, Dpt(9, 1));
                  KoENO_GO_BASE__3.value(hum_s, Dpt(9, 1));
                  KoENO_GO_BASE__4.value(SenVal_D2_04->u8VldTelSenSta.daynight, Dpt(1, 1));
                  KoENO_GO_BASE__5.value(SenVal_D2_04->u8VldTelSenSta.batt, Dpt(5, 1));
                  logDebug("VLD", "Profil: D2-04-01");
                  break;
            case D2_04_02:
                  KoENO_GO_BASE__1.value(voc1_s, Dpt(9, 1));
                  KoENO_GO_BASE__2.value(valuefloat.temp_s, Dpt(9, 1));
                  // KoENO_GO_BASE__3.value(hum_s, Dpt(9, 1));
                  KoENO_GO_BASE__4.value(SenVal_D2_04->u8VldTelSenSta.daynight, Dpt(1, 1));
                  KoENO_GO_BASE__5.value(SenVal_D2_04->u8VldTelSenSta.batt, Dpt(5, 1));
                  logDebug("VLD", "Profil: D2-04-02");
                  break;
            case D2_04_03:
                  KoENO_GO_BASE__1.value(voc1_s, Dpt(9, 1));
                  KoENO_GO_BASE__2.value(SenVal_D2_04, Dpt(9, 1));
                  // KoENO_GO_BASE__3.value(hum_s, Dpt(9, 1));
                  KoENO_GO_BASE__5.value(SenVal_D2_04->u8VldTelSenSta.batt, Dpt(5, 1));
                  logDebug("VLD", "Profil: D2-04-03");
                  break;
            case D2_04_04:
                  KoENO_GO_BASE__1.value(voc1_s, Dpt(9, 1));
                  KoENO_GO_BASE__2.value(valuefloat.temp_s, Dpt(9, 1));
                  // KoENO_GO_BASE__3.value(hum_s, Dpt(9, 1));
                  logDebug("VLD", "Profil: D2-04-04");
                  break;
            case D2_04_05:
                  KoENO_GO_BASE__1.value(voc1_s, Dpt(9, 1));
                  KoENO_GO_BASE__2.value(valuefloat.temp_s, Dpt(9, 1));
                  // KoENO_GO_BASE__3.value(hum_s, Dpt(9, 1));
                  KoENO_GO_BASE__4.value(SenVal_D2_04->u8VldTelSenSta.daynight, Dpt(1, 1));
                  logDebug("VLD", "Profil: D2-04-05");
                  break;
            case D2_04_06:
                  KoENO_GO_BASE__1.value(voc1_s, Dpt(9, 1));
                  // KoENO_GO_BASE__2.value(temp_s, Dpt(9, 1));
                  // KoENO_GO_BASE__3.value(hum_s, Dpt(9, 1));
                  KoENO_GO_BASE__4.value(SenVal_D2_04->u8VldTelSenSta.daynight, Dpt(1, 1));
                  logDebug("VLD", "Profil: D2-04-06");
                  break;
            case D2_04_07:
                  KoENO_GO_BASE__1.value(voc1_s, Dpt(9, 1));
                  // KoENO_GO_BASE__2.value(temp_s, Dpt(9, 1));
                  // KoENO_GO_BASE__3.value(hum_s, Dpt(9, 1));
                  KoENO_GO_BASE__4.value(SenVal_D2_04->u8VldTelSenSta.daynight, Dpt(1, 1));
                  KoENO_GO_BASE__5.value(SenVal_D2_04->u8VldTelSenSta.batt, Dpt(5, 1));
                  logDebug("VLD", "Profil: D2-04-07");
                  break;
            case D2_04_08:
                  KoENO_GO_BASE__1.value(valuefloat2.voc2_s, Dpt(9, 1));
                  KoENO_GO_BASE__2.value(valuefloat.temp_s, Dpt(9, 1));
                  KoENO_GO_BASE__3.value(hum_s, Dpt(9, 1));
                  KoENO_GO_BASE__4.value(SenVal_D2_04->u8VldTelSenSta.daynight, Dpt(1, 1));
                  logDebug("VLD", "Profil: D2-04-08");
                  break;
            case D2_04_09:
                  KoENO_GO_BASE__1.value(valuefloat2.voc2_s, Dpt(9, 1));
                  // KoENO_GO_BASE__2.value(temp_s, Dpt(9, 1));
                  KoENO_GO_BASE__3.value(hum_s, Dpt(9, 1));
                  KoENO_GO_BASE__4.value(SenVal_D2_04->u8VldTelSenSta.daynight, Dpt(1, 1));
                  KoENO_GO_BASE__5.value(SenVal_D2_04->u8VldTelSenSta.batt, Dpt(5, 1));
                  logDebug("VLD", "Profil: D2-04-09");
                  break;
            case D2_04_10:
                  KoENO_GO_BASE__1.value(valuefloat2.voc2_s, Dpt(9, 1));
                  KoENO_GO_BASE__2.value(valuefloat.temp_s, Dpt(9, 1));
                  // KoENO_GO_BASE__3.value(hum_s, Dpt(9, 1));
                  KoENO_GO_BASE__4.value(SenVal_D2_04->u8VldTelSenSta.daynight, Dpt(1, 1));
                  KoENO_GO_BASE__5.value(SenVal_D2_04->u8VldTelSenSta.batt, Dpt(5, 1));
                  logDebug("VLD", "Profil: D2-04-10");
                  break;
            case D2_04_1A:
                  KoENO_GO_BASE__1.value(valuefloat2.voc2_s, Dpt(9, 1));
                  KoENO_GO_BASE__2.value(valuefloat.temp_s, Dpt(9, 1));
                  // KoENO_GO_BASE__3.value(hum_s, Dpt(9, 1));
                  KoENO_GO_BASE__5.value(SenVal_D2_04->u8VldTelSenSta.batt, Dpt(5, 1));
                  logDebug("VLD", "Profil: D2-04-1A");
                  break;
            case D2_04_1B:
                  KoENO_GO_BASE__1.value(valuefloat2.voc2_s, Dpt(9, 1));
                  KoENO_GO_BASE__2.value(valuefloat.temp_s, Dpt(9, 1));
                  // KoENO_GO_BASE__3.value(hum_s, Dpt(9, 1));
                  logDebug("VLD", "Profil: D2-04-1B");
                  break;
            case D2_04_1C:
                  KoENO_GO_BASE__1.value(valuefloat2.voc2_s, Dpt(9, 1));
                  KoENO_GO_BASE__2.value(valuefloat.temp_s, Dpt(9, 1));
                  // KoENO_GO_BASE__3.value(hum_s, Dpt(9, 1));
                  KoENO_GO_BASE__4.value(SenVal_D2_04->u8VldTelSenSta.daynight, Dpt(1, 1));
                  logDebug("VLD", "Profil: D2-04-1C");
                  break;
            case D2_04_1D:
                  KoENO_GO_BASE__1.value(valuefloat2.voc2_s, Dpt(9, 1));
                  // KoENO_GO_BASE__2.value(temp_s, Dpt(9, 1));
                  // KoENO_GO_BASE__3.value(hum_s, Dpt(9, 1));
                  KoENO_GO_BASE__4.value(SenVal_D2_04->u8VldTelSenSta.daynight, Dpt(1, 1));
                  logDebug("VLD", "Profil: D2-04-1D");
                  break;
            case D2_04_1E:
                  KoENO_GO_BASE__1.value(valuefloat2.voc2_s, Dpt(9, 1));
                  // KoENO_GO_BASE__2.value(temp_s, Dpt(9, 1));
                  // KoENO_GO_BASE__3.value(hum_s, Dpt(9, 1));
                  KoENO_GO_BASE__4.value(SenVal_D2_04->u8VldTelSenSta.daynight, Dpt(1, 1));
                  KoENO_GO_BASE__5.value(SenVal_D2_04->u8VldTelSenSta.batt, Dpt(5, 1));
                  logDebug("VLD", "Profil: D2-04-1E");
                  break;

            default:
                  break;
            } // ENDE SWITCH D2-04-xx
            logDebug("VLD", "Temp: %.1f, Hum: %.1f, VOC1: %.1f, VOC2: %.1f, Batterie: %u, Day: %u", valuefloat.temp_s, hum_s, voc1_s, valuefloat2.voc2_s, SenVal_D2_04->u8VldTelSenSta.batt, SenVal_D2_04->u8VldTelSenSta.daynight);

            break; // ENDE case D2_04_xx:

      case D2_05:
            //**************************************************************
            // ----------------- Profil: D2-05-xx --------------------------
            //**************************************************************
            switch (f_Pkt_st->u16DataLength)
            {
            case 3: // 3 Byte länge
                  logDebug("VLD", "3Byte ");

                  ActStatResp_D2_05 = (VLD_D2_05_00_TELEGRAM_CMD_04_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);

                  switch (ActStatResp_D2_05->u8VldTelSenSta.CMD)
                  {
                  case 4: // CMD = 4   -->  Status Jaousie Angle / Pos
                          // swap two bytes
                        mem = f_Pkt_st->u8DataBuffer[2];
                        f_Pkt_st->u8DataBuffer[2] = f_Pkt_st->u8DataBuffer[3];
                        f_Pkt_st->u8DataBuffer[3] = mem;

                        KoENO_GO_BASE__1.value(ActStatResp_D2_05->angle, Dpt(5, 1));
                        KoENO_GO_BASE__2.value(ActStatResp_D2_05->pos, Dpt(5, 1));
                        logDebug("VLD", " CMD=4  Pos: %u | Angle: %u", ActStatResp_D2_05->pos, ActStatResp_D2_05->angle);

                        break;
                  }

                  break;

            default:
                  break;
            }
            break; // ENDE D2-05

      case D2_06:
            //**************************************************************
            // ----------------- Profil: D2-06-xx --------------------------
            //**************************************************************
            //switch (knx.paramWord(firstParameter + ENO_CHProfilVLD06))
            switch (ParamENO_CHProfilVLD06)
            {
            //**************************************************************
            // ----------------- Profil: D2-06-01 --------------------------
            //**************************************************************
            case D2_06_01:
                  SenVal_D2_06_01 = (VLD_D2_06_01_TELEGRAM *)&(f_Pkt_st->u8DataBuffer[1]);
                  if (SenVal_D2_06_01->u8MT == 0x00)
                  {
                        logDebug("VLD", "Profil: D2-06-01");
                        // Alarm
                        if (SenVal_D2_06_01->u8VldTelAlarm.BAL || SenVal_D2_06_01->u8VldTelAlarm.PPAL)
                        {
                              KoENO_GO_BASE__4.value(true, Dpt(1, 1));
                        }
                        else
                        {
                              KoENO_GO_BASE__4.value(false, Dpt(1, 1));
                        }
                        logDebug("VLD", "Alarm: %u", SenVal_D2_06_01->u8VldTelAlarm.BAL);
                        // Window State
                        // closed = State1 = 0 & State2 = 0
                        if (SenVal_D2_06_01->u8VldTelWindow.WS == 0x1 && SenVal_D2_06_01->u8VldTelWindow.HP == 0x2) // Window not Tilted = close & Handle down = close
                        {
                              KoENO_GO_BASE__2.value(false, Dpt(1, 1)); // KO Fenster Status 1
                              KoENO_GO_BASE__3.value(false, Dpt(1, 1)); // KO Fenster Status 2
                        }
                        else if (SenVal_D2_06_01->u8VldTelWindow.WS == 0x2) // Window  Tilted = open
                        {
                              KoENO_GO_BASE__2.value(true, Dpt(1, 1));  // KO Fenster Status 1
                              KoENO_GO_BASE__3.value(false, Dpt(1, 1)); // KO Fenster Status 2
                        }
                        else
                        {
                              KoENO_GO_BASE__2.value(true, Dpt(1, 1)); // KO Fenster Status 1
                              KoENO_GO_BASE__3.value(true, Dpt(1, 1)); // KO Fenster Status 2
                        }

                        // Position of Handle
                        // if (SenVal_D2_06_01->u8VldTelWindow.HP == 0x1) // Handle up = open
                        // if (SenVal_D2_06_01->u8VldTelWindow.HP == 0x2) // Handle down = close
                        // if (SenVal_D2_06_01->u8VldTelWindow.HP == 0x3) // Handle left = open
                        // if (SenVal_D2_06_01->u8VldTelWindow.HP == 0x4) // Handle right = open
                        // state of Window
                        // if (SenVal_D2_06_01->u8VldTelWindow.WS == 0x1) // Window not Tilted = close
                        // if (SenVal_D2_06_01->u8VldTelWindow.WS == 0x2) // Window  Tilted = open
                        logDebug("VLD", "handle Pos: %u, Window tilted: %u", SenVal_D2_06_01->u8VldTelWindow.HP, SenVal_D2_06_01->u8VldTelWindow.WS - 1);
                        // Button Right
                        if (SenVal_D2_06_01->u8VldTelButton.BR == 0x1)
                        {
                              mem = true;
                              KoENO_GO_BASE__9.value(mem, Dpt(1, 1));
                        }
                        if (SenVal_D2_06_01->u8VldTelButton.BR == 0x2)
                        {
                              mem = false;
                              KoENO_GO_BASE__9.value(mem, Dpt(1, 1));
                        }
                        if (SenVal_D2_06_01->u8VldTelButton.BR == 0x0)
                        {
                              KoENO_GO_BASE__9.value(mem, Dpt(1, 1));
                        }
                        logDebug("VLD", "But R: %u", SenVal_D2_06_01->u8VldTelButton.BR);
                        // Button Left
                        if (SenVal_D2_06_01->u8VldTelButton.BL == 0x1)
                        {
                              mem2 = true;
                              KoENO_GO_BASE__10.value(mem2, Dpt(1, 1));
                        }
                        if (SenVal_D2_06_01->u8VldTelButton.BL == 0x2)
                        {
                              mem2 = false;
                              KoENO_GO_BASE__10.value(mem2, Dpt(1, 1));
                        }
                        if (SenVal_D2_06_01->u8VldTelButton.BL == 0x0)
                        {
                              KoENO_GO_BASE__10.value(mem2, Dpt(1, 1));
                        }
                        logDebug("VLD", "But L: %u", SenVal_D2_06_01->u8VldTelButton.BL);
                        // Motion
                        KoENO_GO_BASE__1.value(SenVal_D2_06_01->u8VldTelMotionMode.M, Dpt(1, 1));
                        logDebug("VLD", "Motion Det: %u", SenVal_D2_06_01->u8VldTelMotionMode.M);
                        // Temp
                        valuefloat.temp_s = SenVal_D2_06_01->u8Temp * 0.320 - 20.0;
                        KoENO_GO_BASE__7.value(valuefloat.temp_s, Dpt(9, 1));
                        logDebug("VLD", "Temp: %.1f", valuefloat.temp_s);
                        // Hum
                        hum_s = SenVal_D2_06_01->u8Hum / 2;
                        KoENO_GO_BASE__8.value(hum_s, Dpt(9, 1));
                        logDebug("VLD", "Hum: %.1f", hum_s);
                        // Lux
                        value = (((uint16_t)SenVal_D2_06_01->u8LuxMSB) << 8 | SenVal_D2_06_01->u8LuxLSB);
                        KoENO_GO_BASE__6.value(value, Dpt(9, 1));
                        logDebug("VLD", "Lux: %u", value);
                        // Bat
                        valueBat = SenVal_D2_06_01->u8VldTelBat.BS * 5;
                        KoENO_GO_BASE__5.value(valueBat, Dpt(5, 1));
                        logDebug("VLD", "Bat(%%): %u", valueBat);
                  }
                  break;
            //**************************************************************
            // ----------------- Profil: D2-06-50 --------------------------
            //**************************************************************
            case D2_06_50:

                  SenVal_D2_06_50_0x01 = (VLD_D2_06_50_Window_Status_0x01 *)&(f_Pkt_st->u8DataBuffer[1]);

                  if (SenVal_D2_06_50_0x01->u8MT == 0x01)
                  {
                        logDebug("VLD", "Profil: D2-06-50 (0x01)");
                        // Window State
                        KoENO_GO_BASE__2.value(SenVal_D2_06_50_0x01->u8VldTelWindow.WDS, Dpt(5, 1));
                        logDebug("VLD", "Window State: %u", SenVal_D2_06_50_0x01->u8VldTelWindow.WDS);
                        // Counter Status
                        valueU32.counter = (SenVal_D2_06_50_0x01->u8CT_4 << 24) | (SenVal_D2_06_50_0x01->u8CT_3 << 16) | (SenVal_D2_06_50_0x01->u8CT_2 << 8) | SenVal_D2_06_50_0x01->u8CT_1;
                        KoENO_GO_BASE__6.value(valueU32.counter, Dpt(12, 1));
                        logDebug("VLD", "State Counter: %u", valueU32.counter);
                        // Change Battery
                        KoENO_GO_BASE__5.value(SenVal_D2_06_50_0x01->u8VldTelBattery.CB, Dpt(1, 1));
                        logDebug("VLD", "change Bat: %u", SenVal_D2_06_50_0x01->u8VldTelBattery.CB);
                        // Battery State %
                        KoENO_GO_BASE__1.value(SenVal_D2_06_50_0x01->u8VldTelBattery.BS, Dpt(5, 1));
                        logDebug("VLD", "Bat(%%): %u", SenVal_D2_06_50_0x01->u8VldTelBattery.BS);
                        // Status Bits
                        KoENO_GO_BASE__10.value(SenVal_D2_06_50_0x01->u8StatusBits, Dpt(5, 1));
                        logDebug("VLD", "Status Byte: %u", SenVal_D2_06_50_0x01->u8StatusBits);
                  }
                  if (SenVal_D2_06_50_0x01->u8MT == 0x02)
                  {
                        SenVal_D2_06_50_0x02 = (VLD_D2_06_50_Alarm_Status_0x02 *)&(f_Pkt_st->u8DataBuffer[1]);
                        logDebug("VLD", "Profil: D2-06-50 (0x02), Alarm: %u", SenVal_D2_06_50_0x02->u8VldTelAlarm.BA);
                        KoENO_GO_BASE__4.value(SenVal_D2_06_50_0x02->u8VldTelAlarm.BA, Dpt(1, 1));
                  }
                  break;

            default:
                  break;
            }
            break; // ENDE D2-06

      case D2_14:
            //**************************************************************
            // ----------------- Profil: D2-14-xx --------------------------
            //**************************************************************
            //switch (knx.paramWord(firstParameter + ENO_CHProfilVLD14))
            switch (ParamENO_CHProfilVLD14)
            {
            //**************************************************************
            // ----------------- Profil: D2-14-30 --------------------------
            //**************************************************************
            case D2_14_30:
                  SenStat_D2_14_30 = (VLD_D2_14_30_TELEGRAM *)&(f_Pkt_st->u8DataBuffer[1]);
                  logDebug("VLD", "Profil: D2-14-30");
                  // Smoke Alarm
                  KoENO_GO_BASE__4.value(SenStat_D2_14_30->u8VldTelSenSta5.smokeAlarm, Dpt(1, 1));
                  logDebug("VLD", "Smoke Alarm: %u", SenStat_D2_14_30->u8VldTelSenSta5.smokeAlarm);
                  // Status Bits
                  KoENO_GO_BASE__10.value(SenStat_D2_14_30->u8VldTelSenSta5.statusbits, Dpt(5, 1));
                  logDebug("VLD", "Status Bits: %u", SenStat_D2_14_30->u8VldTelSenSta5.statusbits);
                  // Energy Storage (Battery)
                  KoENO_GO_BASE__5.value(SenStat_D2_14_30->u8VldTelSenSta4.ES, Dpt(5, 1));
                  logDebug("VLD", "Battery: %u", SenStat_D2_14_30->u8VldTelSenSta4.ES);
                  // IAQTH indoor AIR Quality
                  KoENO_GO_BASE__8.value(SenStat_D2_14_30->u8VldTelSenSta.IAQTH, Dpt(5, 1));
                  logDebug("VLD", "Air Quality: %u", SenStat_D2_14_30->u8VldTelSenSta.IAQTH);
                  // HUM
                  mem = (SenStat_D2_14_30->u8VldTelSenSta2.HUM_MSB << 7) | SenStat_D2_14_30->u8VldTelSenSta1.HUM_LSB;
                  hum_s = mem / 2.0;
                  KoENO_GO_BASE__3.value(hum_s, Dpt(9, 1));
                  logDebug("VLD", "Hum: %.1f", hum_s);
                  // TEMP
                  mem = (SenStat_D2_14_30->u8VldTelSenSta3.Temp_MSB << 7) | SenStat_D2_14_30->u8VldTelSenSta2.Temp_LSB;
                  valuefloat.temp_s = mem / 5.0;
                  KoENO_GO_BASE__2.value(valuefloat.temp_s, Dpt(9, 1));
                  logDebug("VLD", "Temp: %.1f", valuefloat.temp_s);
                  // HCI
                  mem = (SenStat_D2_14_30->u8VldTelSenSta1.HCI_MSB << 1) | SenStat_D2_14_30->u8VldTelSenSta.HCI_LSB;
                  KoENO_GO_BASE__7.value(mem, Dpt(5, 1));
                  logDebug("VLD", "HCI: %u", mem);
                  // RPLT
                  mem = (SenStat_D2_14_30->u8VldTelSenSta4.RPLT_MSB << 7) | SenStat_D2_14_30->u8VldTelSenSta3.RPLT_LSB;
                  KoENO_GO_BASE__6.value(mem, Dpt(5, 1));
                  logDebug("VLD", "RPLT: %u", mem);
                  // time since last Event
                  mem = (SenStat_D2_14_30->u8VldTelSenSta5.LastEvent_MSB << 5) | SenStat_D2_14_30->u8VldTelSenSta4.LastEvent_LSB;
                  KoENO_GO_BASE__9.value(mem, Dpt(5, 1));
                  logDebug("VLD", "Time last Event: %u", mem);
                  break;

            //**************************************************************
            // ----------------- Profil: D2-14-41 --------------------------
            //**************************************************************
            case D2_14_41:
                  SenStat_D2_14_41 = (VLD_D2_14_41_TELEGRAM *)&(f_Pkt_st->u8DataBuffer[1]);
                  logDebug("VLD", "Profil: D2-14-41");
                  // Temp-Sensor
                  value = SenStat_D2_14_41->Temp_MSB<<2;
                  value = value^SenStat_D2_14_41->u8VldTelSenSta7.Temp_LSB;
                  valuefloat.temp_s = (float)(value / 10.230) - 40;
                  KoENO_GO_BASE__2.value(valuefloat.temp_s, Dpt(9, 1));
                  logDebug("VLD", "Temp: %.1f  RAW: %u", valuefloat.temp_s, value);
                  // Hum-Sensor
                  mem = SenStat_D2_14_41->u8VldTelSenSta7.Hum_MSB << 2;
                  mem = mem^SenStat_D2_14_41->u8VldTelSenSta6.Hum_LSB;
                  hum_s = (float)mem/2;
                  KoENO_GO_BASE__3.value(hum_s, Dpt(9, 1));
                  logDebug("VLD", "Hum: %.1f  RAW: %u", hum_s, mem);
                  // LUX-Sensor
                  valueU32.lux = SenStat_D2_14_41->u8VldTelSenSta6.LUX_MSB << 8;
                  valueU32.lux = valueU32.lux^SenStat_D2_14_41->LUX;
                  valueU32.lux = valueU32.lux << 3;
                  valueU32.lux = valueU32.lux^SenStat_D2_14_41->u8VldTelSenSta4.LUX_LSB;
                  valueU32.lux = valueU32.lux & 0x1ffff;
                  valuefloat.lux = (float)valueU32.lux;
                  KoENO_GO_BASE__6.value(valueU32.lux, Dpt(9, 1));
                  logDebug("VLD", "LUX: %.1f  RAW: %u", valuefloat.lux, valueU32.lux);
                  // Sensor Status
                  KoENO_GO_BASE__10.value(SenStat_D2_14_41->u8VldTelSenSta4.SensorStatus, Dpt(5, 1));
                  logDebug("VLD", "Sensor Status: %u", SenStat_D2_14_41->u8VldTelSenSta4.SensorStatus);
                  // ACC_X
                  value = SenStat_D2_14_41->u8VldTelSenSta4.ACCX_MSB << 7;
                  value = value^SenStat_D2_14_41->u8VldTelSenSta3.ACCX_LSB;
                  valuefloat2.acc_value = (value-500)*0.005;
                  KoENO_GO_BASE__7.value(valuefloat2.acc_value, Dpt(14, 1));
                  logDebug("VLD", "ACC_X: %.3f  RAW: %u", valuefloat2.acc_value, value);
                  // ACC_Y
                  value = SenStat_D2_14_41->u8VldTelSenSta3.ACCY_MSB << 8;
                  value = value^SenStat_D2_14_41->u8VldTelSenSta2.ACCY_MID;
                  value = (value << 1);
                  value = value^SenStat_D2_14_41->u8VldTelSenSta1.ACCY_LSB;
                  valuefloat2.acc_value = (value-500)*0.005;
                  KoENO_GO_BASE__8.value(valuefloat2.acc_value, Dpt(14, 1));
                  logDebug("VLD", "ACC_Y: %.3f  RAW: %u", valuefloat2.acc_value, value);
                  // ACC_Z
                  value = SenStat_D2_14_41->u8VldTelSenSta1.ACCZ_MSB << 3;
                  value = value^SenStat_D2_14_41->u8VldTelSenSta.ACCZ_LSB;
                  valuefloat2.acc_value = (value-500)*0.005;
                  KoENO_GO_BASE__9.value(valuefloat2.acc_value, Dpt(14, 1));
                  logDebug("VLD", "ACC_Z: %.3f  RAW: %u", valuefloat2.acc_value, value);

                  // Contact
                  KoENO_GO_BASE__4.value(!SenStat_D2_14_41->u8VldTelSenSta.contact, Dpt(1, 1));
                  logDebug("VLD", "Contact: %u", !SenStat_D2_14_41->u8VldTelSenSta.contact);
                  break;

            default:
                  break;
            }
            break; // ENDE D2-14
      default:
            logDebug("VLD", "Profil: ERROR");
            break;
      } // ENDE D2-xx
}
