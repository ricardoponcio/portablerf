#pragma once
#include <Arduino.h>

// Barramento SPI compartilhado entre CC1101 e LoRa. O begin() do LoRa roda
// numa task separada e pode demorar, então todo acesso passa por um mutex.
class SpiBus {
public:
    static void begin();
    static bool lock(TickType_t wait);
    static void unlock();
};

// Trava o SPI enquanto o objeto existir:
//   SpiLock lock(0); if (!lock) return;   // não conseguiu, barramento ocupado
class SpiLock {
public:
    explicit SpiLock(TickType_t wait = portMAX_DELAY) : locked_(SpiBus::lock(wait)) {}
    ~SpiLock() { if (locked_) SpiBus::unlock(); }
    SpiLock(const SpiLock&) = delete;
    SpiLock& operator=(const SpiLock&) = delete;
    explicit operator bool() const { return locked_; }

private:
    bool locked_;
};
