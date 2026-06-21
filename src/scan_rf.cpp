#include "globals.h"
#include "scan_rf.h"

#define NUM_BARS 16
#define BAR_WIDTH 6
#define BAR_SPACING 2
#define MAX_HEIGHT 88
#define COLOR_GRID    0x2104 
#define COLOR_BAR_LOW 0x07FF 
#define COLOR_BAR_MID 0xFFE0 
#define COLOR_BAR_HI  0xF800 

static int barHeights[NUM_BARS];

// --- Lista de frequências comuns ---
static const int NUM_FREQS = 4;
static const float commonFreqs[NUM_FREQS] = {315.0, 433.0, 868.0, 915.0};
static int currentFreqIdx = 1; // Inicia em 433.0
static float stepFreq = 0.12;

static unsigned long codeDisplayTimer = 0;
static bool showingCode = false;
static bool needInitialDraw = true;

// Último sinal capturado (para retransmissão imediata)
static SavedSignal lastSniffed;

static void drawBottomStatus(String text, uint16_t color) {
  tft.fillRect(0, 111, 128, 16, COLOR_BG);
  tft.drawRect(0, 0, 128, 128, COLOR_TITLE); 
  tft.setCursor(2, 116);
  tft.setTextColor(color);
  tft.print(text);
}

static void drawAnalyzerUI() {
  tft.fillScreen(COLOR_BG);
  tft.drawRect(0, 0, 128, 128, COLOR_TITLE);
  
  tft.setCursor(10, 6);
  tft.setTextColor(COLOR_TITLE);
  tft.setTextSize(1);
  tft.print("SNIFFER RF ");
  tft.print(commonFreqs[currentFreqIdx], 0);
  tft.print("M");

  tft.drawLine(0, 18, 128, 18, COLOR_TITLE);
  tft.drawLine(0, 110, 128, 110, COLOR_TITLE);

  drawBottomStatus("Escaneando...", 0x07E0);
}

void scan_rf_setup() {
    needInitialDraw = true;
    lastSniffed.type = SIG_NONE;
    for (int i = 0; i < NUM_BARS; i++) {
        barHeights[i] = 0;
    }
    mySwitch.enableReceive(digitalPinToInterrupt(CC1101_GDO0));
}

void scan_rf_loop() {
    if (needInitialDraw) {
        drawAnalyzerUI();
        needInitialDraw = false;
    }

    if (showingCode && (millis() - codeDisplayTimer > 3000)) {
        showingCode = false;
        if (lastSniffed.type == SIG_DECODED) {
            drawBottomStatus("OK=Reenviar", 0x07E0);
        } else {
            drawBottomStatus("Escaneando...", 0x07E0);
        }
    }

    // O loop interno itera pelas barras (frequências)
    for (int i = 0; i < NUM_BARS; i++) {
        // Verifica botões de navegação
        if (isBtnPressed(BTN_BACK)) {
            waitForBtnRelease(BTN_BACK);
            mySwitch.disableReceive();
            currentState = STATE_MENU;
            return;
        }

        if (isBtnPressed(BTN_UP)) {
            waitForBtnRelease(BTN_UP);
            currentFreqIdx++;
            if (currentFreqIdx >= NUM_FREQS) currentFreqIdx = 0;
            scan_rf_setup(); // Reseta as barras e a tela
            return;          // Sai do loop para reiniciar
        }

        if (isBtnPressed(BTN_DOWN)) {
            waitForBtnRelease(BTN_DOWN);
            currentFreqIdx--;
            if (currentFreqIdx < 0) currentFreqIdx = NUM_FREQS - 1;
            scan_rf_setup(); // Reseta as barras e a tela
            return;          // Sai do loop para reiniciar
        }

        // Retransmite o último código capturado
        if (isBtnPressed(BTN_OK) && lastSniffed.type == SIG_DECODED) {
            waitForBtnRelease(BTN_OK);
            drawBottomStatus("Enviando...", 0xF800);
            
            transmitSignal(lastSniffed);
            
            drawBottomStatus("Enviado!", 0x07E0);
            showingCode = true;
            codeDisplayTimer = millis();
            return;
        }

        float freq = commonFreqs[currentFreqIdx] + (i * stepFreq);
        ELECHOUSE_cc1101.setMHZ(freq);
        ELECHOUSE_cc1101.SetRx(); 
        delayMicroseconds(600); 
        
        int rssi = ELECHOUSE_cc1101.getRssi();
        int targetHeight = map(rssi, -95, -40, 0, MAX_HEIGHT);
        targetHeight = constrain(targetHeight, 0, MAX_HEIGHT);

        if (targetHeight > barHeights[i]) {
            barHeights[i] = targetHeight;
        } else {
            if (barHeights[i] > 0) barHeights[i] -= 12;
            if (barHeights[i] < 0) barHeights[i] = 0;
        }

        int x = 2 + i * (BAR_WIDTH + BAR_SPACING);
        int y = 108 - barHeights[i];
        
        tft.fillRect(x, 20, BAR_WIDTH, MAX_HEIGHT - barHeights[i], COLOR_BG);

        if (barHeights[i] > 0) {
            uint16_t barColor = COLOR_BAR_LOW;
            if (barHeights[i] > MAX_HEIGHT * 0.5) barColor = COLOR_BAR_MID;
            if (barHeights[i] > MAX_HEIGHT * 0.8) barColor = COLOR_BAR_HI;
            tft.fillRect(x, y, BAR_WIDTH, barHeights[i], barColor);
        }

        // Se o sinal for forte, escuta por pacotes OOK
        if (targetHeight > MAX_HEIGHT * 0.6) {
            unsigned long waitStart = millis();
            while (millis() - waitStart < 500) { 
                if (isBtnPressed(BTN_BACK)) {
                    waitForBtnRelease(BTN_BACK);
                    mySwitch.disableReceive();
                    currentState = STATE_MENU;
                    return;
                }

                if (mySwitch.available()) {
                    long value = mySwitch.getReceivedValue();
                    int bitlen = mySwitch.getReceivedBitlength();
                    
                    if (value != 0) {
                        String hexCode = String(value, HEX);
                        hexCode.toUpperCase();
                        String msg = "COD:" + hexCode + " (" + String(bitlen) + "b)";
                        
                        drawBottomStatus(msg, 0xFFE0);
                        showingCode = true;
                        codeDisplayTimer = millis();
                        
                        // Salva para retransmissão imediata e na lista global
                        lastSniffed.type = SIG_DECODED;
                        lastSniffed.decodedValue = value;
                        lastSniffed.bitlength = bitlen;
                        lastSniffed.freq = freq;
                        lastSniffed.rawCount = 0;
                        saveSignalToHistory(lastSniffed);
                    }
                    mySwitch.resetAvailable(); 
                }
                yield();
            }
        }
    }
}
