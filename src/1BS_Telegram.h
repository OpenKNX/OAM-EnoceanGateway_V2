#pragma once

#include "EnoceanModule.h"
#include "EnoceanProfils.h"
#include "EnoceanStructs.h"
#include "knxprod.h"
#include "WindowContact.h"

/*******************************************************************************************
 *  1BS   D5-00-01
 *******************************************************************************************/

// D5-00-01 Kontakt: DB0 Bit 0 = Kontakt (1 = geschlossen), Bit 3 = LRN (0 = Lerntelegramm, wird ignoriert).
// KO_3 (GO_BASE__4) offen/geschlossen, Wert für geschlossen per ETS. Wird vom EEP-Profil und von den Eltako-Kontakten
// (FTK, FTKB, FFKB, FTKB-gr) genutzt.
inline void handle1BS_D5_00_01(PACKET_SERIAL_TYPE_ *f_Pkt_st, uint8_t _channelIndex) {
  uint8_t db0 = f_Pkt_st->u8DataBuffer[1];
  if (!(db0 & 0x08)) {
    logDebug("1BS", "D5-00-01 teach-in telegram ignored");
    return;
  }
  ONEBS_TELEGRAM_TYPE *l1bsTlg_p = (ONEBS_TELEGRAM_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);
  bool bvalue = windowKoValue(l1bsTlg_p->u81bsTelData.State != 1, _channelIndex);
  KoENO_GO_BASE__4.valueNoSend(bvalue, DPT_OpenClose);
  KoENO_GO_BASE__4.objectWritten();
  logDebug("1BS", "D5-00-01: State %u, KNX %u", l1bsTlg_p->u81bsTelData.State, bvalue);
}

// Verarbeitet ein empfangenes 1BS-EnOcean-Telegramm im EEP-Pfad (Profil per ETS, derzeit nur D5-00-01).
inline void handle_1BS(PACKET_SERIAL_TYPE_ *f_Pkt_st, uint8_t _channelIndex) {
  logDebug("1BS", "Profil: 1BS - %u", ParamENO_CHProfilSelection1BS);
  if (ParamENO_CHProfilSelection1BS == D5_00_01)
    handle1BS_D5_00_01(f_Pkt_st, _channelIndex);
}
