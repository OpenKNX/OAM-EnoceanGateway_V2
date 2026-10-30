### Taster KNX nach EnOcean

Der Kanal bildet einen EnOcean-Taster nach. Über das Eingangs-KO wird ein Aktor (z.B. Eltako-Schalt-, Dimm- oder Jalousieaktor) per Funk gesteuert:

- **Schalten:** EIN = Taste oben, AUS = Taste unten.
- **Dimmen:** Taste wird gedrückt gehalten, bis Stopp empfangen wird (spätestens 10 s).
- **Jalousie:** AUF/AB = Taste kurz tippen, Stopp = erneut in der letzten Richtung tippen.

Der Kanal sendet mit der Sende-ID **Base-ID + Kanalnummer − 1**.

**Lerntelegramm an Aktor senden**

1. Den Aktor in den Lernmodus bringen (siehe Anleitung des Aktors).
2. Bei bestehender Online-Verbindung **Lerntelegramm an Aktor senden** drücken. Das Gateway sendet einen Tastendruck oben mit der Sende-ID dieses Kanals.
3. Den Lernmodus am Aktor beenden.

Ohne ETS geht das auch über die Konsole mit `eno learn <Kanal>`.
