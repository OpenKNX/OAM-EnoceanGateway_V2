### EEP-Profil

Das EnOcean Equipment Profile beschreibt, wie ein Gerät seine Daten im Funktelegramm codiert. Es steht in der Anleitung oder auf dem Gerät, z.B. „A5-04-02“:

- erste Stelle (**F6**, **D5**, **A5**, **D2**): Telegrammtyp RPS, 1BS, 4BS, VLD
- zweite Stelle: Funktion (z.B. **04** = Temperatur und Feuchte)
- dritte Stelle: Typ (z.B. **02** = Messbereich)

**RPS - Schalter** ist das Profil für Taster und Wippenschalter (F6-02-xx) mit Funktionen wie Schalten, Dimmen, Jalousie und Szene.

Nach dem Teach-In zeigt **Erkanntes Profil**, was das Gerät beim Einlernen gemeldet hat. Passt der Telegrammtyp nicht zur Auswahl, wird das dort angezeigt.
