#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <SPI.h>
#include <ELECHOUSE_CC1101_SRC_DRV.h>
#include <RadioLib.h>
#include "pins.h"

// Definindo o pino do LED (LED_BUILTIN normalmente é o pino 2 na maioria das placas ESP32)
#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

// --- Rádios (homologação de solda) ---
#define CC1101_FREQ_MHZ 433.0
#define LORA_FREQ_MHZ   915.0
// Este 900M22S usa cristal comum, sem TCXO: precisa ser 0 (o default do RadioLib, 1.6V,
// liga o DIO3 como fonte de TCXO e a calibração falha com XOSC_START_ERR / -707)
#define LORA_TCXO_V     0

// Passa o SPI explicitamente: o construtor padrão chamaria SPI.begin() com os pinos
// default (MOSI=23), e aqui o MOSI está no GPIO 25.
SX1262 loraRadio = new Module(LORA_CS, LORA_DIO1, LORA_RST, LORA_BUSY, SPI, RADIOLIB_DEFAULT_SPI_SETTINGS);

bool cc1101Detected = false;
volatile int loraInitState = RADIOLIB_ERR_UNKNOWN;
// begin() do SX1262 pode travar ~20s se o chip não responder (timeouts do BUSY),
// então roda numa task separada para não segurar o servidor web e o LED.
// Enquanto for false, o LoRa está usando o SPI e o CC1101 não deve ser acessado.
volatile bool loraInitDone = false;

void initCC1101() {
    ELECHOUSE_cc1101.setSpiPin(SPI_SCK, SPI_MISO, SPI_MOSI, CC1101_CS);
    ELECHOUSE_cc1101.setGDO0(CC1101_GDO0);
    ELECHOUSE_cc1101.Init();
    cc1101Detected = ELECHOUSE_cc1101.getCC1101();
    if (cc1101Detected) {
        ELECHOUSE_cc1101.setMHZ(CC1101_FREQ_MHZ);
        ELECHOUSE_cc1101.setCCMode(1);
        ELECHOUSE_cc1101.setModulation(2);
        ELECHOUSE_cc1101.SetRx();
    }
}

void loraInitTask(void*) {
    loraInitState = loraRadio.begin(LORA_FREQ_MHZ, 125.0, 9, 7, RADIOLIB_SX126X_SYNC_WORD_PRIVATE, 10, 8, LORA_TCXO_V);
    Serial.println(loraInitState == RADIOLIB_ERR_NONE ? "LoRa (SX1262) inicializado." : "Falha ao inicializar LoRa, codigo: " + String(loraInitState));
    loraInitDone = true;
    vTaskDelete(NULL);
}

const int ledPin = LED_BUILTIN;

// Credenciais pré-fixadas da sua rede Wi-Fi (Substitua pelos dados da sua rede)
const char* ssid = "4Work";
const char* password = "delorean_1058";

// Servidor Web na porta 80
WebServer server(80);

// Variáveis para controle do LED piscando sem bloquear o loop (Non-blocking Blink)
unsigned long previousMillis = 0;
const long interval = 500; // Intervalo de 500ms (1Hz)
bool ledState = LOW;

// Função para gerar o HTML da página dinamicamente com informações da conexão
String getHTMLPage() {
    String html = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Teste - Status Wi-Fi</title>
    <style>
        :root {
            --bg-color: #0f172a;
            --card-bg: #1e293b;
            --primary: #38bdf8;
            --text: #f8fafc;
            --text-secondary: #94a3b8;
            --success: #22c55e;
        }
        * { box-sizing: border-box; margin: 0; padding: 0; font-family: 'Segoe UI', sans-serif; }
        body { background-color: var(--bg-color); color: var(--text); display: flex; justify-content: center; align-items: center; min-height: 100vh; padding: 20px; }
        .container { background-color: var(--card-bg); padding: 30px; border-radius: 16px; box-shadow: 0 10px 25px rgba(0, 0, 0, 0.5); max-width: 420px; width: 100%; border: 1px solid #334155; }
        h1 { color: var(--primary); font-size: 1.6rem; margin-bottom: 8px; text-align: center; }
        p.subtitle { color: var(--text-secondary); font-size: 0.9rem; text-align: center; margin-bottom: 24px; }
        .info-card { background-color: #0f172a; border-radius: 8px; padding: 15px; margin-bottom: 20px; border-left: 4px solid var(--primary); }
        .info-item { display: flex; justify-content: space-between; margin-bottom: 8px; font-size: 0.95rem; }
        .info-item:last-child { margin-bottom: 0; }
        .label { color: var(--text-secondary); }
        .value { font-weight: bold; color: var(--text); }
        .status-badge { display: inline-block; background-color: rgba(34, 197, 94, 0.2); color: var(--success); padding: 6px 14px; border-radius: 12px; font-size: 0.85rem; font-weight: 600; }
        .status-container { text-align: center; margin-top: 15px; }
        .radio-badge { padding: 3px 10px; border-radius: 10px; font-size: 0.8rem; font-weight: 600; }
        .radio-ok { background-color: rgba(34, 197, 94, 0.2); color: var(--success); }
        .radio-fail { background-color: rgba(239, 68, 68, 0.2); color: #ef4444; }
    </style>
</head>
<body>
    <div class="container">
        <h1>ESP32 Control Hub</h1>
        <p class="subtitle">Modo Estação (Conectado à Rede Wi-Fi)</p>

        <div class="info-card">
            <div class="info-item">
                <span class="label">Rede Wi-Fi (SSID):</span>
                <span class="value">)rawliteral";
    html += String(ssid);
    html += R"rawliteral(</span>
            </div>
            <div class="info-item">
                <span class="label">Endereço IP Local:</span>
                <span class="value">)rawliteral";
    html += WiFi.localIP().toString();
    html += R"rawliteral(</span>
            </div>
            <div class="info-item">
                <span class="label">Sinal RSSI:</span>
                <span class="value">)rawliteral";
    html += String(WiFi.RSSI()) + " dBm";
    html += R"rawliteral(</span>
            </div>
            <div class="info-item">
                <span class="label">Endereço MAC:</span>
                <span class="value">)rawliteral";
    html += WiFi.macAddress();
    html += R"rawliteral(</span>
            </div>
        </div>

        <div class="status-container">
            <span class="status-badge">● LED Piscando (1Hz) | HTTP Server Ativo</span>
        </div>

        <div class="info-card" style="margin-top:20px;">
            <div class="info-item">
                <span class="label">CC1101 (433MHz)</span>
                <span class="value"><span id="cc1101-badge" class="radio-badge">...</span></span>
            </div>
            <div class="info-item">
                <span class="label">RSSI CC1101</span>
                <span class="value" id="cc1101-rssi">-- dBm</span>
            </div>
            <div class="info-item">
                <span class="label">LoRa SX1262 (915MHz)</span>
                <span class="value"><span id="lora-badge" class="radio-badge">...</span></span>
            </div>
        </div>
    </div>
    <script>
        function refreshStatus() {
            fetch('/api/status').then(r => r.json()).then(d => {
                const cc = document.getElementById('cc1101-badge');
                cc.textContent = d.cc1101_ok ? 'OK' : 'NÃO DETECTADO';
                cc.className = 'radio-badge ' + (d.cc1101_ok ? 'radio-ok' : 'radio-fail');
                document.getElementById('cc1101-rssi').textContent = d.cc1101_ok ? (d.cc1101_rssi + ' dBm') : '--';

                const lo = document.getElementById('lora-badge');
                lo.textContent = d.lora_pending ? 'INICIALIZANDO...' : (d.lora_ok ? 'OK' : ('FALHA (' + d.lora_state + ')'));
                lo.className = 'radio-badge ' + (d.lora_pending ? '' : (d.lora_ok ? 'radio-ok' : 'radio-fail'));
            });
        }
        refreshStatus();
        setInterval(refreshStatus, 1500);
    </script>
</body>
</html>
)rawliteral";
    return html;
}

void handleRoot() {
    server.send(200, "text/html", getHTMLPage());
}

void handleStatus() {
    int rssi = (cc1101Detected && loraInitDone) ? ELECHOUSE_cc1101.getRssi() : 0;
    String json = "{";
    json += "\"cc1101_ok\":" + String(cc1101Detected ? "true" : "false") + ",";
    json += "\"cc1101_rssi\":" + String(rssi) + ",";
    json += "\"lora_pending\":" + String(loraInitDone ? "false" : "true") + ",";
    json += "\"lora_ok\":" + String(loraInitState == RADIOLIB_ERR_NONE ? "true" : "false") + ",";
    json += "\"lora_state\":" + String(loraInitState);
    json += "}";
    server.send(200, "application/json", json);
}

void setup() {
    pinMode(ledPin, OUTPUT);

    Serial.begin(115200);
    delay(500);
    Serial.println("\n--- Conectando a rede Wi-Fi ---");

    // Configura o ESP32 no modo Estação (STA)
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false); // sem power save: página e ping respondem na hora
    WiFi.begin(ssid, password);

    // Tenta conectar à rede Wi-Fi
    Serial.print("Conectando em ");
    Serial.print(ssid);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nWi-Fi Conectado!");
    Serial.print("Endereço IP obtido: ");
    Serial.println(WiFi.localIP());

    // Configura rotas da página web
    server.on("/", handleRoot);
    server.on("/api/status", handleStatus);
    server.begin();
    Serial.println("Servidor HTTP iniciado.");

    // Inicializa o barramento SPI compartilhado e os rádios
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
    initCC1101();
    Serial.println(cc1101Detected ? "CC1101 detectado." : "CC1101 NAO detectado.");
    xTaskCreatePinnedToCore(loraInitTask, "loraInit", 4096, NULL, 1, NULL, 0);
}

void loop() {
    // Trata requisições web
    server.handleClient();

    // Blink do LED sem bloquear (Non-blocking)
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        ledState = !ledState;
        digitalWrite(ledPin, ledState);
    }
}