#include "web_ui.h"
#include <WiFi.h>
#include "web_page.h"

void WebUi::begin() {
    server_.on("/", [this] { handleRoot(); });
    server_.on("/api/status", [this] { handleStatus(); });
    server_.on("/api/send", HTTP_POST, [this] { handleSend(); });
    server_.on("/api/auto", HTTP_POST, [this] { handleAuto(); });
    server_.on("/api/log", [this] { handleLog(); });
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
        json += ",\"auto_ms\":" + String(channels_[i].autoPing.intervalMs());
        json += "}";
    }
    json += "}}";
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

void WebUi::handleLog() {
    uint32_t since = server_.arg("since").toInt();
    uint32_t first = max(log_.firstId(), since + 1);
    String json = "{\"events\":[";
    for (uint32_t id = first; id < log_.nextId(); id++) {
        const RadioEvent& e = log_.get(id);
        if (id != first) json += ",";
        json += "{\"id\":" + String(e.id) + ",\"ms\":" + String(e.ms);
        json += ",\"radio\":\"" + String(e.radio == RadioId::LoRa ? "lora" : "cc1101") + "\"";
        json += ",\"tx\":" + String(e.tx ? "true" : "false");
        json += ",\"ok\":" + String(e.ok ? "true" : "false");
        json += ",\"rssi\":" + String(e.rssi) + ",\"snr\":" + String(e.snr, 1);
        json += ",\"msg\":\"" + jsonEscape(e.msg) + "\"}";
    }
    json += "]}";
    server_.send(200, "application/json", json);
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
