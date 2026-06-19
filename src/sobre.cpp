#include "globals.h"
#include "sobre.h"

static bool needDrawSobre = true;

void sobre_setup() {
    needDrawSobre = true;
}

void sobre_loop() {
    if (needDrawSobre) {
        tft.fillScreen(COLOR_BG);
        tft.drawRect(0, 0, 128, 128, COLOR_TITLE);
        
        tft.setTextColor(COLOR_TITLE);
        tft.setTextSize(2);
        tft.setCursor(5, 20);
        tft.print("PortableRF");

        tft.setTextColor(COLOR_TEXT);
        tft.setTextSize(1);
        tft.setCursor(15, 60);
        tft.print("Projeto criado e");
        tft.setCursor(15, 75);
        tft.print("desenvolvido por:");

        tft.setTextColor(COLOR_HIGHLIGHT);
        tft.setTextSize(1);
        tft.setCursor(20, 95);
        tft.print("Ricardo Poncio");

        needDrawSobre = false;
    }

    if (isBtnPressed(BTN_BACK)) {
        waitForBtnRelease(BTN_BACK);
        currentState = STATE_MENU;
    }
}
