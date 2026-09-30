#pragma once
#include <Arduino.h>

// Pisca o LED sem bloquear: sinal visual de que o loop() está rodando.
class StatusLed {
public:
    StatusLed(uint8_t pin, uint32_t periodMs) : pin_(pin), periodMs_(periodMs) {}

    void begin();
    void update();  // chamar no loop()

private:
    uint8_t pin_;
    uint32_t periodMs_;
    uint32_t lastMs_ = 0;
    bool on_ = false;
};
