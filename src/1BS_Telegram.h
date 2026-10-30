#pragma once

#include "EnoceanModule.h"
#include "EnoceanProfils.h"
#include "EnoceanStructs.h"
#include "knxprod.h"

/*******************************************************************************************
 *  1BS   D5-00-01
 *******************************************************************************************/

// Verarbeitet ein empfangenes 1BS-EnOcean-Telegramm (D5-00-01 Kontakt) und schreibt den Open/Close-Zustand auf das
// KNX-Gruppenobjekt.
inline void handle_1BS(PACKET_SERIAL_TYPE_ *f_Pkt_st, uint8_t _channelIndex) {
  bool bvalue;

  ONEBS_TELEGRAM_TYPE *l1bsTlg_p;

  logDebug("1BS", "Profil: 1BS - %u", ParamENO_CHProfilSelection1BS);

  switch (ParamENO_CHProfilSelection1BS) {
  case D5_00_01:

    // Läd den Wert in das Profil-Strukt
    l1bsTlg_p = (ONEBS_TELEGRAM_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);

    // ETS Parameter to define state for open / Close
    // D5-00-01 Contact: 0 = open / 1 = close
    if (l1bsTlg_p->u81bsTelData.State == 1) // CLOSE
    {
      if (ParamENO_CHWindowcloseValue)
        bvalue = true;
      else
        bvalue = false;
    } else { // OPEN
      if (ParamENO_CHWindowcloseValue)
        bvalue = false;
      else
        bvalue = true;
    }
    KoENO_GO_BASE__4.valueNoSend(bvalue, DPT_OpenClose);
    KoENO_GO_BASE__4.objectWritten();

    logDebug("1BS", "D5-00-01 detected: 1BS State: %u, Output KNX: %u", l1bsTlg_p->u81bsTelData.State, bvalue);
    break;

  default:
    break;
  }
}