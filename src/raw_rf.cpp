#include "globals.h"
#include "raw_rf.h"

static const int NUM_FREQS = 4;
static const float rawFreqs[NUM_FREQS] = {315.0, 433.92, 868.0, 915.0};
static int freqIdx = 1; 

static bool redrawRaw = true;
static bool isListening = false;

// Captura local da tela (limpa quando muda de frequência)
static SavedSignal localCapture;

volatile unsigned int irqDurations[MAX_RAW_BUFFER];
volatile int irqCount = 0;
volatile unsigned long irqLastMicros = 0;
volatile bool irqCapturing = false;

void ICACHE_RAM_ATTR rawInterruptHandler() {
    if (!irqCapturing) return;
    unsigned long now = micros();
    unsigned int dur = now - irqLastMicros;
    irqLastMicros = now;
    
    if (dur > 60 && dur < 15000) {
        if (irqCount < MAX_RAW_BUFFER) {
            irqDurations[irqCount++] = dur;
        }
    }
}

static void drawRawUI() {
    tft.fillScreen(COLOR_BG);
    tft.drawRect(0, 0, 128, 128, 0xFFE0);
    
    tft.setCursor(10, 6);
    tft.setTextColor(0xFFE0);
    tft.setTextSize(1);
    tft.print("SNIFFER RAW");
    tft.drawLine(0, 18, 128, 18, 0xFFE0);

    tft.setTextColor(COLOR_TEXT);
    tft.setCursor(5, 30);
    tft.print("Frequencia:");
    
    tft.setTextSize(2);
    tft.setCursor(5, 45);
    tft.setTextColor(COLOR_TITLE);
    tft.print(rawFreqs[freqIdx], 2);
    tft.setTextSize(1);
    
    tft.setCursor(5, 75);
    // Só mostra "Capturado" se a captura local é da frequência atual
    if (localCapture.type == SIG_RAW && abs(localCapture.freq - rawFreqs[freqIdx]) < 0.1) {
        tft.setTextColor(0x07E0);
        tft.print("Capturado: ");
        tft.print(localCapture.rawCount);
        tft.setCursor(5, 90);
        tft.print("OK p/ Replay");
    } else {
        tft.setTextColor(COLOR_TEXT);
        tft.print("Sem capturas.");
    }

    tft.drawLine(0, 110, 128, 110, 0xFFE0);
    tft.setCursor(2, 116);
    if (isListening) {
        tft.setTextColor(0xF800);
        tft.print("ESCUTANDO...");
    } else {
        tft.setTextColor(0xFFE0);
        tft.print("UP=Escutar");
    }
}

void raw_rf_setup() {
    redrawRaw = true;
    isListening = false;
    localCapture.type = SIG_NONE;
    
    mySwitch.disableReceive();
    
    ELECHOUSE_cc1101.setCCMode(0);
    ELECHOUSE_cc1101.setModulation(2);
    ELECHOUSE_cc1101.setMHZ(rawFreqs[freqIdx]);
    ELECHOUSE_cc1101.SetRx();
    
    pinMode(CC1101_GDO0, INPUT);
    attachInterrupt(digitalPinToInterrupt(CC1101_GDO0), rawInterruptHandler, CHANGE);
}

static void captureRawInterrupt() {
    tft.fillRect(0, 111, 128, 16, COLOR_BG);
    tft.setCursor(2, 116);
    tft.setTextColor(0xFFE0);
    tft.print("Aguardando...");
    
    ELECHOUSE_cc1101.setMHZ(rawFreqs[freqIdx]);
    ELECHOUSE_cc1101.SetRx();
    
    unsigned long waitStart = millis();
    bool foundSignal = false;
    
    while(millis() - waitStart < 8000) {
        if (isBtnPressed(BTN_BACK)) return;
        
        if (ELECHOUSE_cc1101.getRssi() > -70) {
            foundSignal = true;
            break;
        }
        yield();
    }
    
    if (!foundSignal) {
        tft.fillRect(0, 111, 128, 16, COLOR_BG);
        tft.setCursor(2, 116);
        tft.setTextColor(0xF800);
        tft.print("Timeout!");
        delay(800);
        return;
    }
    
    tft.fillRect(0, 111, 128, 16, COLOR_BG);
    tft.setCursor(2, 116);
    tft.setTextColor(0xF800);
    tft.print("GRAVANDO!!");
    
    irqCount = 0;
    irqLastMicros = micros();
    irqCapturing = true;
    
    unsigned long startMs = millis();
    
    while(millis() - startMs < 600) { 
        if (irqCount >= MAX_RAW_BUFFER) {
            break;
        }
        yield();
    }
    
    irqCapturing = false;
    
    if (irqCount > 10) { // Pelo menos 10 pulsos válidos
        localCapture.type = SIG_RAW;
        localCapture.freq = rawFreqs[freqIdx];
        localCapture.rawCount = irqCount;
        localCapture.decodedValue = 0;
        localCapture.bitlength = 0;
        for(int i = 0; i < irqCount; i++) {
            localCapture.rawDurations[i] = (uint16_t)irqDurations[i];
        }
        
        // Salva na lista global unificada
        saveSignalToHistory(localCapture);
    }
}

static void restoreCCModeForOtherMenus() {
    detachInterrupt(digitalPinToInterrupt(CC1101_GDO0));
    ELECHOUSE_cc1101.setCCMode(1);
    ELECHOUSE_cc1101.setModulation(2);
    ELECHOUSE_cc1101.SetRx();
    mySwitch.enableReceive(digitalPinToInterrupt(CC1101_GDO0));
}

void raw_rf_loop() {
    if (redrawRaw) {
        drawRawUI();
        redrawRaw = false;
    }

    if (isBtnPressed(BTN_BACK)) {
        waitForBtnRelease(BTN_BACK);
        restoreCCModeForOtherMenus();
        currentState = STATE_MENU;
        return;
    }

    if (isBtnPressed(BTN_DOWN)) {
        waitForBtnRelease(BTN_DOWN);
        freqIdx++;
        if (freqIdx >= NUM_FREQS) freqIdx = 0;
        ELECHOUSE_cc1101.setMHZ(rawFreqs[freqIdx]);
        ELECHOUSE_cc1101.SetRx();
        redrawRaw = true; // Vai redesenhar e mostrar "Sem captura" se freq mudou
    }

    if (isBtnPressed(BTN_UP)) {
        waitForBtnRelease(BTN_UP);
        isListening = true;
        drawRawUI();
        
        captureRawInterrupt();
        
        isListening = false;
        redrawRaw = true;
    }

    // OK só retransmite se tem captura E a frequência atual bate
    if (isBtnPressed(BTN_OK)) {
        waitForBtnRelease(BTN_OK);
        if (localCapture.type == SIG_RAW && abs(localCapture.freq - rawFreqs[freqIdx]) < 0.1) {
            tft.fillRect(0, 111, 128, 16, COLOR_BG);
            tft.setCursor(2, 116);
            tft.setTextColor(0xF800);
            tft.print("TOCANDO FITA...");
            
            transmitSignal(localCapture);
            
            // Restaura modo RAW de recepção após transmitir
            ELECHOUSE_cc1101.setCCMode(0);
            ELECHOUSE_cc1101.setModulation(2);
            ELECHOUSE_cc1101.setMHZ(rawFreqs[freqIdx]);
            ELECHOUSE_cc1101.SetRx();
            pinMode(CC1101_GDO0, INPUT);
            attachInterrupt(digitalPinToInterrupt(CC1101_GDO0), rawInterruptHandler, CHANGE);
            
            delay(300);
            redrawRaw = true;
        }
    }
}
