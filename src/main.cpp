#include "globals.h"
#include "menu.h"
#include "scan_rf.h"
#include "analyze_rf.h"
#include "sobre.h"

SystemState lastState = STATE_MENU;

void showSplashScreen() {
    tft.fillScreen(COLOR_BG);
    tft.drawRect(0, 0, 128, 128, 0xFFFF);
    tft.setTextColor(0xFFFF);
    tft.setTextSize(2);
    tft.setCursor(5, 50);
    tft.print("PortableRF");
    delay(2000);
}

void setup() {
    // Inicializa botões
    pinMode(BTN_UP, INPUT_PULLDOWN_16);
    pinMode(BTN_DOWN, INPUT_PULLUP);
    pinMode(BTN_OK, INPUT_PULLUP);
    // BTN_BACK (A0) não precisa de pinMode para analogRead

    // Inicializa Display
    tft.initR(INITR_144GREENTAB);
    showSplashScreen();

    // Inicia CC1101
    tft.fillScreen(COLOR_BG);
    tft.setTextColor(0xFFFF);
    tft.setTextSize(1);
    tft.setCursor(10, 65);
    tft.print("Iniciando Hardware");

    ELECHOUSE_cc1101.setSpiPin(14, 12, 13, CC1101_CS);
    ELECHOUSE_cc1101.Init();
    ELECHOUSE_cc1101.setCCMode(1); 
    ELECHOUSE_cc1101.setModulation(2);

    if (ELECHOUSE_cc1101.getCC1101()) {
        tft.setTextColor(COLOR_TITLE);
        tft.setCursor(20, 85);
        tft.print("CC1101 OK!");
        delay(1000);
    } else {
        tft.setTextColor(0xF800);
        tft.setCursor(15, 85);
        tft.print("ERRO CC1101");
        while(1) yield();
    }

    mySwitch.enableReceive(digitalPinToInterrupt(CC1101_GDO0));

    // Força setup inicial
    menu_setup();
    currentState = STATE_MENU;
    lastState = STATE_MENU;
}

void loop() {
    // Gerenciador de Transição de Estado
    if (currentState != lastState) {
        if (currentState == STATE_MENU) {
            menu_setup();
        } else if (currentState == STATE_SCAN_RF) {
            scan_rf_setup();
        } else if (currentState == STATE_ANALYZE_RF) {
            analyze_rf_setup();
        } else if (currentState == STATE_SOBRE) {
            sobre_setup();
        }
        lastState = currentState;
    }

    // Executa o loop do estado atual
    if (currentState == STATE_MENU) {
        menu_loop();
    } else if (currentState == STATE_SCAN_RF) {
        scan_rf_loop();
    } else if (currentState == STATE_ANALYZE_RF) {
        analyze_rf_loop();
    } else if (currentState == STATE_SOBRE) {
        sobre_loop();
    }
}