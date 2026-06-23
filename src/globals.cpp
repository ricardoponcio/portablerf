#include "globals.h"

SystemState currentState = STATE_MENU;
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
RCSwitch mySwitch = RCSwitch();

SavedSignal signalHistory[MAX_HISTORY];
int historyCount = 0;

// CC1101 interrupt state
volatile bool irqCapturing = false;
volatile unsigned long irqLastMicros = 0;
volatile unsigned int irqCount = 0;

void saveSignalToHistory(SavedSignal &sig) {
    // Evita duplicata consecutiva de decoded
    if (sig.type == SIG_DECODED && historyCount > 0 
        && signalHistory[0].type == SIG_DECODED
        && signalHistory[0].decodedValue == sig.decodedValue) {
        return;
    }
    
    // Empurra tudo pra frente (mais recente na posicao 0)
    for (int i = MAX_HISTORY - 1; i > 0; i--) {
        signalHistory[i] = signalHistory[i - 1];
    }
    signalHistory[0] = sig;
    if (historyCount < MAX_HISTORY) historyCount++;
}

void transmitSignal(SavedSignal &sig) {
    if (sig.type == SIG_DECODED) {
        ELECHOUSE_cc1101.setCCMode(1);
        ELECHOUSE_cc1101.setModulation(2);
        ELECHOUSE_cc1101.setMHZ(sig.freq);
        
        mySwitch.disableReceive();
        mySwitch.enableTransmit(CC1101_GDO0);
        mySwitch.setRepeatTransmit(5);
        
        noInterrupts(); // Desliga interrupções durante o envio crítico
        mySwitch.send(sig.decodedValue, sig.bitlength);
        interrupts();   // Reativa interrupções imediatamente
        
        mySwitch.disableTransmit();
        
        ELECHOUSE_cc1101.SetRx();
        mySwitch.enableReceive(digitalPinToInterrupt(CC1101_GDO0));
        
    } else if (sig.type == SIG_RAW) {
        detachInterrupt(digitalPinToInterrupt(CC1101_GDO0));
        mySwitch.disableReceive();
        
        ELECHOUSE_cc1101.setCCMode(0);
        ELECHOUSE_cc1101.setModulation(2);
        ELECHOUSE_cc1101.setMHZ(sig.freq);
        ELECHOUSE_cc1101.SetTx();
        pinMode(CC1101_GDO0, OUTPUT);
        
        for (int rep = 0; rep < 5; rep++) {
            noInterrupts(); // Desliga interrupções durante o envio bit-bang preciso
            int state = HIGH;
            for (int i = 0; i < sig.rawCount; i++) {
                digitalWrite(CC1101_GDO0, state);
                delayMicroseconds(sig.rawDurations[i]);
                state = !state;
            }
            digitalWrite(CC1101_GDO0, LOW);
            interrupts();   // Reativa interrupções para processamento de background/WiFi
            
            delay(15);
            yield();
        }
        
        // Restaura CCMode padrão (1) e escuta
        ELECHOUSE_cc1101.setCCMode(1);
        ELECHOUSE_cc1101.SetRx();
        pinMode(CC1101_GDO0, INPUT);
    }
}

bool isBtnPressed(uint8_t btn) {
    if (btn == BTN_BACK) {
        // Filtro robusto: tira 5 amostras e exige que TODAS sejam altas (>= 900).
        // Se for ruído induzido pelo rádio, o sinal flutuará e pelo menos uma amostra será baixa.
        // Se for um clique físico direto na linha de VCC, todas serão altas e estáveis.
        for (int i = 0; i < 5; i++) {
            if (analogRead(BTN_BACK) < 900) return false;
            delayMicroseconds(50);
        }
        return true;
    } else if (btn == BTN_UP) {
        if (digitalRead(BTN_UP) != HIGH) return false;
        delay(1); // Ignora ruídos transientes rápidos
        return digitalRead(BTN_UP) == HIGH;
    } else {
        if (digitalRead(btn) != LOW) return false;
        delay(2); // Ignora transições rápidas de dados seriais da UART do SDK (115200 bps = ~8.6us/bit)
        return digitalRead(btn) == LOW;
    }
}

void waitForBtnRelease(uint8_t btn) {
    if (btn == BTN_BACK) {
        // Para o botão analógico BACK, não fazemos o loop de espera do ADC.
        // Isso evita leituras extremamente rápidas que conflitam com o rádio WiFi e travam o ESP8266.
        delay(150); // Delay simples de debounce
        return;
    }
    unsigned long start = millis();
    // Timeout de 2 segundos para evitar travamento em caso de ruído contínuo
    while (isBtnPressed(btn) && (millis() - start < 2000)) {
        delay(10);
    }
    delay(50); // Debounce
}


