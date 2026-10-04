#pragma once
#include <Arduino.h>
#include "config.h"

// "Sniff" do CC1101: pico de RSSI a cada SNIFF_BUCKET_MS, sem decodificar nada.
// Sinal acima do piso de ruído = alguém transmitindo na frequência, mesmo que o
// pacote não feche (modulação, taxa, sync word ou CRC diferentes).
constexpr uint8_t SNIFF_RX     = 1;  // pacote recebido com CRC ok nessa janela
constexpr uint8_t SNIFF_TX     = 2;  // pacote enviado nessa janela
constexpr uint8_t SNIFF_BAD    = 4;  // sync word casou mas o CRC falhou

struct SniffSample {
    int8_t rssi;    // pico em dBm na janela
    uint8_t flags;  // SNIFF_*
};

// Buffer circular igual ao EventLog: ids crescentes, quem lê pede "tudo depois do id X".
// Não é thread-safe: usar só a partir do loop().
class Sniffer {
public:
    void add(int rssi);       // chamar a cada leitura de RSSI; guarda o pico da janela
    void mark(uint8_t flag);  // marca a janela atual (SNIFF_*)

    // Faixa de ids disponíveis: [firstId(), nextId())
    uint32_t firstId() const { return nextId_ > SNIFF_SIZE ? nextId_ - SNIFF_SIZE : 1; }
    uint32_t nextId() const { return nextId_; }
    const SniffSample& get(uint32_t id) const { return samples_[id % SNIFF_SIZE]; }

private:
    void commit();

    SniffSample samples_[SNIFF_SIZE];
    uint32_t nextId_ = 1;
    uint32_t bucketStart_ = 0;
    bool open_ = false;
    int peak_ = 0;
    uint8_t flags_ = 0;
};
