### Funksignal-Diagnose

Gibt für jedes empfangene Telegramm des Geräts die Funkqualität auf zwei KOs aus. Hilft bei der Inbetriebnahme, z.B. um den besten Platz für Gateway, Antenne oder Repeater zu finden.

- **Signalstärke (dBm)** (DPT 6.010): Empfangspegel als negativer Wert, z.B. -65. Richtwerte: besser als -70 dBm gut, -70 bis -85 dBm ausreichend, schlechter als -85 dBm kritisch.
- **Repeater-Stufe** (DPT 5.010): 0 = direkt empfangen, 1 oder 2 = über einen bzw. zwei Repeater.

Einstellungen:

- **aus:** keine Ausgabe, die KOs werden nicht angezeigt.
- **bei jedem Telegramm senden:** beide KOs werden bei jedem Telegramm des Geräts gesendet.
- **nur bei Änderung senden:** nur wenn sich die Signalstärke um mindestens 3 dBm oder die Repeater-Stufe ändert.

Unabhängig davon zeigt der Konsolenbefehl `eno signal` für alle Kanäle die zuletzt empfangene Signalstärke, die Repeater-Stufe und das Alter des letzten Telegramms.
