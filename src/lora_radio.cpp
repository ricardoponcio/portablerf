#include "lora_radio.h"
#include <SPI.h>
#include "pins.h"
#include "spi_bus.h"

volatile bool LoRaRadio::dio1Flag_ = false;

// Passa o SPI explicitamente: o construtor padrão do Module chamaria SPI.begin() com os
// pinos default (MOSI=23, danificado nesta placa), e aqui o MOSI está no GPIO 25.
LoRaRadio::LoRaRadio(EventLog& log, Sniffer& sniffer, const LoRaConfig& cfg, float tcxoVoltage)
    : log_(log), sniffer_(sniffer), cfg_(cfg), tcxoVoltage_(tcxoVoltage),
      radio_(new Module(LORA_CS, LORA_DIO1, LORA_RST, LORA_BUSY, SPI, RADIOLIB_DEFAULT_SPI_SETTINGS)) {}

void LoRaRadio::beginAsync() {
    xTaskCreatePinnedToCore(initTask, "loraInit", 4096, this, 1, NULL, 0);
}

void LoRaRadio::initTask(void* param) {
    LoRaRadio* self = static_cast<LoRaRadio*>(param);
    const LoRaConfig& c = self->cfg_;
    int state;
    {
        SpiLock lock;
        state = self->radio_.begin(c.freqMhz, c.bwKhz, c.sf, c.cr, c.syncWord, 10, c.preamble, self->tcxoVoltage_);
        if (state == RADIOLIB_ERR_NONE) {
            self->radio_.setDio1Action(onDio1);
            state = self->radio_.startReceive();
        }
    }
    self->state_ = state;
    self->initDone_ = true;
    Serial.println(state == RADIOLIB_ERR_NONE ? "LoRa (SX1262) inicializado, em RX."
                                              : "Falha ao inicializar LoRa, codigo: " + String(state));
    vTaskDelete(NULL);
}

void IRAM_ATTR LoRaRadio::onDio1() {
    dio1Flag_ = true;
}

int LoRaRadio::apply(const LoRaConfig& c) {
    int state = radio_.standby();
    if (state == RADIOLIB_ERR_NONE) state = radio_.setFrequency(c.freqMhz);
    if (state == RADIOLIB_ERR_NONE) state = radio_.setBandwidth(c.bwKhz);
    if (state == RADIOLIB_ERR_NONE) state = radio_.setSpreadingFactor(c.sf);
    if (state == RADIOLIB_ERR_NONE) state = radio_.setCodingRate(c.cr);
    if (state == RADIOLIB_ERR_NONE) state = radio_.setSyncWord(c.syncWord);
    if (state == RADIOLIB_ERR_NONE) state = radio_.setPreambleLength(c.preamble);
    return state;
}

int LoRaRadio::setConfig(const LoRaConfig& cfg) {
    if (!ready()) return state_;
    SpiLock lock(pdMS_TO_TICKS(100));
    if (!lock) return RADIO_ERR_BUS_BUSY;

    int state = apply(cfg);
    if (state == RADIOLIB_ERR_NONE) cfg_ = cfg;
    else apply(cfg_);  // valor recusado: volta para a config que funcionava
    dio1Flag_ = false;
    radio_.startReceive();
    Serial.printf("LoRa: %.3f MHz SF%u BW%.1f CR4/%u sync 0x%02X pre %u -> %d\n", cfg_.freqMhz, cfg_.sf,
                  cfg_.bwKhz, cfg_.cr, cfg_.syncWord, cfg_.preamble, state);
    return state;
}

int LoRaRadio::setFrequencyMhz(float mhz) {
    LoRaConfig c = cfg_;
    c.freqMhz = mhz;
    return setConfig(c);
}

PacketProto LoRaRadio::proto() const {
    switch (cfg_.syncWord) {
        case 0x34: return PacketProto::LoRaWAN;
        case 0x2B: return PacketProto::Meshtastic;
        default:   return PacketProto::None;
    }
}

int LoRaRadio::send(const uint8_t* data, size_t len) {
    if (!ready()) return state_;
    if (len > MAX_MSG_LEN) len = MAX_MSG_LEN;
    SpiLock lock(pdMS_TO_TICKS(50));
    if (!lock) return RADIO_ERR_BUS_BUSY;

    // RadioLib 6.x recebe uint8_t* não-const, mas não altera o buffer
    int state = radio_.transmit(const_cast<uint8_t*>(data), len);
    // O DIO1 também dispara no fim do TX: limpa antes de voltar para RX
    dio1Flag_ = false;
    radio_.startReceive();
    sniffer_.mark(SNIFF_TX);
    log_.add(id(), true, state == RADIOLIB_ERR_NONE, data, len);
    return state;
}

void LoRaRadio::poll() {
    if (!ready()) return;
    SpiLock lock(0);
    if (!lock) return;
    if (!dio1Flag_) {
        // Em RX e sem pacote: energia no canal agora, qualquer que seja SF/sync word (sniff)
        sniffer_.add((int)radio_.getRSSI(false));
        return;
    }
    dio1Flag_ = false;

    uint8_t buf[256];
    size_t len = radio_.getPacketLength();
    int state = radio_.readData(buf, len);
    // CRC ruim ainda registra o pacote (útil para ver ruído/alcance), marcado como falha
    if (state == RADIOLIB_ERR_NONE || state == RADIOLIB_ERR_CRC_MISMATCH) {
        sniffer_.mark(state == RADIOLIB_ERR_NONE ? SNIFF_RX : SNIFF_BAD);
        log_.add(id(), false, state == RADIOLIB_ERR_NONE, buf, len, (int)radio_.getRSSI(), radio_.getSNR(), proto());
    }
    radio_.startReceive();
}
