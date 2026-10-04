#pragma once
#include <RadioLib.h>
#include "radio.h"
#include "event_log.h"
#include "sniffer.h"

// Parâmetros LoRa que a página pode trocar em tempo de execução.
struct LoRaConfig {
    float freqMhz;
    float bwKhz;
    uint8_t sf;
    uint8_t cr;         // denominador da taxa de código: 5 = 4/5 ... 8 = 4/8
    uint8_t syncWord;   // 0x12 privada, 0x34 LoRaWAN, 0x2B Meshtastic
    uint16_t preamble;
};

// SX1262 (módulo 900M22S) em LoRa, recepção por interrupção no DIO1.
class LoRaRadio : public Radio {
public:
    LoRaRadio(EventLog& log, Sniffer& sniffer, const LoRaConfig& cfg, float tcxoVoltage);

    // Chamar depois de SpiBus::begin(). O begin() do SX1262 pode travar ~20s se o
    // chip não responder (timeouts do BUSY), então roda numa task separada.
    void beginAsync();

    RadioId id() const override { return RadioId::LoRa; }
    const char* name() const override { return "lora"; }
    float frequencyMhz() const override { return cfg_.freqMhz; }
    int setFrequencyMhz(float mhz) override;
    bool initPending() const override { return !initDone_; }
    int initState() const override { return state_; }

    const LoRaConfig& config() const { return cfg_; }
    // Aplica e volta para RX. Se o rádio recusar algum valor, restaura a config anterior.
    int setConfig(const LoRaConfig& cfg);

    using Radio::send;
    int send(const uint8_t* data, size_t len) override;
    void poll() override;  // também alimenta o Sniffer com o RSSI instantâneo

private:
    static void initTask(void* self);
    static void IRAM_ATTR onDio1();

    int apply(const LoRaConfig& cfg);  // chamar com o SPI travado
    PacketProto proto() const;

    // Setado pela interrupção do DIO1 (pacote recebido ou fim de TX)
    static volatile bool dio1Flag_;

    EventLog& log_;
    Sniffer& sniffer_;
    LoRaConfig cfg_;
    float tcxoVoltage_;
    SX1262 radio_;
    volatile bool initDone_ = false;
    volatile int state_ = RADIO_ERR_NOT_READY;
};
