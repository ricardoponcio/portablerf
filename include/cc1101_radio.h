#pragma once
#include "radio.h"
#include "event_log.h"
#include "sniffer.h"

// CC1101 em modo pacote: 2-FSK, sync word 16/16, CRC ligado.
class Cc1101Radio : public Radio {
public:
    Cc1101Radio(EventLog& log, Sniffer& sniffer, float freqMhz) : log_(log), sniffer_(sniffer), freqMhz_(freqMhz) {}

    // Chamar depois de SpiBus::begin(). Rápido (~200ms), roda no setup().
    void begin();

    RadioId id() const override { return RadioId::CC1101; }
    const char* name() const override { return "cc1101"; }
    float frequencyMhz() const override { return freqMhz_; }
    int setFrequencyMhz(float mhz) override;
    bool initPending() const override { return false; }
    int initState() const override { return state_; }

    using Radio::send;
    int send(const uint8_t* data, size_t len) override;
    void poll() override;  // também alimenta o Sniffer com o RSSI

private:
    // Valores do registrador MARCSTATE (datasheet, tabela 32)
    static constexpr byte MARC_IDLE = 0x01;
    static constexpr byte MARC_RXFIFO_OVERFLOW = 0x11;

    void restartRxIfStuck();

    EventLog& log_;
    Sniffer& sniffer_;
    float freqMhz_;
    int state_ = RADIO_ERR_NOT_READY;
};
