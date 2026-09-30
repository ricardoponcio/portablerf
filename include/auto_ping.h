#pragma once
#include "radio.h"

// Envia "PING #n" periodicamente por um rádio (teste de alcance).
class AutoPing {
public:
    explicit AutoPing(Radio& radio) : radio_(radio) {}

    void setIntervalMs(uint32_t ms);  // 0 desliga; mínimo 500ms
    uint32_t intervalMs() const { return intervalMs_; }

    void poll();  // chamar no loop()

private:
    Radio& radio_;
    uint32_t intervalMs_ = 0;
    uint32_t lastMs_ = 0;
    uint32_t counter_ = 0;
};
