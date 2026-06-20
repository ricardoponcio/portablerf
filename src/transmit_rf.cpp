#include "globals.h"
#include "transmit_rf.h"

static int selectedHistory = 0;
static bool redrawTransmit = true;

static void drawTransmitUI() {
    tft.fillScreen(COLOR_BG);
    tft.drawRect(0, 0, 128, 128, COLOR_TITLE);
    
    tft.setCursor(10, 6);
    tft.setTextColor(COLOR_TITLE);
    tft.setTextSize(1);
    tft.print("TRANSMITIR RF");
    tft.drawLine(0, 18, 128, 18, COLOR_TITLE);

    if (historyCount == 0) {
        tft.setTextColor(COLOR_TEXT);
        tft.setCursor(5, 50);
        tft.print("Lista vazia.");
        tft.setCursor(5, 65);
        tft.print("Capture sinais");
        tft.setCursor(5, 80);
        tft.print("com o Sniffer.");
        return;
    }

    for (int i = 0; i < historyCount; i++) {
        int y = 25 + (i * 16);
        if (i == selectedHistory) {
            tft.fillRect(2, y - 2, 124, 14, COLOR_TITLE);
            tft.setTextColor(COLOR_BG);
        } else {
            tft.setTextColor(COLOR_TEXT);
        }
        tft.setCursor(5, y);
        
        // Mostra frequência
        tft.print(signalHistory[i].freq, 0);
        tft.print("M ");
        
        // Mostra tipo e valor
        if (signalHistory[i].type == SIG_DECODED) {
            tft.print(signalHistory[i].decodedValue, HEX);
        } else if (signalHistory[i].type == SIG_RAW) {
            tft.print("RAW(");
            tft.print(signalHistory[i].rawCount);
            tft.print(")");
        }
    }

    tft.drawLine(0, 110, 128, 110, COLOR_TITLE);
    tft.setCursor(2, 116);
    tft.setTextColor(0x07E0);
    tft.print("OK p/ Enviar");
}

void transmit_rf_setup() {
    redrawTransmit = true;
    selectedHistory = 0;
}

void transmit_rf_loop() {
    if (redrawTransmit) {
        drawTransmitUI();
        redrawTransmit = false;
    }

    if (isBtnPressed(BTN_BACK)) {
        waitForBtnRelease(BTN_BACK);
        currentState = STATE_MENU;
        return;
    }

    if (historyCount > 0) {
        if (isBtnPressed(BTN_UP)) {
            waitForBtnRelease(BTN_UP);
            selectedHistory--;
            if (selectedHistory < 0) selectedHistory = historyCount - 1;
            redrawTransmit = true;
        }
        
        if (isBtnPressed(BTN_DOWN)) {
            waitForBtnRelease(BTN_DOWN);
            selectedHistory++;
            if (selectedHistory >= historyCount) selectedHistory = 0;
            redrawTransmit = true;
        }

        if (isBtnPressed(BTN_OK)) {
            waitForBtnRelease(BTN_OK);
            
            tft.fillRect(0, 111, 128, 16, COLOR_BG);
            tft.setCursor(2, 116);
            tft.setTextColor(0xF800);
            tft.print("Enviando...");

            transmitSignal(signalHistory[selectedHistory]);

            tft.fillRect(0, 111, 128, 16, COLOR_BG);
            tft.setCursor(2, 116);
            tft.setTextColor(0x07E0);
            tft.print("Enviado!");
            delay(800);
            
            redrawTransmit = true;
        }
    }
}
