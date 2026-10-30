### Base-ID und Transceiver

Die Base-ID ist der Startwert der Sende-IDs des Gateways. Jeder Kanal sendet mit der ID **Base-ID + Kanalnummer − 1** (Kanal 1 = Base-ID, Kanal 2 = Base-ID + 1 usw.). EnOcean-Aktoren lernen diese Sende-ID ein. Ändert sich die Base-ID, reagieren die Aktoren nicht mehr auf das Gateway.

Jeder Transceiver (TCM 615) hat ab Werk eine eigene Base-ID. Damit ein Hardwaretausch die eingelernten Aktoren nicht unbrauchbar macht, wird die Base-ID in der ETS gespeichert:

1. ETS online mit dem Gateway verbinden.
2. **Base-ID und Transceiver-Info aus dem Gateway lesen** drücken. Die Base-ID wird in die Felder übernommen, **Base-ID aus der ETS in den Transceiver schreiben** wird eingeschaltet und die Zeile **Transceiver** zeigt EURID, Firmware und verbleibende Base-ID-Schreibzyklen.
3. Applikation herunterladen.

Bei jedem Start vergleicht das Gateway die Base-ID im Transceiver mit der aus der ETS. Weichen sie ab (z.B. nach einem Hardwaretausch), schreibt es die Base-ID aus der ETS in den Transceiver.

**Zulässiger Bereich:** FF800000 bis FFFFFF80, die unteren 7 Bit müssen 0 sein (128 Sende-IDs).

**Hinweis:** Beim TCM 615 kann die Base-ID beliebig oft geändert werden. Bei TCM 3xx/4xx ist das nur 10-mal möglich.
