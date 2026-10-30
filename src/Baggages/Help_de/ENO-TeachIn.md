### Teach-In

Die ETS muss online mit dem Gateway verbunden sein. Die Applikation muss bereits einmal heruntergeladen worden sein.

**Ablauf**

1. **Teach-In starten:** Das Gateway öffnet den Lernmodus für diesen Kanal. Im TCM 615 wird dieselbe Kanalnummer verwendet. Das Zeitfenster ist unter Allgemein → Teach-In Übersicht einstellbar (Standard 60 s).
2. Innerhalb des Zeitfensters die Lerntaste am Gerät drücken. Bei Tastern ohne Lerntaste genügt ein Tastendruck.
3. **Ergebnis übernehmen:** Status, eingelernte ID und erkanntes Profil werden aus dem Gateway gelesen und in den Kanal geschrieben.
4. Applikation herunterladen. Damit steht die ID fest im ETS-Projekt.

Bis zum Download arbeitet der Kanal bereits mit der im Gateway gespeicherten ID.

**Was als Lerntelegramm erkannt wird**

- 4BS- und 1BS-Geräte: Telegramm mit gesetztem Lern-Bit
- VLD-Geräte: UTE-Teach-In-Anfrage
- Taster ohne Lerntelegramm: erstes Telegramm eines noch unbekannten Senders
- Verschlüsselte Geräte: Secure Teach-In, der Transceiver übernimmt den Schlüssel

Ein Telegramm von einer ID, die schon einem anderen Kanal zugeordnet ist, wird nicht eingelernt.

**Schutz vor Überschreiben**

Ein eingelernter Kanal kann nicht erneut eingelernt werden. Soll ein anderes Gerät in diesen Kanal, zuerst **Einlernung löschen**. Das entfernt die ID im Gateway, bei verschlüsselten Geräten auch den Schlüssel im Transceiver, und setzt die ID-Felder auf 0.

**Status**

- Nicht eingelernt
- Teach-In läuft (mit Restzeit)
- Eingelernt: ID (bei verschlüsselten Geräten mit Hinweis)
- Fehler beim Teach-In, z.B. Secure-Link-Tabelle voll oder Gerät braucht einen Pre-Shared Key (wird noch nicht unterstützt)

**Erkanntes Profil**

4BS-Lerntelegramme mit Profilangabe und UTE enthalten das EEP-Profil, es wird angezeigt. Stimmt der Telegrammtyp nicht mit dem im Kanal eingestellten Profil überein, erscheint ein Hinweis. Das Profil wird nicht automatisch geändert.
