#pragma once

#include "EnoceanModule.h"
#include "knxprod.h"

// Wert des KOs "offen/geschlossen" gemäß ETS-Parameter "Wert für geschlossen": im Ruhezustand (geschlossen) wird der
// Parameterwert gesendet, sonst (offen / gekippt) der invertierte Wert.
inline bool windowKoValue(bool active, uint8_t _channelIndex) {
  return active != ParamENO_CHWindowcloseValue;
}

// Schreibt den Fenstergriff-/Kontaktzustand auf die Gruppenobjekte (Belegung wie A5-14-09):
// KO2 = offen, KO3 = gekippt (beide fest 1 = Zustand liegt vor), KO4 = offen/geschlossen (nach "Wert für geschlossen").
inline void setWindowHandleKOs(bool open, bool tilted, uint8_t _channelIndex) {
  bool closed = !open && !tilted;
  KoENO_GO_BASE__2.value(open, Dpt(1, 1));
  KoENO_GO_BASE__3.value(tilted, Dpt(1, 1));
  KoENO_GO_BASE__4.value(windowKoValue(!closed, _channelIndex), Dpt(1, 1));
}
