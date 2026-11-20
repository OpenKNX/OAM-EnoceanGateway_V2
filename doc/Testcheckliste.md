# Testcheckliste EnOcean Gateway V2

Für den Stand mit 20 KOs je Kanal. Punkte, die nur aus der Spezifikation abgeleitet und noch nicht am Gerät geprüft sind, stehen zuerst. Konsole: `eno signal` zeigt Signalstärke und Repeater-Stufe aller Kanäle, das Log zeigt jedes empfangene Telegramm (`RX R-ORG ...`).

## Vorbereitung

- [ ] Version gesetzt, `ReplacesVersions` leer
- [ ] knxprod importiert, Gerät in der ETS **neu eingefügt** (kein Update möglich)
- [ ] Firmware geflasht, ETS meldet beim Download keine Abweichung von Firmware und Applikation
- [ ] Konsole `v`: OpenKNX-ID 0xA2, Applikation und Version wie in der ETS
- [ ] Logikkanäle beginnen bei KO 620

## Taster (EEP „RPS - Schalter“ und Eltako)

Bedienung „Wippe“:

- [ ] 1 Wippe, Schalten: oben EIN / unten AUS auf KO0
- [ ] Dimmen: kurz EIN/AUS (KO0), lang heller/dunkler (KO1), Loslassen stoppt
- [ ] Jalousie: lang AUF/AB (KO0), kurz Stopp/Lamelle (KO1)
- [ ] 2 Wippen: rechte Wippe = B (KO4/KO5), linke Wippe = A (KO0/KO1)
- [ ] 1 + 2 Wippen: beide Wippen gleichzeitig = Wippe AB (KO8/KO9), einzelne Wippen lösen dabei nicht aus

Bedienung „Einzeltasten“:

- [ ] Taste oben und unten mit verschiedenen Funktionen, KOs: A oben KO0/1, A unten KO2/3, B oben KO4/5, B unten KO6/7, AB oben KO8/9, AB unten KO10/11
- [ ] Umschalten: jeder Druck schaltet um; Wert von KNX auf das KO schreiben, nächster Druck schaltet davon ausgehend
- [ ] Gedrückt: Drücken 1 / Loslassen 0; „Beim Loslassen senden = Nein“ sendet nur die 1; invertiert 0 / 1
- [ ] Gedrückt + langer Tastendruck „EIN senden“: kurz = 1 auf dem ersten KO, lang = 1 auf dem zweiten KO
- [ ] Umschalten + lang Szene: kurz Umschalten, lang Szene auf dem zweiten KO (1-Byte)
- [ ] Dimmen mit einer Taste: kurz Umschalten, lang dimmen, Richtung wechselt bei jedem langen Druck, bei ausgeschaltetem Licht heller
- [ ] Jalousie mit einer Taste: lang fahren (Richtung wechselt), kurz Stopp
- [ ] Wippe AB als Einzeltasten mit langem Tastendruck
- [ ] Wechsel Umschalten ↔ Gedrückt in der ETS: Gruppenadressen bleiben erhalten

Geräte und Sonderfälle:

- [ ] Taster mit 1 Taste (z.B. FMH1W) und EEP-Schaltertyp „1 Taste (F6-01-01)“
- [ ] FZS65 Zugschalter (0x30 / 0x00)
- [ ] FS55 / FS65E (0x76 / 0x56) als 1 Wippe
- [ ] 6-/8-fach-Sender: beide IDs in je einem Kanal, Zuordnung „Tasten 1-4 (1. ID)“ / „Tasten 5-x (2. ID)“ passt zu den physischen Tasten
- [ ] **Repeater-Filter**: Taster über einen Repeater empfangen, „Umschalten“ schaltet genau einmal; schnelles Doppeldrücken derselben Taste (< 250 ms) zählt als ein Druck
- [ ] Zeit langer Tastendruck 300 ms und 500 ms

## Taster KNX -> EnOcean

- [ ] Rocker-Emulation (KO0): Schalten, Dimmen, Jalousie, Lerntelegramm in einen Eltako-Aktor
- [ ] **F6-02-03**: Lerntelegramm (Licht EIN), KO1 EIN/AUS (0x70/0x50), KO0 heller/dunkler (0x30/0x10) – reagiert der Aktor ohne Loslass-Telegramm?

## Kontakte

- [ ] **D5-00-01 über EEP-Profil**: Zustand auf **KO3** (vorher fälschlich KO0 angezeigt)
- [ ] **Eltako FTK / FTKB / FFKB / FTKB-gr**: Zustand auf KO3 (vorher abhängig vom ausgeblendeten EEP-Parameter)
- [ ] Lerntaste am Kontakt drücken: kein „offen“ auf dem Bus
- [ ] Wert für geschlossen 1 und 0: nur KO3 (offen/geschlossen) folgt dem Parameter, KO1 offen und KO2 gekippt senden immer 1 = Zustand liegt vor
- [ ] Fenstergriff F6-10-00 (FFG7B, FTKE), A5-14-09
- [ ] eTronic (A5-14-01), mTronic (A5-14-0A): Kontakt, Fensterzustand, Vibration, Batteriespannung

## Alarm

- [ ] Alarm zurücksetzen = automatisch: Alarm 1, Aufhebung 0
- [ ] Alarm zurücksetzen = nur Alarm senden: Aufhebung sendet nichts, KO bleibt 1; 0 von KNX schreiben setzt das KO zurück, nächster Alarm sendet wieder 1
- [ ] **FWS81**: Wasser = 1, trocken = 0 (Statusbyte 0x30 / 0x20); über Eltako-Produkt und über EEP F6-05-01
- [ ] FRWB / FHMB (A5-30-03), F6-05-00/02, D2-14-30

## Sensoren

- [ ] **A5-09 über EEP-Profil** (CO2 A5-09-04, VOC A5-09-05 / 0C): KOs sichtbar, Werte kommen (war vorher nie funktionsfähig)
- [ ] FHD60SB: KO LUX sichtbar und plausibel
- [ ] **FAH65S / FIH65S**: Helligkeit kommt an (Data_byte0 = 0x0F)
- [ ] neue Eltako-Sensoren: FFT65B/FFTF65B/FFT55B, FLGTF65/55, FCO2TS, FB65B/FB55B, FBH…-Melder
- [ ] **F6-04-01 / 02 Hotelkartenschalter**: KO0 = 1 bei gesteckter Karte, 0 beim Ziehen

## Neue A5-Profile

- [ ] A5-10 Raumbediengerät (z.B. Eltako FTR78S = A5-10-03, FTR86B = A5-10-06): Temperatur, Sollwert-Stellrad, ggf. Lüfter, Präsenz, Tag/Nacht
- [ ] A5-12-01 Stromzähler mit Teiler (DIV): Nachkommastellen korrekt; A5-12-02/03 Gas/Wasser: m³ und l/h
- [ ] A5-13-01 Wetterstation (z.B. Eltako FWS61): Temperatur, Dämmerung, Wind, Regen, Tag/Nacht, Sonne West/Süd/Ost
- [ ] A5-30-01/02 Digitaleingang: Kontakt, Batterie schwach
- [ ] A5-09-08/09 CO2
- [ ] Eltako-Raumregler: FTR78S (Soll 8..30 °C), FTR86B / FTR..-Serie FHK, FTR65HS / FTAF (Nachtabsenkung), FUTH (Feuchte und **Soll-Feuchte – Skalierung prüfen**, angenommen 0..255 = 0..100 %)
- [ ] Eltako FWS61 über die Kategorie Wetterstation
- [ ] Eltako-Zähler (FWZ14, DSZ14, F3Z14D Strom/Gas/Wasser): Zählerstand mit 0,1 kWh, Tarifwechsel, keine falschen Werte durch die Seriennummer-Telegramme alle 10 Minuten

## Heizung und Aktoren

- [ ] FKS-SV, FKS-B, **FKS-H** (A5-20-04)
- [ ] A5-38-08 Schalt- und Dimmaktor (EEP und Eltako), Status aus Bestätigungen
- [ ] Eltako-Jalousieaktor: Position, Endlagen, Zwischenposition
- [ ] D2-01 Aktor: Schalten, Status, Messung

## Funksignal und Ausfall

- [ ] Funksignal-Diagnose „bei jedem Telegramm“: KO17 negative dBm (z.B. -65), KO18 Repeater-Stufe 0 bei Direktempfang, 1 über Repeater
- [ ] „nur bei Änderung“: sendet erst ab 3 dBm Änderung oder anderer Repeater-Stufe
- [ ] `eno signal`: Werte und Alter für alle Kanäle mit Empfang
- [ ] Ausfallüberwachung: KO19 meldet Ausfall nach eingestellter Zeit, 0 sobald das Gerät wieder sendet

## ETS

- [ ] Hilfetexte erscheinen bei den Kanalparametern
- [ ] Trennlinien zwischen den Wippen und unter „Zeit langer Tastendruck“
- [ ] Release-Build (Strg+Shift+B) läuft durch, `check_ets_ui.py` meldet 0 Fehler
