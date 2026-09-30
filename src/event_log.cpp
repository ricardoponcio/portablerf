#include "event_log.h"

void EventLog::add(RadioId radio, bool tx, bool ok, const uint8_t* data, size_t len, int rssi, float snr) {
    RadioEvent& e = events_[nextId_ % EVENT_LOG_SIZE];
    e.id = nextId_++;
    e.ms = millis();
    e.radio = radio;
    e.tx = tx;
    e.ok = ok;
    e.rssi = rssi;
    e.snr = snr;
    if (len > MAX_MSG_LEN) len = MAX_MSG_LEN;
    // Recepção pode trazer lixo binário: troca não imprimíveis por '.'
    for (size_t i = 0; i < len; i++) e.msg[i] = isprint(data[i]) ? data[i] : '.';
    e.msg[len] = '\0';

    Serial.printf("[%s %s] %s \"%s\" rssi=%d\n", radio == RadioId::LoRa ? "LoRa" : "CC1101",
                  tx ? "TX" : "RX", ok ? "ok" : "FALHA", e.msg, rssi);
}

uint32_t EventLog::firstId() const {
    return nextId_ > EVENT_LOG_SIZE ? nextId_ - EVENT_LOG_SIZE : 1;
}
