#pragma once
#include <RadioLib.h>
#include "radio.h"
#include "event_log.h"

// SX1262 (módulo 900M22S) em LoRa, SF9 / BW125 / CR4:7, recepção por interrupção no DIO1.
class LoRaRadio : public Radio {
public:
    LoRaRadio(EventLog& log, float freqMhz, float tcxoVoltage);

    // Chamar depois de SpiBus::begin(). O begin() do SX1262 pode travar ~20s se o
    // chip não responder (timeouts do BUSY), então roda numa task separada.
    void beginAsync();

    RadioId id() const override { return RadioId::LoRa; }
    const char* name() const override { return "lora"; }
    bool initPending() const override { return !initDone_; }
    int initState() const override { return state_; }

    using Radio::send;
    int send(const uint8_t* data, size_t len) override;
    void poll() override;

private:
    static void initTask(void* self);
    static void IRAM_ATTR onDio1();

    // Setado pela interrupção do DIO1 (pacote recebido ou fim de TX)
    static volatile bool dio1Flag_;

    EventLog& log_;
    float freqMhz_;
    float tcxoVoltage_;
    SX1262 radio_;
    volatile bool initDone_ = false;
    volatile int state_ = RADIO_ERR_NOT_READY;
};
