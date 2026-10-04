#include "web_ui.h"
#include <WiFi.h>
#include "web_page.h"

void WebUi::begin() {
    server_.on("/", [this] { handleRoot(); });
    server_.on("/api/status", [this] { handleStatus(); });
    server_.on("/api/send", HTTP_POST, [this] { handleSend(); });
    server_.on("/api/auto", HTTP_POST, [this] { handleAuto(); });
    server_.on("/api/freq", HTTP_POST, [this] { handleFreq(); });
    server_.on("/api/lora", HTTP_POST, [this] { handleLora(); });
    server_.on("/api/mesh", HTTP_POST, [this] { handleMesh(); });
    server_.on("/api/log", [this] { handleLog(); });
    server_.on("/api/sniff", [this] { handleSniff(); });
    server_.begin();
    Serial.println("Servidor HTTP iniciado.");
}

void WebUi::handle() {
    server_.handleClient();
}

void WebUi::handleRoot() {
    server_.send_P(200, "text/html", WEB_PAGE_HTML);
}

void WebUi::handleStatus() {
    String json = "{";
    json += "\"ssid\":\"" + jsonEscape(WiFi.SSID().c_str()) + "\",";
    json += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
    json += "\"wifi_rssi\":" + String(WiFi.RSSI()) + ",";
    json += "\"radios\":{";
    for (size_t i = 0; i < 2; i++) {
        const Radio& r = channels_[i].radio;
        if (i) json += ",";
        json += "\"" + String(r.name()) + "\":{";
        json += "\"pending\":" + String(r.initPending() ? "true" : "false");
        json += ",\"ok\":" + String(r.ready() ? "true" : "false");
        json += ",\"state\":" + String(r.initState());
        json += ",\"freq\":" + String(r.frequencyMhz(), 2);
        json += ",\"auto_ms\":" + String(channels_[i].autoPing.intervalMs());
        if (&r == &lora_) json += ",\"cfg\":" + loraConfigJson();
        json += "}";
    }
    json += "},\"mesh\":" + meshChannelsJson() + "}";
    server_.send(200, "application/json", json);
}

void WebUi::handleSend() {
    RadioChannel* ch = channelArg();
    if (!ch) return;
    String msg = server_.arg("msg");
    if (msg.length() == 0) {
        server_.send(400, "application/json", "{\"ok\":false,\"err\":\"mensagem vazia\"}");
        return;
    }
    int state = ch->radio.send(msg);
    server_.send(200, "application/json",
                 "{\"ok\":" + String(state == RADIO_OK ? "true" : "false") + ",\"state\":" + String(state) + "}");
}

void WebUi::handleAuto() {
    RadioChannel* ch = channelArg();
    if (!ch) return;
    ch->autoPing.setIntervalMs(server_.arg("ms").toInt());
    server_.send(200, "application/json", "{\"ok\":true}");
}

void WebUi::handleFreq() {
    RadioChannel* ch = channelArg();
    if (!ch) return;
    int state = ch->radio.setFrequencyMhz(server_.arg("mhz").toFloat());
    server_.send(200, "application/json",
                 "{\"ok\":" + String(state == RADIO_OK ? "true" : "false") + ",\"state\":" + String(state) +
                 ",\"freq\":" + String(ch->radio.frequencyMhz(), 2) + "}");
}

// Parâmetros ausentes mantêm o valor atual; sync vem em hex ("34", "2B")
void WebUi::handleLora() {
    LoRaConfig c = lora_.config();
    if (server_.hasArg("freq")) c.freqMhz = server_.arg("freq").toFloat();
    if (server_.hasArg("bw")) c.bwKhz = server_.arg("bw").toFloat();
    if (server_.hasArg("sf")) c.sf = server_.arg("sf").toInt();
    if (server_.hasArg("cr")) c.cr = server_.arg("cr").toInt();
    if (server_.hasArg("sync")) c.syncWord = strtoul(server_.arg("sync").c_str(), nullptr, 16);
    if (server_.hasArg("pre")) c.preamble = server_.arg("pre").toInt();
    int state = lora_.setConfig(c);
    server_.send(200, "application/json",
                 "{\"ok\":" + String(state == RADIO_OK ? "true" : "false") + ",\"state\":" + String(state) +
                 ",\"cfg\":" + loraConfigJson() + "}");
}

void WebUi::handleMesh() {
    String names[MeshDecoder::MAX_CHANNELS], psks[MeshDecoder::MAX_CHANNELS];
    size_t n = 0;
    for (size_t i = 0; i < MeshDecoder::MAX_CHANNELS; i++) {
        String name = server_.arg("n" + String(i)), psk = server_.arg("k" + String(i));
        name.trim();
        psk.trim();
        if (name.length() == 0 && psk.length() == 0) continue;
        names[n] = name;
        psks[n] = psk;
        n++;
    }
    bool ok = mesh_.setChannels(names, psks, n);
    server_.send(200, "application/json",
                 "{\"ok\":" + String(ok ? "true" : "false") + ",\"mesh\":" + meshChannelsJson() + "}");
}

String WebUi::meshChannelsJson() const {
    String json = "[";
    for (size_t i = 0; i < mesh_.count(); i++) {
        const MeshChannel& ch = mesh_.channel(i);
        if (i) json += ",";
        json += "{\"name\":\"" + jsonEscape(ch.name.c_str()) + "\",\"psk\":\"" + jsonEscape(ch.psk.c_str()) +
                "\",\"hash\":" + String(ch.hash) + "}";
    }
    return json + "]";
}

String WebUi::loraConfigJson() const {
    const LoRaConfig& c = lora_.config();
    char buf[128];
    snprintf(buf, sizeof(buf), "{\"freq\":%.3f,\"bw\":%.1f,\"sf\":%u,\"cr\":%u,\"sync\":%u,\"pre\":%u}",
             c.freqMhz, c.bwKhz, c.sf, c.cr, c.syncWord, c.preamble);
    return buf;
}

void WebUi::handleLog() {
    uint32_t since = server_.arg("since").toInt();
    uint32_t first = max(log_.firstId(), since + 1);
    // "next" deixa a página perceber que a placa reiniciou (next menor que o último id visto)
    String json = "{\"next\":" + String(log_.nextId()) + ",\"events\":[";
    for (uint32_t id = first; id < log_.nextId(); id++) {
        const RadioEvent& e = log_.get(id);
        if (id != first) json += ",";
        json += "{\"id\":" + String(e.id) + ",\"ms\":" + String(e.ms);
        json += ",\"radio\":\"" + String(e.radio == RadioId::LoRa ? "lora" : "cc1101") + "\"";
        json += ",\"tx\":" + String(e.tx ? "true" : "false");
        json += ",\"ok\":" + String(e.ok ? "true" : "false");
        json += ",\"rssi\":" + String(e.rssi) + ",\"snr\":" + String(e.snr, 1);
        // Texto (não imprimíveis viram '.') para mensagens, hex para pacotes binários
        char text[EVENT_MAX_BYTES + 1];
        char hex[2 * EVENT_MAX_BYTES + 1];
        for (size_t i = 0; i < e.len; i++) {
            text[i] = isprint(e.data[i]) ? e.data[i] : '.';
            sprintf(hex + 2 * i, "%02X", e.data[i]);
        }
        text[e.len] = '\0';
        hex[2 * e.len] = '\0';
        static const char* const protos[] = {"", "lorawan", "meshtastic"};
        json += ",\"proto\":\"" + String(protos[(int)e.proto]) + "\"";
        json += ",\"msg\":\"" + jsonEscape(text) + "\",\"hex\":\"" + String(hex) + "\"";
        if (e.proto == PacketProto::Meshtastic && !e.tx) {
            uint8_t plain[EVENT_MAX_BYTES];
            int ch = mesh_.decrypt(e.data, e.len, plain);
            if (ch >= 0) {
                size_t n = e.len - MeshDecoder::HEADER_LEN;
                for (size_t i = 0; i < n; i++) sprintf(hex + 2 * i, "%02X", plain[i]);
                hex[2 * n] = '\0';
                json += ",\"chan\":\"" + jsonEscape(mesh_.channel(ch).name.c_str()) + "\",\"plain\":\"" + String(hex) + "\"";
            }
        }
        json += "}";
    }
    json += "]}";
    server_.send(200, "application/json", json);
}

// "next" deixa a página perceber que a placa reiniciou (next menor que o último id visto)
void WebUi::handleSniff() {
    RadioChannel* ch = channelArg();
    if (!ch) return;
    const Sniffer& sniffer = ch->sniffer;
    uint32_t since = server_.arg("since").toInt();
    uint32_t first = max(sniffer.firstId(), since + 1);
    String r, f;
    for (uint32_t id = first; id < sniffer.nextId(); id++) {
        const SniffSample& s = sniffer.get(id);
        if (id != first) { r += ','; f += ','; }
        r += (int)s.rssi;
        f += (int)s.flags;
    }
    server_.send(200, "application/json",
                 "{\"next\":" + String(sniffer.nextId()) + ",\"ms\":" + String(SNIFF_BUCKET_MS) +
                 ",\"r\":[" + r + "],\"f\":[" + f + "]}");
}

RadioChannel* WebUi::channelArg() {
    String name = server_.arg("radio");
    for (RadioChannel& ch : channels_) {
        if (name == ch.radio.name()) return &ch;
    }
    server_.send(400, "application/json", "{\"ok\":false,\"err\":\"radio invalido\"}");
    return nullptr;
}

String WebUi::jsonEscape(const char* s) {
    String out;
    for (; *s; s++) {
        if (*s == '"' || *s == '\\') out += '\\';
        if ((uint8_t)*s < 0x20) out += '.';  // EventLog já filtra, mas o SSID não
        else out += *s;
    }
    return out;
}
