#pragma once
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <ELECHOUSE_CC1101_SRC_DRV.h>
#include <RCSwitch.h>

// --- Pinos ---
#define TFT_CS     15
#define TFT_RST    2
#define TFT_DC     0

#define CC1101_CS   5
#define CC1101_GDO0 4

#define BTN_BACK    A0
#define BTN_UP      16
#define BTN_DOWN    1
#define BTN_OK      3

// --- Cores Globais ---
#define COLOR_BG      0x0000 
#define COLOR_TITLE   0x07E0 
#define COLOR_TEXT    0xFFFF
#define COLOR_HIGHLIGHT 0xF800

// --- Estado do Sistema ---
enum SystemState {
    STATE_MENU,
    STATE_SCAN_RF,
    STATE_RAW_RF,
    STATE_ANALYZE_RF,
    STATE_TRANSMIT,
    STATE_JAMMER,
    STATE_SOBRE
};

// --- Sinais Capturados (Lista Unificada) ---
#define MAX_RAW_BUFFER 300
#define MAX_HISTORY 5

enum SignalType {
    SIG_NONE,
    SIG_DECODED,  // Sniffer RF (rc-switch decodificou)
    SIG_RAW       // Sniffer RAW (pulsos brutos)
};

struct SavedSignal {
    SignalType type;
    float freq;
    // Para sinais decodificados (Sniffer RF)
    long decodedValue;
    int bitlength;
    // Para sinais brutos (Sniffer RAW)
    uint16_t rawDurations[MAX_RAW_BUFFER];
    int rawCount;
};

extern SavedSignal signalHistory[MAX_HISTORY];
extern int historyCount;
extern void saveSignalToHistory(SavedSignal &sig);
extern void transmitSignal(SavedSignal &sig);

extern SystemState currentState;
extern Adafruit_ST7735 tft;
extern RCSwitch mySwitch;

// --- Funções Auxiliares ---
bool isBtnPressed(uint8_t btn);
void waitForBtnRelease(uint8_t btn);
