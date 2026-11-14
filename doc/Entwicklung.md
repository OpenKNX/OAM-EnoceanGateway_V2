# Hinweise für die Weiterentwicklung

Erkenntnisse aus der Entwicklung des EnOcean-Gateways, die beim Ändern von Firmware und ETS-Applikation wichtig sind. Die Anleitung für Nutzer steht in der [Applikationsbeschreibung](Applikationsbeschreibung.md).

## Versionen und ETS-Update

* **ApplicationVersion erhöhen = ReplacesVersions erweitern.** Die Updatefähigkeit soll immer erhalten bleiben, Ausnahmen werden bewusst entschieden.
* **Nicht updatefähige Version:** `ReplacesVersions="0"` setzen (eine Version 0.0 gab es nie). Ein leeres `ReplacesVersions=""` lehnt der OpenKNXproducer ab, das Attribut ist Pflicht.
* **Firmware und Applikation müssen zusammenpassen:** Die ETS vergleicht OpenKNX-ID, Applikationsnummer und -version (PropId 78). Abweichungen führen zu „Firmware und Applikation passen nicht zusammen“. Nach Änderungen immer knxprod und Firmware gemeinsam neu erzeugen.
* **Gleiche Version, anderer Inhalt:** Hat eine neue Firmware dieselbe Applikationsnummer und -version wie die im Gerät gespeicherte Konfiguration, lädt OpenKNX die alte Konfiguration als gültig. Ändern sich dabei KO-Anzahl oder Parameterlayout, greift die Firmware auf KOs hinter dem Tabellenende zu (der KNX-Stack prüft keine Grenzen) und stürzt ab. Deshalb: bei Layoutänderungen immer die Version erhöhen. Als Schutz entlädt `main.cpp` beim Start die KO-Tabelle, wenn sie weniger Einträge hat als `MAIN_MaxKoNumber`; das Gerät startet dann unkonfiguriert.
* **KO-Verknüpfungen (ComObjectRef) nie löschen oder umnummerieren**, sobald eine knxprod damit erzeugt wurde – auch nicht bei Zwischenständen. Fehlt beim ETS-Update eine Verknüpfung, an der eine Gruppenadresse hängt, stürzt die ETS mit einer NullReferenceException ab (RfMulti.ResetGrpFlagReadyDeviceHandler). Nicht mehr genutzte Verknüpfungen bleiben definiert.
* **KO-Anzahl je Kanal:** Der Producer bildet die ETS-IDs der KO-Verknüpfungen aus der KO-Nummer (`O-21_R-...`). Eine Änderung der KO-Anzahl je Kanal ändert alle IDs ab Kanal 2 und ist damit immer ein Update-Bruch.

## Build

* **Profilnummern** (A5_09_04 usw.) erzeugt `scripts/generate_profiles.py` vor jedem Build aus den Profillisten in `EnoceanModule.share.xml` nach `include/EnoceanProfilsGenerated.h`. Nicht von Hand ändern; nur Sonderfälle stehen in `EnoceanProfils.h`. Bei A5-09 waren ETS und Firmware früher verschieden, dadurch funktionierte das Profil nie.
* **ETS-Prüfung im Release-Build:** `scripts/check_ets_ui.py` prüft die erzeugte Applikation (when-Werte gegen Parametertypen, doppelte KOs, KOs aller Einzeltasten in 20.000 zufälligen Taster-Konfigurationen) und bricht den Release ab. Anlass: FHD60SB hing an Wert 0 statt 1, deshalb fehlte das KO.
* **Baggages beim Release:** `Build-Release.ps1` verschiebt `src/EnoceanGateway.baggages` nach `release/data`. Danach mit `git restore src/EnoceanGateway.baggages` zurückholen bzw. den Producer erneut laufen lassen (er legt den Ordner für die aktuelle Version an, z.B. `A2/00/02`).

## ETS-Applikation

* **Hilfetexte:** je Thema eine Datei `src/Baggages/Help_de/ENO-*.md`, eingebunden über `HelpContext` am Parameter. Neue Parameter immer mit HelpContext anlegen.
* **Taster-Blöcke im Template** sind mehrfach vorhanden (EEP-Pfad und Eltako mit unterschiedlicher Einrückung und Texten). `op:include` kann das nicht parametrieren; Änderungen an allen Stellen machen und mit `check_ets_ui.py` prüfen.
* **Unions:** neue Parameter möglichst in Unions ablegen, die das jeweilige Profil nicht nutzt. Parameter mit eigener Speicheradresse brauchen die ID-Form `_P-`, Union-Parameter `_UP-`.
* **Eltako-Pfad nie an EEP-Parameter koppeln:** Parameter des EEP-Pfads (z.B. „1BS-Profil“) sind bei Eltako-Kanälen ausgeblendet und haben dort keinen verlässlichen Wert. Gemeinsame Auswertungen als eigene Funktion ohne EEP-Parameter schreiben (z.B. `handle1BS_D5_00_01`).

## Offene Punkte

* F6-02-04 (Wippentaster ERP2) und F6-03-01/02 (4 Wippen) sind nicht umgesetzt.
* A5-10-18..23, A5-12-04/05/10, A5-13-03..06 (Datum, Zeit, Position) sind nicht umgesetzt. Multisensoren FMMS/FMS und FBH65TF fehlen.
* Eltako FABH130 (Bewegungsmelder, F6-02-01) ist auswählbar, aber noch nicht ausgewertet.
* „Alarm Energy Low“ (F6-05) wird nur gesetzt, nie zurückgesetzt.
* A5-30-03 ist nur über die Eltako-Melder umgesetzt, nicht über das EEP-Profil.
* Nicht am Gerät geprüft: Zuordnung der Tasten zu „1. ID“ / „2. ID“ bei den 6- und 8-fach-Sendern, Repeater-Filter (250 ms), Soll-Feuchte des FUTH (Skalierung), F6-02-03 ohne Loslass-Telegramm.
