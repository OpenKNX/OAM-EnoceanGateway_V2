#pragma once

#include "EnoceanModule.h"
#include "knxprod.h"

// Alarm-KO (DPT 1.005) für alle Alarm-Profile: ein aktiver Alarm wird immer gesendet. Ein aufgehobener Alarm nur, wenn
// per ETS "Alarm zurücksetzen = automatisch" gewählt ist; sonst bleibt das KO auf 1, bis es von KNX (z.B. durch eine
// Person in der Visualisierung) zurückgesetzt wird.
inline void sendAlarm(GroupObject &ko, bool active, uint8_t _channelIndex) {
  if (active || !ParamENO_CHAlarmLatch)
    ko.value(active, Dpt(1, 5));
}
