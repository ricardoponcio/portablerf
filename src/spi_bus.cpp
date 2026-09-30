#include "spi_bus.h"
#include <SPI.h>
#include "pins.h"

static SemaphoreHandle_t spiMutex = nullptr;

void SpiBus::begin() {
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
    spiMutex = xSemaphoreCreateMutex();
}

bool SpiBus::lock(TickType_t wait) {
    return spiMutex && xSemaphoreTake(spiMutex, wait) == pdTRUE;
}

void SpiBus::unlock() {
    xSemaphoreGive(spiMutex);
}
