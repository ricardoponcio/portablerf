#include "event_log.h"

void EventLog::add(RadioId radio, bool tx, bool ok, const uint8_t* data, size_t len, int rssi, float snr,
                   PacketProto proto) {
    RadioEvent& e = events_[nextId_ % EVENT_LOG_SIZE];
    e.id = nextId_++;
    e.ms = millis();
    e.radio = radio;
    e.tx = tx;
    e.ok = ok;
    e.rssi = rssi;
    e.snr = snr;
    e.proto = proto;
    if (len > EVENT_MAX_BYTES) len = EVENT_MAX_BYTES;
    e.len = len;
    memcpy(e.data, data, len);

    // Recepção pode trazer lixo binário: na serial troca não imprimíveis por '.'
    char text[MAX_MSG_LEN + 1];
    size_t n = min(len, (size_t)MAX_MSG_LEN);
    for (size_t i = 0; i < n; i++) text[i] = isprint(data[i]) ? data[i] : '.';
    text[n] = '\0';
    Serial.printf("[%s %s] %s \"%s\"%s (%u bytes) rssi=%d\n", radio == RadioId::LoRa ? "LoRa" : "CC1101",
                  tx ? "TX" : "RX", ok ? "ok" : "FALHA", text, len > n ? "..." : "", (unsigned)len, rssi);
}

uint32_t EventLog::firstId() const {
    return nextId_ > EVENT_LOG_SIZE ? nextId_ - EVENT_LOG_SIZE : 1;
}
