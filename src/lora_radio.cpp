#include "lora_radio.h"
#include <SPI.h>
#include "pins.h"
#include "spi_bus.h"

volatile bool LoRaRadio::dio1Flag_ = false;

// Passa o SPI explicitamente: o construtor padrão do Module chamaria SPI.begin() com os
// pinos default (MOSI=23, danificado nesta placa), e aqui o MOSI está no GPIO 25.
LoRaRadio::LoRaRadio(EventLog& log, float freqMhz, float tcxoVoltage)
    : log_(log), freqMhz_(freqMhz), tcxoVoltage_(tcxoVoltage),
      radio_(new Module(LORA_CS, LORA_DIO1, LORA_RST, LORA_BUSY, SPI, RADIOLIB_DEFAULT_SPI_SETTINGS)) {}

void LoRaRadio::beginAsync() {
    xTaskCreatePinnedToCore(initTask, "loraInit", 4096, this, 1, NULL, 0);
}

void LoRaRadio::initTask(void* param) {
    LoRaRadio* self = static_cast<LoRaRadio*>(param);
    int state;
    {
        SpiLock lock;
        state = self->radio_.begin(self->freqMhz_, 125.0, 9, 7, RADIOLIB_SX126X_SYNC_WORD_PRIVATE,
                                   10, 8, self->tcxoVoltage_);
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
    log_.add(id(), true, state == RADIOLIB_ERR_NONE, data, len);
    return state;
}

void LoRaRadio::poll() {
    if (!ready() || !dio1Flag_) return;
    SpiLock lock(0);
    if (!lock) return;
    dio1Flag_ = false;

    uint8_t buf[256];
    size_t len = radio_.getPacketLength();
    int state = radio_.readData(buf, len);
    // CRC ruim ainda registra o pacote (útil para ver ruído/alcance), marcado como falha
    if (state == RADIOLIB_ERR_NONE || state == RADIOLIB_ERR_CRC_MISMATCH) {
        log_.add(id(), false, state == RADIOLIB_ERR_NONE, buf, len, (int)radio_.getRSSI(), radio_.getSNR());
    }
    radio_.startReceive();
}
