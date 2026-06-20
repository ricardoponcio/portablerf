#include "globals.h"
#include "analyze_rf.h"

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

static bool needInitialDraw = true;

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
  tft.print("RADAR RF ");
  tft.print(commonFreqs[currentFreqIdx], 0);
  tft.print("M");

  tft.drawLine(0, 18, 128, 18, COLOR_TITLE);
  tft.drawLine(0, 110, 128, 110, COLOR_TITLE);

  drawBottomStatus("Apenas Escuta...", 0x07E0);
}

void analyze_rf_setup() {
    needInitialDraw = true;
    for (int i = 0; i < NUM_BARS; i++) {
        barHeights[i] = 0;
    }
}

void analyze_rf_loop() {
    if (needInitialDraw) {
        drawAnalyzerUI();
        needInitialDraw = false;
    }

    // O loop interno itera pelas barras (frequências)
    for (int i = 0; i < NUM_BARS; i++) {
        // Verifica botões de navegação
        if (isBtnPressed(BTN_BACK)) {
            waitForBtnRelease(BTN_BACK);
            currentState = STATE_MENU;
            return;
        }

        if (isBtnPressed(BTN_UP)) {
            waitForBtnRelease(BTN_UP);
            currentFreqIdx++;
            if (currentFreqIdx >= NUM_FREQS) currentFreqIdx = 0;
            analyze_rf_setup(); // Reseta as barras e a tela
            return;          // Sai do loop para reiniciar
        }

        if (isBtnPressed(BTN_DOWN)) {
            waitForBtnRelease(BTN_DOWN);
            currentFreqIdx--;
            if (currentFreqIdx < 0) currentFreqIdx = NUM_FREQS - 1;
            analyze_rf_setup(); // Reseta as barras e a tela
            return;          // Sai do loop para reiniciar
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
        
        // Aqui NAO pausamos para ler código. Ele passa direto e plota a próxima barra!
    }
}
