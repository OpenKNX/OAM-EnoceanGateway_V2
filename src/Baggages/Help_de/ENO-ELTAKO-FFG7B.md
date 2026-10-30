### Header 1

In der linken oberen Ecke des Gehäuses befindet sich eine Öffnung, über die ein Mikro taster zugänglich ist. Dieser dient zur Navigation durch die im Folgenden beschriebene Menüstruktur.

Hauptmenü:
Aufrufen: Taste ca. 1 Sekunde gedrückt halten, dann loslassen LED-Bestätigung: 1× blinken

*Untermenü 1: Verschlüsselung*
Aufrufen: Taste 1× kurz drücken Ɓ 1 × blinken
Optionen:
1 × drücken Ɓ Verschlüsselung aus (voreingestellt) Ɓ LED: 1 × kurz + 1 × lang
2 × drücken Ɓ Verschlüsselung ein Ɓ
LED: 2 × kurz + 1× lang

*Untermenü 2: Protokoll (EEP)*
Aufrufen: Taste 2 × kurz drücken Ɓ
2 × blinken
Optionen:
1 × drücken Ɓ 'EEP A5-14-09' (voreingestellt)
Ɓ LED: 1 × kurz + 1× lang
2 × drücken Ɓ 'EEP F6-10-00' (sendet nicht
zyklisch). Ɓ LED: 2 × kurz + 1× lang
3 × drücken Ɓ 'EEP F6-10-00' (sendet zyklisch
alle 15min). Ɓ LED: 3 × kurz + 1× lang.

Telegramme nach EEP: A5-14-09
Lerntelegramm: 0x50480D80
Datentelegramme:
Data_byte0 = 0x08 = Fenster geschlossen
0x0E = Fenster offen
0x0A = Fenster gekippt
Data_byte3 = Batteriespannung:
0..250, 0..5 V

Telegramme nach EEP: F6-10-00
'Lerntelegramm': 0xF0
ORG = 0x05
Data_byte3 = 0xF0 = Fenster geschlossen
0xE0 = Fenster offen
0xD0 = Fenster gekippt

