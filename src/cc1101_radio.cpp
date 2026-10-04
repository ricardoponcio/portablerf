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
    // A lib grava PKTLEN=0, mas em tamanho variável ele é o máximo aceito e não pode ser 0
    ELECHOUSE_cc1101.setPacketLength(MAX_MSG_LEN);
    ELECHOUSE_cc1101.setPA(10);
    ELECHOUSE_cc1101.SetRx();
    state_ = RADIO_OK;
    Serial.println("CC1101 detectado, em RX.");
}

int Cc1101Radio::setFrequencyMhz(float mhz) {
    // Faixas suportadas pelo sintetizador do CC1101 (datasheet, seção 4.1)
    bool valid = (mhz >= 300 && mhz <= 348) || (mhz >= 387 && mhz <= 464) || (mhz >= 779 && mhz <= 928);
    if (!valid) return RADIO_ERR_BAD_PARAM;
    if (!ready()) {
        freqMhz_ = mhz;  // aplicada se/quando o begin() rodar
        return state_;
    }
    SpiLock lock(pdMS_TO_TICKS(50));
    if (!lock) return RADIO_ERR_BUS_BUSY;

    // Sai do RX antes de mexer no sintetizador; o SetRx() recalibra (FS_AUTOCAL) na nova frequência
    ELECHOUSE_cc1101.setSidle();
    ELECHOUSE_cc1101.setMHZ(mhz);
    ELECHOUSE_cc1101.SetRx();
    freqMhz_ = mhz;
    Serial.printf("CC1101 em %.2f MHz.\n", mhz);
    return RADIO_OK;
}

int Cc1101Radio::send(const uint8_t* data, size_t len) {
    if (!ready()) return state_;
    if (len > MAX_MSG_LEN) len = MAX_MSG_LEN;
    SpiLock lock(pdMS_TO_TICKS(50));
    if (!lock) return RADIO_ERR_BUS_BUSY;

    // SendData espera o fim do TX pelo GDO0; volta para RX no próximo CheckRxFifo()
    ELECHOUSE_cc1101.SendData((byte*)data, (byte)len);
    sniffer_.mark(SNIFF_TX);
    log_.add(id(), true, true, data, len);
    return RADIO_OK;
}

void Cc1101Radio::poll() {
    if (!ready()) return;
    SpiLock lock(0);
    if (!lock) return;
    restartRxIfStuck();
    if (!ELECHOUSE_cc1101.CheckRxFifo(10)) {
        // Rádio em RX e sem pacote: o RSSI é o que está no ar agora (sniff)
        sniffer_.add(ELECHOUSE_cc1101.getRssi());
        return;
    }

    bool crcOk = ELECHOUSE_cc1101.CheckCRC();
    int rssi = ELECHOUSE_cc1101.getRssi();
    sniffer_.mark(crcOk ? SNIFF_RX : SNIFF_BAD);
    // O byte de tamanho vem do ar: com CRC ruim pode ser qualquer valor até 255
    byte buf[256];
    byte len = ELECHOUSE_cc1101.ReceiveData(buf);
    // Ruído às vezes casa com a sync word e gera "pacote" vazio: não polui o log
    if (len == 0) return;
    log_.add(id(), false, crcOk, buf, len, rssi);
}

// Chamar com o SPI travado. O CC1101 às vezes sai do RX sozinho e fica em IDLE com o
// FIFO vazio (ex.: ruído casa com a sync word e o "pacote" é descartado). A lib guarda o
// estado em cache e acha que ainda está em RX, então nunca volta: a recepção morria aqui.
void Cc1101Radio::restartRxIfStuck() {
    byte marc = ELECHOUSE_cc1101.SpiReadStatus(CC1101_MARCSTATE) & 0x1F;
    bool idleEmpty = marc == MARC_IDLE && !(ELECHOUSE_cc1101.SpiReadStatus(CC1101_RXBYTES) & 0x7F);
    if (!idleEmpty && marc != MARC_RXFIFO_OVERFLOW) return;
    ELECHOUSE_cc1101.setSidle();
    ELECHOUSE_cc1101.SpiStrobe(CC1101_SFRX);
    ELECHOUSE_cc1101.SetRx();
}
