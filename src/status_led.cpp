#include "status_led.h"

void StatusLed::begin() {
    pinMode(pin_, OUTPUT);
}

void StatusLed::update() {
    if (millis() - lastMs_ < periodMs_) return;
    lastMs_ = millis();
    on_ = !on_;
    digitalWrite(pin_, on_);
}
