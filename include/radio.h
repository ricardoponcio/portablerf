#pragma once
#include <Arduino.h>

// Códigos de retorno comuns aos rádios. O LoRa repassa também os códigos
// do RadioLib (negativos, ver RadioLib/src/TypeDef.h); 0 é sempre sucesso.
constexpr int RADIO_OK             = 0;
constexpr int RADIO_ERR_NOT_FOUND  = -2;     // mesmo valor de RADIOLIB_ERR_CHIP_NOT_FOUND
constexpr int RADIO_ERR_NOT_READY  = -1000;  // ainda inicializando
constexpr int RADIO_ERR_BUS_BUSY   = -1001;  // SPI ocupado pelo outro rádio

enum class RadioId : uint8_t { CC1101, LoRa };

// Interface comum dos rádios: a página web e o auto-ping só enxergam isso.
class Radio {
public:
    virtual ~Radio() = default;

    virtual RadioId id() const = 0;
    virtual const char* name() const = 0;  // identificador usado na API web ("cc1101", "lora")

    virtual bool initPending() const = 0;
    virtual int initState() const = 0;      // RADIO_OK ou código de erro
    bool ready() const { return !initPending() && initState() == RADIO_OK; }

    // Envia e registra no log de eventos. Retorna RADIO_OK ou código de erro.
    virtual int send(const uint8_t* data, size_t len) = 0;
    int send(const String& msg) { return send((const uint8_t*)msg.c_str(), msg.length()); }

    // Verifica se chegou pacote; se sim, registra no log. Chamar no loop().
    virtual void poll() = 0;
};
