#include "globals.h"
#include "menu.h"

static const int NUM_ITEMS = 3;
static const char* menuItems[NUM_ITEMS] = {"Scan RF", "Analyze RF", "Sobre"};
static int selectedItem = 0;
static bool redrawMenu = true;

static void drawMenu() {
    tft.fillScreen(COLOR_BG);
    tft.drawRect(0, 0, 128, 128, COLOR_TITLE);
    
    tft.setCursor(10, 10);
    tft.setTextColor(COLOR_TITLE);
    tft.setTextSize(1);
    tft.print("PORTABLE RF - MENU");
    tft.drawLine(0, 25, 128, 25, COLOR_TITLE);

    for (int i = 0; i < NUM_ITEMS; i++) {
        int y = 45 + (i * 20);
        if (i == selectedItem) {
            tft.fillRect(15, y - 2, 98, 14, COLOR_TITLE);
            tft.setTextColor(COLOR_BG);
        } else {
            tft.setTextColor(COLOR_TEXT);
        }
        tft.setCursor(20, y);
        tft.print(menuItems[i]);
    }
}

void menu_setup() {
    redrawMenu = true;
}

void menu_loop() {
    if (redrawMenu) {
        drawMenu();
        redrawMenu = false;
    }

    if (isBtnPressed(BTN_UP)) {
        waitForBtnRelease(BTN_UP);
        selectedItem--;
        if (selectedItem < 0) selectedItem = NUM_ITEMS - 1;
        redrawMenu = true;
    }
    
    if (isBtnPressed(BTN_DOWN)) {
        waitForBtnRelease(BTN_DOWN);
        selectedItem++;
        if (selectedItem >= NUM_ITEMS) selectedItem = 0;
        redrawMenu = true;
    }

    if (isBtnPressed(BTN_OK)) {
        waitForBtnRelease(BTN_OK);
        if (selectedItem == 0) {
            currentState = STATE_SCAN_RF;
        } else if (selectedItem == 1) {
            currentState = STATE_ANALYZE_RF;
        } else if (selectedItem == 2) {
            currentState = STATE_SOBRE;
        }
    }
}
