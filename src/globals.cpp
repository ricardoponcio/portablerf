#include "globals.h"

SystemState currentState = STATE_MENU;
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
RCSwitch mySwitch = RCSwitch();

SavedSignal signalHistory[MAX_HISTORY];
int historyCount = 0;

void saveSignalToHistory(SavedSignal &sig) {
    // Evita duplicata consecutiva de decoded
    if (sig.type == SIG_DECODED && historyCount > 0 
        && signalHistory[0].type == SIG_DECODED
        && signalHistory[0].decodedValue == sig.decodedValue) {
        return;
    }
    
    // Empurra tudo pra frente (mais recente na posicao 0)
    for (int i = MAX_HISTORY - 1; i > 0; i--) {
        signalHistory[i] = signalHistory[i - 1];
    }
    signalHistory[0] = sig;
    if (historyCount < MAX_HISTORY) historyCount++;
}

void transmitSignal(SavedSignal &sig) {
    if (sig.type == SIG_DECODED) {
        ELECHOUSE_cc1101.setCCMode(1);
        ELECHOUSE_cc1101.setModulation(2);
        ELECHOUSE_cc1101.setMHZ(sig.freq);
        
        mySwitch.disableReceive();
        mySwitch.enableTransmit(CC1101_GDO0);
        mySwitch.setRepeatTransmit(5);
        mySwitch.send(sig.decodedValue, sig.bitlength);
        mySwitch.disableTransmit();
        
        ELECHOUSE_cc1101.SetRx();
        mySwitch.enableReceive(digitalPinToInterrupt(CC1101_GDO0));
        
    } else if (sig.type == SIG_RAW) {
        detachInterrupt(digitalPinToInterrupt(CC1101_GDO0));
        mySwitch.disableReceive();
        
        ELECHOUSE_cc1101.setCCMode(0);
        ELECHOUSE_cc1101.setModulation(2);
        ELECHOUSE_cc1101.setMHZ(sig.freq);
        ELECHOUSE_cc1101.SetTx();
        pinMode(CC1101_GDO0, OUTPUT);
        
        for (int rep = 0; rep < 5; rep++) {
            int state = HIGH;
            for (int i = 0; i < sig.rawCount; i++) {
                digitalWrite(CC1101_GDO0, state);
                delayMicroseconds(sig.rawDurations[i]);
                state = !state;
            }
            digitalWrite(CC1101_GDO0, LOW);
            delay(15);
            yield();
        }
        
        ELECHOUSE_cc1101.SetRx();
        pinMode(CC1101_GDO0, INPUT);
    }
}

bool isBtnPressed(uint8_t btn) {
    if (btn == BTN_BACK) {
        return analogRead(BTN_BACK) > 800;
    } else if (btn == BTN_UP) {
        return digitalRead(BTN_UP) == HIGH;
    } else {
        return digitalRead(btn) == LOW;
    }
}

void waitForBtnRelease(uint8_t btn) {
    while (isBtnPressed(btn)) {
        delay(10);
    }
    delay(50); // Debounce
}
