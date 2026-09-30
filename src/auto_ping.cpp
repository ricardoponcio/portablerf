#include "auto_ping.h"

void AutoPing::setIntervalMs(uint32_t ms) {
    intervalMs_ = ms == 0 ? 0 : max(ms, (uint32_t)500);
    lastMs_ = 0;
}

void AutoPing::poll() {
    if (intervalMs_ == 0 || millis() - lastMs_ < intervalMs_) return;
    lastMs_ = millis();
    radio_.send("PING #" + String(++counter_));
}
