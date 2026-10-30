### EnOcean Gateway

Das Gateway verbindet bis zu 30 EnOcean-Geräte mit KNX. Jedes Gerät belegt einen Kanal mit bis zu 10 Kommunikationsobjekten.

**Einrichtung eines Kanals**

1. Unter **Profildefinition** wählen, ob das Gerät über ein EEP-Profil oder als Eltako-Produkt eingerichtet wird.
2. Unter **Einrichtung des Geräts** wählen, wie die EnOcean-ID ins Gateway kommt: per **Teach-In** am Gerät oder **ID manuell** eingegeben.
3. Profil- bzw. Produkteinstellungen und Kommunikationsobjekte festlegen.

**Allgemein**

- **Verfügbare Kanäle:** Anzahl der Kanäle, die in der ETS angezeigt werden.
- **Senden Unknown-IDs:** meldet IDs von Geräten, die keinem Kanal zugeordnet sind.
- **Repeater:** Das Gateway wiederholt empfangene Funktelegramme.
- **Base-ID:** Startwert der Sende-IDs des Gateways, wird in der ETS gespeichert und nach einem Hardwaretausch wieder gesetzt.
- **Teach-In Übersicht:** Status aller Kanäle und Dauer des Teach-In-Zeitfensters.

**Einlernen über KNX**

Das KO **aktiviert Kanal zum Einlernen** startet das Teach-In wie der Button in der ETS: Wert 1 bis 30 = Teach-In für diesen Kanal, 255 = Teach-In beenden, 100 = aktiven Kanal abfragen. Das KO **Channel bereit zum Einlernen** meldet den Kanal, der gerade einlernt (0 = keiner).
