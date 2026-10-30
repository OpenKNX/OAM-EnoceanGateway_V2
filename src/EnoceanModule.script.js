// ETS-Script des EnOcean-Moduls: Teach-In und Transceiver-Info über FunctionProperty 160/7
// (Firmware: EnoceanModule::processFunctionProperty, Befehle ENO_FP_CMD_*).

var ENO_FP_OBJECT = 160;
var ENO_FP_PROPERTY = 7;

function ENO_hex2(value) {
    return ("0" + (value & 0xFF).toString(16).toUpperCase()).slice(-2);
}

function ENO_idString(resp, offset) {
    return ENO_hex2(resp[offset]) + ENO_hex2(resp[offset + 1]) + ENO_hex2(resp[offset + 2]) + ENO_hex2(resp[offset + 3]);
}

function ENO_eepString(rorg, func, type) {
    if (rorg == 0)
        return "verschlüsselt, ohne Profil";
    if (func == 0 && type == 0)
        return "R-ORG " + ENO_hex2(rorg) + ", ohne Profil";
    return ENO_hex2(rorg) + "-" + ENO_hex2(func) + "-" + ENO_hex2(type);
}

function ENO_causeText(cause) {
    switch (cause) {
        case 0x00: return "Secure-Link-Tabelle im TCM 615 ist voll";
        case 0x04: return "Teach-In-Telegramm fehlerhaft, bitte wiederholen";
        case 0x05:
        case 0x06: return "Gerät braucht PSK (noch nicht unterstützt)";
        default: return "Ursache " + cause;
    }
}

// Ruft die Firmware auf und prüft die allgemeinen Fehler.
function ENO_call(online, data) {
    var resp = online.invokeFunctionProperty(ENO_FP_OBJECT, ENO_FP_PROPERTY, data);
    if (resp[0] == 1)
        throw new Error("Das Gateway ist nicht konfiguriert. Bitte zuerst die Applikation herunterladen.");
    if (resp[0] == 4)
        throw new Error("Ungültige Kanalnummer.");
    if (resp[0] == 6)
        throw new Error("Die Firmware im Gateway kennt diesen Befehl nicht. Bitte die Firmware aktualisieren.");
    return resp;
}

function ENO_param(device, name) {
    return device.getParameterByName(name);
}

// Schreibt einen Text in ein Textfeld und kürzt ihn auf dessen Größe in Byte (UTF-8, Umlaute zählen doppelt).
// Die ETS bricht das Script sonst mit "The maximum byte length cannot exceed" ab.
function ENO_setText(device, name, text, maxBytes) {
    var bytes = 0;
    for (var i = 0; i < text.length; i++) {
        var c = text.charCodeAt(i);
        var len = c < 0x80 ? 1 : (c < 0x800 ? 2 : 3);
        if (bytes + len > maxBytes) {
            text = text.substring(0, i);
            break;
        }
        bytes += len;
    }
    ENO_param(device, name).value = text;
}

// Schreibt eine 4-Byte-ID in die 8 Nibble-Parameter des Kanals (ENO_CHnId0..7).
function ENO_setChannelId(device, channel, resp, offset) {
    for (var i = 0; i < 4; i++) {
        var b = resp ? resp[offset + i] : 0;
        ENO_param(device, "ENO_CH" + channel + "Id" + (2 * i)).value = (b >> 4) & 0x0F;
        ENO_param(device, "ENO_CH" + channel + "Id" + (2 * i + 1)).value = b & 0x0F;
    }
}

// Status eines Kanals als Text für die ETS (Antwort von ENO_FP_CMD_STATUS).
function ENO_statusText(resp) {
    switch (resp[1]) {
        case 0: return "Nicht eingelernt";
        case 1: return "Teach-In läuft, noch " + resp[9] + " s: Lerntaste am Gerät drücken";
        case 2: return "Eingelernt: " + ENO_idString(resp, 2) + ((resp[9] & 0x01) ? " (verschlüsselt)" : "");
        case 3: return "Fehler beim Teach-In: " + ENO_causeText(resp[9]);
        default: return "Unbekannter Status " + resp[1];
    }
}

// Überträgt den Status eines Kanals (Antwort von ENO_FP_CMD_STATUS) in die ETS: Statustext und, wenn eingelernt,
// ID und erkanntes Profil. Wird von allen Buttons verwendet, damit ID-Felder und Status immer zusammenpassen.
function ENO_applyStatus(device, ch, resp) {
    ENO_setText(device, "ENO_CH" + ch + "TeachStatus", ENO_statusText(resp), 80);
    if (resp[1] != 2)
        return;

    ENO_setChannelId(device, ch, resp, 2);
    var eep = ENO_eepString(resp[6], resp[7], resp[8]);
    // Hinweis, wenn das Gerät einen anderen Telegrammtyp sendet als im Kanal eingestellt (nur EEP-Profil)
    var productsel = ENO_param(device, "ENO_CH" + ch + "Productsel").value;
    var profile = ENO_param(device, "ENO_CH" + ch + "ProfilSelection").value;
    var expected = (profile == 250) ? 0xF6 : profile; // Rocker = RPS
    if (productsel == 1 && resp[6] != 0 && expected != resp[6])
        eep += ", Kanal: R-ORG " + ENO_hex2(expected);
    ENO_setText(device, "ENO_CH" + ch + "TeachEep", eep, 40);
}

function ENO_teachStart(device, online, progress, context) {
    var ch = context.Channel;
    progress.setText("Kanal " + ch + ": Teach-In wird gestartet...");
    online.connect();
    try {
        var resp = ENO_call(online, [2, ch]);
        if (resp[0] == 2) {
            // bereits eingelernt: ID und Status trotzdem in die ETS übernehmen
            ENO_applyStatus(device, ch, ENO_call(online, [1, ch]));
            throw new Error("Kanal " + ch + " ist bereits mit der ID " + ENO_idString(resp, 1) + " eingelernt. Erst \"Einlernung löschen\" ausführen, wenn ein anderes Gerät eingelernt werden soll.");
        }
        if (resp[0] == 3)
            throw new Error("Für Kanal " + resp[1] + " läuft gerade ein Teach-In. Bitte warten, bis es beendet ist.");
        if (resp[0] != 0)
            throw new Error("Kanal " + ch + ": Teach-In konnte nicht gestartet werden (Code " + resp[0] + ").");

        ENO_setText(device, "ENO_CH" + ch + "TeachStatus", "Teach-In läuft: Lerntaste am Gerät drücken, dann \"Ergebnis übernehmen\"", 80);
        progress.setText("Kanal " + ch + ": Teach-In läuft. Jetzt die Lerntaste am Gerät drücken, danach \"Ergebnis übernehmen\".");
    } finally {
        online.disconnect();
    }
}

function ENO_teachRead(device, online, progress, context) {
    var ch = context.Channel;
    progress.setText("Kanal " + ch + ": Ergebnis wird gelesen...");
    online.connect();
    try {
        var resp = ENO_call(online, [1, ch]);
        var status = ENO_statusText(resp);
        ENO_applyStatus(device, ch, resp);

        if (resp[1] == 2) {
            progress.setText("Kanal " + ch + ": " + status + ". Die ID wird mit dem nächsten Download übernommen.");
        } else {
            progress.setText("Kanal " + ch + ": " + status);
        }
    } finally {
        online.disconnect();
    }
}

function ENO_teachDelete(device, online, progress, context) {
    var ch = context.Channel;
    progress.setText("Kanal " + ch + ": Einlernung wird gelöscht...");
    online.connect();
    try {
        var resp = ENO_call(online, [3, ch]);
        if (resp[0] != 0)
            throw new Error("Kanal " + ch + ": Einlernung konnte nicht gelöscht werden (Code " + resp[0] + ").");
        ENO_setChannelId(device, ch, null, 0);
        ENO_setText(device, "ENO_CH" + ch + "TeachStatus", "Nicht eingelernt", 80);
        ENO_setText(device, "ENO_CH" + ch + "TeachEep", "", 40);
        progress.setText("Kanal " + ch + ": Einlernung gelöscht. Bitte die Applikation herunterladen.");
    } finally {
        online.disconnect();
    }
}

function ENO_sendLearn(device, online, progress, context) {
    var ch = context.Channel;
    progress.setText("Kanal " + ch + ": Lerntelegramm wird gesendet...");
    online.connect();
    try {
        var resp = ENO_call(online, [6, ch]);
        if (resp[0] == 7)
            throw new Error("Kanal " + ch + " sendet nicht an EnOcean. Das Lerntelegramm gibt es nur für den Taster in Richtung KNX nach EnOcean.");
        if (resp[0] != 0)
            throw new Error("Kanal " + ch + ": Lerntelegramm konnte nicht gesendet werden (Code " + resp[0] + ").");
        progress.setText("Kanal " + ch + ": Lerntelegramm gesendet. Den Lernmodus am Aktor jetzt beenden.");
    } finally {
        online.disconnect();
    }
}

function ENO_readTransceiver(device, online, progress, context) {
    progress.setText("Base-ID und Transceiver-Info werden gelesen...");
    online.connect();
    try {
        var resp = ENO_call(online, [4]);
        if (resp[0] != 0)
            throw new Error("Der Transceiver antwortet nicht.");

        // Base-ID muss zu den ETS-Feldern passen: FF, 8..F, beliebig, beliebig, beliebig, beliebig, 0/8, 0
        if (resp[1] != 0xFF || resp[2] < 0x80 || (resp[4] & 0x7F) != 0)
            throw new Error("Die Base-ID " + ENO_idString(resp, 1) + " liegt außerhalb des zulässigen Bereichs FF800000 bis FFFFFF80.");
        for (var i = 0; i < 4; i++) {
            ENO_param(device, "ENO_Id" + (2 * i)).value = (resp[1 + i] >> 4) & 0x0F;
            ENO_param(device, "ENO_Id" + (2 * i + 1)).value = resp[1 + i] & 0x0F;
        }
        // Base-ID beim Start schreiben, damit sie nach einem Hardwaretausch wieder gesetzt wird
        ENO_param(device, "ENO_SetBaseIdFunc").value = 1;

        var info = "EURID " + ENO_idString(resp, 6) + ", Base-ID-Schreibzyklen: " + (resp[5] == 0xFF ? "unbegrenzt" : resp[5]);
        var ver = ENO_call(online, [5]);
        if (ver[0] == 0)
            info += ", Firmware " + ver[1] + "." + ver[2] + "." + ver[3] + "." + ver[4];
        ENO_setText(device, "ENO_TransceiverInfo", info, 80);

        progress.setText("Base-ID " + ENO_idString(resp, 1) + " übernommen. Bitte die Applikation herunterladen.");
    } finally {
        online.disconnect();
    }
}

function ENO_readAllChannels(device, online, progress, context) {
    var count = ENO_param(device, "ENO_VisibleChannels").value;
    var errors = 0;
    online.connect();
    try {
        for (var ch = 1; ch <= count; ch++) {
            progress.setText("Status Kanal " + ch + " von " + count + "...");
            if (ENO_param(device, "ENO_CH" + ch + "Productsel").value == 0) {
                ENO_setText(device, "ENO_CH" + ch + "TeachStatus", "Kanal inaktiv", 80);
            } else if (ENO_param(device, "ENO_CH" + ch + "SetupMode").value == 0) {
                var id = "";
                for (var i = 0; i < 8; i++)
                    id += ENO_param(device, "ENO_CH" + ch + "Id" + i).value.toString(16).toUpperCase();
                ENO_setText(device, "ENO_CH" + ch + "TeachStatus", "ID manuell: " + id, 80);
            } else {
                // Teach-In-Kanal: Status und, wenn eingelernt, auch ID und Profil übernehmen. Ein Fehler bei einem
                // Kanal bricht die Abfrage der übrigen nicht ab.
                try {
                    ENO_applyStatus(device, ch, ENO_call(online, [1, ch]));
                } catch (e) {
                    errors++;
                    ENO_setText(device, "ENO_CH" + ch + "TeachStatus", "Abfrage fehlgeschlagen: " + e.message, 80);
                }
            }
        }
        progress.setText("Status von " + count + " Kanälen gelesen" + (errors ? ", " + errors + " mit Fehler." : "."));
    } finally {
        online.disconnect();
    }
}
