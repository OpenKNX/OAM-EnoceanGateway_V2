### Ausfallüberwachung

Meldet über das KO **Gerät ausgefallen** (DPT 1.005), wenn vom Gerät dieses Kanals länger als die eingestellte Zeit kein Telegramm empfangen wurde. Typische Ursachen: leere Batterie, zu wenig Licht bei Solarsensoren, Gerät außer Reichweite.

- **aus:** keine Überwachung, das KO wird nicht angezeigt.
- **1 bis 48 h:** Zeit ohne Telegramm, nach der der Ausfall gemeldet wird.

Sobald das Gerät wieder sendet, geht das KO auf 0. Die Zeit startet nach jedem Neustart des Gateways neu.

**Zeit passend zum Gerät wählen:** Viele Sensoren senden zyklisch alle 15 bis 60 Minuten ein Lebenszeichen, dann passt z.B. 2 bis 4 h. Kontakte und Taster senden oft nur bei Änderung oder Betätigung, dort ist die Überwachung nur mit sehr langen Zeiten oder gar nicht sinnvoll.
