#pragma once
#include <Arduino.h>
#include "config.h"
#include "radio.h"

struct RadioEvent {
    uint32_t id;
    uint32_t ms;       // millis() do evento
    RadioId radio;
    bool tx;           // false = recepção
    bool ok;           // TX: enviou; RX: CRC ok
    int rssi;          // só RX
    float snr;         // só RX do LoRa
    char msg[MAX_MSG_LEN + 1];
};

// Buffer circular com os últimos pacotes enviados/recebidos. Os ids são
// crescentes, então quem lê pede "tudo depois do id X" (ver WebUi /api/log).
// Não é thread-safe: usar só a partir do loop().
class EventLog {
public:
    void add(RadioId radio, bool tx, bool ok, const uint8_t* data, size_t len, int rssi = 0, float snr = 0);

    // Faixa de ids disponíveis: [firstId(), nextId())
    uint32_t firstId() const;
    uint32_t nextId() const { return nextId_; }
    const RadioEvent& get(uint32_t id) const { return events_[id % EVENT_LOG_SIZE]; }

private:
    RadioEvent events_[EVENT_LOG_SIZE];
    uint32_t nextId_ = 1;
};
