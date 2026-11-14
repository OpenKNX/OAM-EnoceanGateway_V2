# Supported Enocean Devices

Geräte, die in der ETS unter **Hersteller: ELTAKO** direkt ausgewählt werden können. Die Telegramm-Inhalte stammen aus dem Eltako-Dokument „Inhalte der Eltako-Funktelegramme“. Alle anderen Geräte werden über die EEP-Profile eingebunden (Übersicht in der [README](../README.md)).

## Taster

| Layout | Geräte | EEP |
|---|---|---|
| 1 Taste | F1FT65, F1T55E, F1T65, F1T80, FET55E, FKD, FMH1W, FNS55B, FNS55EB, FNS65EB | F6-01-01 |
| 1 Wippe | F2FT65, F2FT65B, F2FZT65B, F2T55E, F2T55EB, F2T65, F2T65B, F2ZT55E, F2ZT65, FHS2, FMH2, FMH2S, FZT55, F4T55B und FT55 mit Einzelwippe | F6-02-01 |
| 2 Wippen | F4FT65, F4FT65B, F4PT, F4PT55, F4T55E, F4T55EB, F4T65, F4T65B, FHS4, FMH4, FMH4S, FT4F, F4T55B und FT55 mit Doppelwippe | F6-02-01 |
| 6-fach (2 IDs) | F6T55B, F6T55EB, F6T65B: „Tasten 1-4 (1. ID)“ und „Tasten 5-6 (2. ID)“ | F6-02-01 |
| 8-fach (2 IDs) | FF8, FMH8: „Tasten 1-4 (1. ID)“ und „Tasten 5-8 (2. ID)“ | F6-02-01 |

F4CT55 wird bewusst nicht angeboten.

## Fenster- und Türkontakte

| Gerät | EEP |
|---|---|
| FFG7B Fenstergriffsensor | A5-14-09 oder F6-10-00 (per Parameter) |
| FFKB, FTK, FTKB Tür-/Fensterkontakt | D5-00-01 (+ 4BS Spannung) |
| FTKE Fenster-/Türkontakt, FFTE Fenster-Tastkontakt | F6-10-00 (auch verschlüsselt) |

## Umweltsensoren

| Gerät | EEP |
|---|---|
| FFT60SB, FTFB, FTFSB, FFT55EB Temperatur/Feuchte | A5-04-02 |
| FLGTF55E Temperatur/Feuchte/Luftgüte | A5-04-02 + A5-09-0C |
| FCO2TF65 Temperatur/Feuchte/CO2 | A5-09-04 |
| FLT58 Luftgüte, Temperatur, Feuchte | A5-04-02 + A5-09-05 |

## Helligkeit

| Gerät | EEP | KOs |
|---|---|---|
| FHD60SB Helligkeits-Dämmerungs-Sensor | A5-06-01 (Eltako) | LUX |

## Bewegungsmelder

| Gerät | EEP |
|---|---|
| FB55EB | A5-07-01 |
| FBHT55ESB | A5-08-01 |
| FABH65S | A5-08-01 (Eltako-Anpassung) |
| FABH130/230V | F6-02-01 – noch nicht ausgewertet |

## Rauch-/Hitzemelder

| Gerät | EEP | KOs |
|---|---|---|
| FRWB Rauchmelder, FHMB Hitzemelder | A5-30-03 | Temperatur, Alarm (mit „Alarm zurücksetzen“) |

## Heizungsstellantriebe

| Gerät | EEP |
|---|---|
| FKS-SV Klein-Stellantrieb Smart Valve | A5-20-01 (bidirektional) |
| FKS-B Funk-Klein-Stellantrieb mit Batterie | A5-20-04 (bidirektional) |

## Wassersensoren

| Gerät | EEP | KOs |
|---|---|---|
| FWS81 Wassersensor | F6-05-01 (Wasser im Statusbyte: 0x30 = Wasser, 0x20 = kein Wasser) | Alarm (mit „Alarm zurücksetzen“) |

## Aktoren (KNX → EnOcean)

| Produkt | Geräte | Telegramm |
|---|---|---|
| Schaltaktor | FSR61/71/14, FMS61/14, FASSA, FASSL, FSSG, FSLA, TF61L/R, FTA65L, F4SR14-LED | A5-38-08 (Eltako) |
| Dimmaktor | FUD61/71/14, FD62, FTA65D, TF-TA65D, FD2G14/71, FDG14, FSG14/71-1-10V | A5-38-08 (Eltako) |
| Jalousie-/Rollladenaktor | FSB14, FSB61, FSB71, FJ62 | A5-3F-7F (Eltako) |
| FSUD-230V Zwischenstecker-Dimmer | FSUD-230V | A5-38-08 mit eigenem Lerntelegramm |
