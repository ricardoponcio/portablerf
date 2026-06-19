#include "globals.h"

SystemState currentState = STATE_MENU;
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
RCSwitch mySwitch = RCSwitch();

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
