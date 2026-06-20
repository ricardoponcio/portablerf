#include "globals.h"
#include "menu.h"

static const int NUM_ITEMS = 6;
static const char* menuItems[NUM_ITEMS] = {"Sniffer RF", "Sniffer RAW", "Radar RF", "Transmitir", "Jammer", "Sobre"};
static int selectedItem = 0;
static bool redrawMenu = true;

static const int VISIBLE_ITEMS = 5;
static int scrollOffset = 0;

static void drawMenu() {
    tft.fillScreen(COLOR_BG);
    tft.drawRect(0, 0, 128, 128, COLOR_TITLE);
    
    tft.setCursor(10, 10);
    tft.setTextColor(COLOR_TITLE);
    tft.setTextSize(1);
    tft.print("PORTABLE RF - MENU");
    tft.drawLine(0, 25, 128, 25, COLOR_TITLE);

    if (selectedItem < scrollOffset) {
        scrollOffset = selectedItem;
    }
    if (selectedItem >= scrollOffset + VISIBLE_ITEMS) {
        scrollOffset = selectedItem - VISIBLE_ITEMS + 1;
    }

    for (int i = 0; i < VISIBLE_ITEMS; i++) {
        int itemIdx = scrollOffset + i;
        if (itemIdx >= NUM_ITEMS) break;

        int y = 35 + (i * 18);
        if (itemIdx == selectedItem) {
            tft.fillRect(10, y - 2, 100, 14, COLOR_TITLE);
            tft.setTextColor(COLOR_BG);
        } else {
            tft.setTextColor(COLOR_TEXT);
        }
        tft.setCursor(15, y);
        tft.print(menuItems[itemIdx]);
    }
    
    if (NUM_ITEMS > VISIBLE_ITEMS) {
        int scrollH = (VISIBLE_ITEMS * 80) / NUM_ITEMS;
        int scrollY = 35 + ((scrollOffset * 80) / NUM_ITEMS);
        tft.drawRect(118, 35, 4, 80, 0x4208); // Fundo da barra
        tft.fillRect(118, scrollY, 4, scrollH, COLOR_TITLE); // Marcador
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
            currentState = STATE_RAW_RF;
        } else if (selectedItem == 2) {
            currentState = STATE_ANALYZE_RF;
        } else if (selectedItem == 3) {
            currentState = STATE_TRANSMIT;
        } else if (selectedItem == 4) {
            currentState = STATE_JAMMER;
        } else if (selectedItem == 5) {
            currentState = STATE_SOBRE;
        }
    }
}
