#include "cc1101_radio.h"
#include <ELECHOUSE_CC1101_SRC_DRV.h>
#include "pins.h"
#include "spi_bus.h"

void Cc1101Radio::begin() {
    SpiLock lock;
    ELECHOUSE_cc1101.setSpiPin(SPI_SCK, SPI_MISO, SPI_MOSI, CC1101_CS);
    ELECHOUSE_cc1101.setGDO0(CC1101_GDO0);
    ELECHOUSE_cc1101.Init();

    // getCC1101() da lib aceita qualquer resposta != 0 (dava "detectado" mesmo com o
    // MOSI morto); o registrador VERSION do CC1101 é sempre 0x14 (0x04 em revisões antigas).
    byte version = ELECHOUSE_cc1101.SpiReadStatus(CC1101_VERSION);
    if (version != 0x14 && version != 0x04) {
        state_ = RADIO_ERR_NOT_FOUND;
        Serial.printf("CC1101 NAO detectado (VERSION=0x%02X).\n", version);
        return;
    }

    ELECHOUSE_cc1101.setCCMode(1);      // modo pacote (FIFO)
    ELECHOUSE_cc1101.setModulation(0);  // 2-FSK
    ELECHOUSE_cc1101.setMHZ(freqMhz_);
    ELECHOUSE_cc1101.setSyncMode(2);    // 16/16 bits de sync word
    ELECHOUSE_cc1101.setCrc(1);
    ELECHOUSE_cc1101.setPA(10);
    ELECHOUSE_cc1101.SetRx();
    state_ = RADIO_OK;
    Serial.println("CC1101 detectado, em RX.");
}

int Cc1101Radio::send(const uint8_t* data, size_t len) {
    if (!ready()) return state_;
    if (len > MAX_MSG_LEN) len = MAX_MSG_LEN;
    SpiLock lock(pdMS_TO_TICKS(50));
    if (!lock) return RADIO_ERR_BUS_BUSY;

    // SendData espera o fim do TX pelo GDO0; volta para RX no próximo CheckRxFifo()
    ELECHOUSE_cc1101.SendData((byte*)data, (byte)len);
    log_.add(id(), true, true, data, len);
    return RADIO_OK;
}

void Cc1101Radio::poll() {
    if (!ready()) return;
    SpiLock lock(0);
    if (!lock) return;
    if (!ELECHOUSE_cc1101.CheckRxFifo(10)) return;

    bool crcOk = ELECHOUSE_cc1101.CheckCRC();
    int rssi = ELECHOUSE_cc1101.getRssi();
    // O byte de tamanho vem do ar: com CRC ruim pode ser qualquer valor até 255
    byte buf[256];
    byte len = ELECHOUSE_cc1101.ReceiveData(buf);
    // Ruído às vezes casa com a sync word e gera "pacote" vazio: não polui o log
    if (len == 0) return;
    log_.add(id(), false, crcOk, buf, len, rssi);
}
