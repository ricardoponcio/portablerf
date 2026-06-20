#include "globals.h"
#include "jammer_rf.h"

static const int NUM_FREQS = 4;
static const float jammerFreqs[NUM_FREQS] = {315.0, 433.0, 868.0, 915.0};
static int freqIdx = 1; // Start 433
static bool isJamming = false;
static bool redrawJammer = true;

static void drawJammerUI() {
    tft.fillScreen(COLOR_BG);
    tft.drawRect(0, 0, 128, 128, 0xF800); // Red border
    
    tft.setCursor(10, 6);
    tft.setTextColor(0xF800);
    tft.setTextSize(1);
    tft.print("JAMMER RF");
    tft.drawLine(0, 18, 128, 18, 0xF800);

    tft.setTextColor(COLOR_TEXT);
    tft.setCursor(20, 40);
    tft.print("Frequencia:");
    
    tft.setTextSize(2);
    tft.setCursor(20, 60);
    if (isJamming) {
        tft.setTextColor(0xF800); // Red if active
    } else {
        tft.setTextColor(COLOR_TITLE);
    }
    tft.print(jammerFreqs[freqIdx], 0);
    tft.print("M");
    tft.setTextSize(1);

    tft.drawLine(0, 110, 128, 110, 0xF800);
    tft.setCursor(2, 116);
    if (isJamming) {
        tft.setTextColor(0xF800);
        tft.print("ATIVO! OK=Parar");
    } else {
        tft.setTextColor(0x07E0);
        tft.print("OK p/ Iniciar");
    }
}

void jammer_rf_setup() {
    redrawJammer = true;
    isJamming = false;
    ELECHOUSE_cc1101.SetRx(); // Stop any TX
}

void jammer_rf_loop() {
    if (redrawJammer) {
        drawJammerUI();
        redrawJammer = false;
    }

    if (isBtnPressed(BTN_BACK)) {
        waitForBtnRelease(BTN_BACK);
        if (isJamming) {
            ELECHOUSE_cc1101.SetRx();
            isJamming = false;
        }
        currentState = STATE_MENU;
        return;
    }

    if (!isJamming) {
        if (isBtnPressed(BTN_UP)) {
            waitForBtnRelease(BTN_UP);
            freqIdx++;
            if (freqIdx >= NUM_FREQS) freqIdx = 0;
            redrawJammer = true;
        }
        
        if (isBtnPressed(BTN_DOWN)) {
            waitForBtnRelease(BTN_DOWN);
            freqIdx--;
            if (freqIdx < 0) freqIdx = NUM_FREQS - 1;
            redrawJammer = true;
        }
    }

    if (isBtnPressed(BTN_OK)) {
        waitForBtnRelease(BTN_OK);
        isJamming = !isJamming;
        
        if (isJamming) {
            ELECHOUSE_cc1101.setMHZ(jammerFreqs[freqIdx]);
            ELECHOUSE_cc1101.SetTx(); // Transmissão contínua
        } else {
            ELECHOUSE_cc1101.SetRx();
        }
        redrawJammer = true;
    }
}
