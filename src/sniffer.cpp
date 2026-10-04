#include "sniffer.h"

void Sniffer::add(int rssi) {
    uint32_t now = millis();
    if (open_ && now - bucketStart_ >= SNIFF_BUCKET_MS) commit();
    if (!open_) {
        open_ = true;
        bucketStart_ = now;
        peak_ = rssi;
    } else if (rssi > peak_) {
        peak_ = rssi;
    }
}

void Sniffer::mark(uint8_t flag) {
    flags_ |= flag;
}

void Sniffer::commit() {
    SniffSample& s = samples_[nextId_ % SNIFF_SIZE];
    s.rssi = constrain(peak_, -128, 127);
    s.flags = flags_;
    nextId_++;
    open_ = false;
    flags_ = 0;
}
