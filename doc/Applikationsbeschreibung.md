# Applikationsbeschreibung EnOcean Gateway V2

## Wichtige Hinweise

* Diese KNXprod wird nicht von der KNX Association offiziell unterstützt!
* Die Erzeugung der KNXprod geschieht auf Eure eigene Verantwortung!

## Änderungshistorie

### Entwicklungsstand 0.7 (seit 0.2)

* **Eltako-Taster** unter Hersteller ELTAKO → Taster: alle gängigen Eltako-Taster und Handsender mit einer Taste, einer Wippe, zwei Wippen sowie 6- und 8-fach-Sender (siehe [Supported-EnOcean-Devices](Supported-EnOcean-Devices.md)).
* **Bedienung je Wippe**: „Wippe (oben/unten gemeinsam)“ oder „Einzeltasten (oben und unten getrennt)“, im EEP-Pfad (Rocker) und bei den Eltako-Tastern.
* **Einzeltasten**: Funktionen Umschalten, Gedrückt, Szene, Dimmen und Jalousie mit einer Taste; langer Tastendruck mit eigener Funktion (Umschalten, EIN, AUS, Szene) auf einem eigenen KO.
* **Beide Wippen gleichzeitig** (Wippe AB) mit eigenen KOs.
* **KO-Belegung der Taster neu geordnet**: alle KOs einer Wippe stehen zusammen (siehe [KO-Belegung Taster](#ko-belegung-taster)). **Gruppenadressen an Taster-KOs müssen nach dem Update neu zugeordnet werden.**
* **Fensterkontakte**: „Wert für geschlossen“ gilt jetzt für alle drei KOs (offen, gekippt, offen/geschlossen); bei A5-14-01..04 wirkte der Parameter vorher gar nicht.
* **Alarm-Profile**: neuer Parameter „Alarm zurücksetzen“ (automatisch oder nur Alarm senden).
* **Eltako FWS81** (Wassersensor) umgesetzt; F6-05-01 wertet das Statusbyte aus (Alarm wird wieder aufgehoben).
* **Eltako FHD60SB**: KO LUX wird in der ETS angezeigt (vorher fehlte es).
* **A5-09 (CO2/VOC) über EEP-Profil** funktioniert jetzt (ETS und Firmware nutzten verschiedene Profilnummern).
* Aktoren KNX → EnOcean: A5-38-08 (EEP und Eltako), Eltako-Jalousieaktor A5-3F-7F, FSUD-230V, D2-01 Typ 00..13.
* Speicher je Kanal von 41 auf 45 Byte vergrößert.

## Dokumentation

* [Unterstützte EnOcean-Geräte (Eltako)](Supported-EnOcean-Devices.md)
* Übersicht der EEP-Profile: [README](../README.md)

## Allgemeine Parameter

### In Betrieb senden alle

Das Gerät kann einen Status "Ich bin noch in Betrieb" über das KO 1 senden. Hier wird das Sendeintervall in Sekunden eingestellt.

### Zeit bis das Gerät nach einem Neustart aktiv wird

Nach einem Neustart des Geräts, sei es durch Busspannungsausfall, Reset über den Bus oder auch durch ein Drücken der Reset-Taste, kann man hier festlegen, wie viele Sekunden vergehen sollen, bis das Gerät seine Funktion aufnimmt.

Da das Gerät prinzipiell (sofern parametriert) auch Lesetelegramme auf den Bus senden kann, kann mit dieser Einstellung verhindert werden, dass bei einem Busneustart von vielen Geräten viele Lesetelegramme auf einmal gesendet werden und so der Bus überlastet wird.

## Kanäle

Jeder Kanal wird entweder **über EEP-Profil** (beliebiger Hersteller, Profil frei wählbar) oder über **Hersteller: ELTAKO** (Produkt-Kategorie und Produkt) eingerichtet. Beim Eltako-Pfad berücksichtigt die Firmware die Abweichungen der Eltako-Geräte vom EEP-Standard (Quelle: Eltako „Inhalte der Eltako-Funktelegramme“). Beide Pfade nutzen im Hintergrund dieselbe Auswertung.

Pro Kanal stehen 11 KOs zur Verfügung (KO0 bis KO10, für Kanal 1 die KO-Nummern 20 bis 30). KO10 ist immer die Ausfallmeldung.

## Taster

### Tastenlayout

Im EEP-Pfad (Profil Rocker) wird das Layout über „Schaltertyp“ gewählt: 1 Wippe, 2 Wippen oder 1 + 2 Wippen (A, B und beide gleichzeitig = AB). Bei den Eltako-Tastern ergibt es sich aus dem Produkt:

| Eltako-Produkt | Telegramm | Layout |
|---|---|---|
| 1 Taste (z.B. FMH1W, F1T55E) | F6-01-01: Drücken 0x10, Loslassen 0x00 | eine Einzeltaste (Einstellungen wie „Wippe A, Taste oben“) |
| 1 Wippe (z.B. F2T55E, FMH2) | F6-02-01: oben 0x70, unten 0x50 | Wippe A |
| 2 Wippen (z.B. F4T55E, FMH4) | F6-02-01: rechts 0x70 / 0x50 = Wippe B, links 0x30 / 0x10 = Wippe A | Wippe A, B und AB |
| 6- und 8-fach-Sender (F6T…, FF8, FMH8) | zwei Sende-IDs | nur Einzeltasten, kein AB |

**6- und 8-fach-Sender** senden mit zwei IDs. Ein Kanal kann nur eine ID, deshalb gibt es pro Tastengruppe einen eigenen Produkteintrag (z.B. „FMH8 - Tasten 1-4 (1. ID)“ und „FMH8 - Tasten 5-8 (2. ID)“). Für jede Tastengruppe einen eigenen Kanal anlegen und mit einer Taste dieser Gruppe einlernen.

### Bedienung je Wippe

* **Wippe (oben/unten gemeinsam)**: eine Funktion für die ganze Wippe (Schalten EIN/AUS, Dimmen, Jalousie, Szene a/b).
* **Einzeltasten (oben und unten getrennt)**: Taste oben und Taste unten bekommen je eine eigene Funktion und eigene KOs, z.B. oben „Tor öffnen“, unten „Licht Einfahrt“. Die Umstellung gilt immer für beide Enden der Wippe.

### Funktionen einer Einzeltaste

| Funktion | kurzer Tastendruck | langer Tastendruck |
|---|---|---|
| Umschalten | Umschalten | wählbar (siehe unten) |
| Gedrückt | Drücken = 1, optional Loslassen = 0, optional invertiert | wählbar (siehe unten) |
| Szene | Szene (oben Szene a, unten Szene b) | wählbar (siehe unten) |
| Dimmen | Umschalten | Dimmen, Richtung wechselt bei jedem langen Druck (bei ausgeschaltetem Licht immer heller), Loslassen stoppt |
| Jalousie | Stopp/Lamelle in letzter Fahrtrichtung | Fahren, Richtung wechselt bei jedem langen Druck |

**Langer Tastendruck** (Umschalten, Gedrückt, Szene): keine, Umschalten, EIN senden, AUS senden oder Szene (eigene Szenennummer „Szene lang“). Die Zeit für den langen Tastendruck ist einstellbar (300–500 ms).

* Ohne langen Tastendruck reagiert die Taste sofort beim Drücken.
* Mit langem Tastendruck wird der kurze Druck erst beim Loslassen ausgeführt. Bei „Gedrückt“ mit „Beim Loslassen senden“ entsteht dann ein kurzer Impuls (1, direkt danach 0).
* Wippe AB hat bei Einzeltasten nur Umschalten, Gedrückt und Szene (keine freien KOs für langen Tastendruck).

Beispiel: kurzer Tastendruck sendet 1 auf 0/0/1, langer Tastendruck sendet 1 auf 0/0/2 → Funktion „Gedrückt“, „Langer Tastendruck“ = „EIN senden“, erstes KO = 0/0/1, zweites KO = 0/0/2.

### KO-Belegung Taster

Alle KOs einer Wippe stehen in der ETS untereinander.

| KO | Bedienung „Wippe“ | Bedienung „Einzeltasten“ |
|---|---|---|
| KO0 | Wippe A: Schalten / AUF-AB / Szene | A oben: kurz |
| KO1 | Wippe A: Dimmen / Stopp-Lamelle | A oben: lang / Dimmen / Stopp |
| KO2 | – | A unten: kurz |
| KO3 | – | A unten: lang / Dimmen / Stopp |
| KO4 | Wippe B: Schalten / AUF-AB / Szene | B oben: kurz |
| KO5 | Wippe B: Dimmen / Stopp-Lamelle | B oben: lang / Dimmen / Stopp |
| KO6 | – | B unten: kurz |
| KO7 | – | B unten: lang / Dimmen / Stopp |
| KO8 | Wippe AB: Schalten / AUF-AB / Szene | AB oben |
| KO9 | Wippe AB: Dimmen / Stopp-Lamelle | AB unten |
| KO10 | Ausfall | Ausfall |

Die Schalt-KOs (Umschalten / Gedrückt) sind beschreibbar, damit „Umschalten“ den aktuellen Zustand vom Bus übernimmt. Umschalten und Gedrückt nutzen dasselbe KO, die Gruppenadressen bleiben beim Wechsel erhalten.

KO0 ist bei der Richtung KNX → EnOcean (Taster-Emulation) der Eingang; diese Richtung ist von der Auswertung EnOcean → KNX getrennt.

### Repeater

EnOcean-Repeater senden jedes Telegramm ein zweites Mal. Ein zweites „Gedrückt“ derselben Taste innerhalb von 250 ms wird verworfen, sonst würde z.B. „Umschalten“ zweimal schalten.

## Fenster- und Türkontakte

KO1 = offen, KO2 = gekippt, KO3 = offen/geschlossen (Belegung wie A5-14-09). Der Parameter **„Wert für geschlossen“** gilt für alle drei KOs: im Ruhezustand (geschlossen) wird der Parameterwert gesendet, bei aktivem Zustand der invertierte Wert. Voreinstellung ist 1 (DPT 1.009: 1 = geschlossen); damit senden „offen“ und „gekippt“ im aktiven Zustand eine 0. Wer bei offenem Fenster eine 1 erwartet, stellt den Parameter auf 0.

Gilt für F6-10-00/01, A5-14-01..04, A5-14-09/0A und D5-00-01.

## Alarm-Profile

Alarm-KO ist immer KO3 (DPT 1.005), mit dem Parameter **„Alarm zurücksetzen“**:

* **automatisch (Alarm aufgehoben = 0 senden)**: Voreinstellung.
* **nur Alarm senden (Zurücksetzen über KNX)**: das KO sendet nur die 1 und bleibt auf 1, bis es von KNX (z.B. durch eine Person in der Visualisierung) zurückgesetzt wird. Das KO ist dafür beschreibbar.

Gilt für F6-05-00/01/02 (Rauch, Wind, Wasser), D2-06-01 und D2-06-50 (Fenstergriff-Alarm), D2-14-30 (Rauchmelder) sowie Eltako FRWB, FHMB und FWS81.

F6-05-01 (Wasser): Data 0x11, ob Wasser da ist steht im Statusbyte (Eltako FWS81: 0x30 = Wasser, 0x20 = kein Wasser).

## Aktoren (KNX → EnOcean)

* **A5-38-08 Zentralbefehl** (Schalten, Dimmen) über EEP-Profil oder Eltako-Produkt; Eltako-Abweichungen (Lerntelegramm, Dimmgeschwindigkeit, Bestätigungstelegramme als Status) werden berücksichtigt.
* **Eltako-Jalousieaktor** (A5-3F-7F) mit Fahrzeit, Positionsberechnung aus den Bestätigungen und Status Position/Fahrt.
* **FSUD-230V** mit eigenem Lerntelegramm.
* **D2-01** Typ 00..13 (Schalten, Dimmen, Messung und Status je nach Typ), Einlernen per UTE.
* Einlernen in den Aktor über den ETS-Button „Lerntelegramm an Aktor senden“.

## Hinweise für die Weiterentwicklung (Findings)

* **KO-Verknüpfungen (ComObjectRef) nie löschen oder umnummerieren**, sobald eine knxprod damit erzeugt wurde – auch nicht bei Zwischenständen. Fehlt beim ETS-Update eine Verknüpfung, an der eine Gruppenadresse hängt, stürzt die ETS mit einer NullReferenceException ab (RfMulti.ResetGrpFlagReadyDeviceHandler). Nicht mehr genutzte Verknüpfungen bleiben deshalb definiert.
* **ApplicationVersion erhöhen = ReplacesVersions erweitern**: die Updatefähigkeit soll immer erhalten bleiben; Ausnahmen werden bewusst entschieden.
* **Applikationsnummer**: ETS und Firmware vergleichen OpenKNX-ID, Applikationsnummer und -version (PropId 78). Abweichungen führen zu „Firmware und Applikation passen nicht zusammen“; nach Änderungen immer knxprod und Firmware gemeinsam neu erzeugen.
* **Profilnummern in ETS und Firmware müssen übereinstimmen** (EnoceanProfils.h gegen die ParameterTypes in EnoceanModule.share.xml). Bei A5-09 waren sie verschieden, dadurch funktionierte das Profil nie.
* **when-Werte der ETS-Zweige** müssen zu den Produktwerten passen (FHD60SB hing an Wert 0 statt 1, deshalb fehlte das KO).
* **Unions**: neue Parameter möglichst in Unions ablegen, die das jeweilige Profil nicht nutzt; Parameter mit eigener Speicheradresse brauchen die ID-Form `_P-`, Union-Parameter `_UP-`.
* **Offene Punkte**:
  * Eltako FABH130 (Bewegungsmelder, F6-02-01) ist auswählbar, aber noch nicht ausgewertet.
  * „Alarm Energy Low“ (F6-05) wird nur gesetzt, nie zurückgesetzt.
  * A5-30-03 ist nur über die Eltako-Melder umgesetzt, nicht über das EEP-Profil.
  * Zuordnung der physischen Tasten zu „1. ID“ / „2. ID“ bei den 6- und 8-fach-Sendern ist nicht am Gerät geprüft.
  * Repeater-Filter (250 ms) mit echten Repeatern prüfen.
