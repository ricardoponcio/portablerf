#include "globals.h"
#include "wifi_config.h"
#include "network.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

static bool redrawWifi = true;
static bool apModeActive = false;

static ESP8266WebServer* configServer = nullptr;

static const char CONFIG_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html>
<head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'>
<title>PortableRF - Config WiFi</title>
<style>
  body{background:#111;color:#eee;font-family:monospace;padding:20px}
  h2{color:#00ff00}
  input{background:#222;color:#fff;border:1px solid #555;padding:8px;width:100%;box-sizing:border-box;margin:4px 0 12px}
  button{background:#00aa00;color:#fff;border:none;padding:12px 24px;cursor:pointer;width:100%;font-size:16px}
  label{font-size:12px;color:#aaa}
</style></head>
<body>
<h2>PortableRF - Config WiFi</h2>
<form action='/save' method='POST'>
  <label>SSID do WiFi</label>
  <input name='ssid' placeholder='NomeDoSeuWiFi' required>
  <label>Senha do WiFi</label>
  <input name='pass' type='password' placeholder='SenhaDoWiFi'>
  <label>URL da API (ex: https://api.seusite.com)</label>
  <input name='url' placeholder='https://...'>
  <label>Token Bearer da API</label>
  <input name='token' placeholder='sua_senha_secreta'>
  <button type='submit'>Salvar e Reiniciar</button>
</form>
</body></html>
)rawliteral";



static void startAPMode() {
    tft.fillScreen(COLOR_BG);
    tft.drawRect(0, 0, 128, 128, 0x07FF);
    
    tft.setCursor(10, 20);
    tft.setTextColor(0xFFE0);
    tft.setTextSize(1);
    tft.print("Ativando Modo AP...");
    
    tft.setCursor(10, 45);
    tft.setTextColor(COLOR_TEXT);
    tft.print("O aparelho precisa");
    tft.setCursor(10, 57);
    tft.print("reiniciar para");
    tft.setCursor(10, 69);
    tft.print("subir o radio AP.");
    
    tft.setCursor(10, 95);
    tft.setTextColor(0x07E0);
    tft.print("Reiniciando...");

    // Salva o flag do AP no EEPROM
    net_set_ap_mode_flag(true);
    
    delay(2000);
    ESP.restart();
}

void run_dedicated_ap_mode() {
    // 1) Garante que o CC1101 esta deselecionado do barramento SPI para evitar qualquer colisao fisica
    pinMode(CC1101_CS, OUTPUT);
    digitalWrite(CC1101_CS, HIGH);

    // 2) Inicializa botoes necessarios para o cancelamento manual
    pinMode(BTN_UP, INPUT_PULLDOWN_16);
    pinMode(BTN_DOWN, INPUT_PULLUP);
    pinMode(BTN_OK, INPUT_PULLUP);

    // 3) Inicializa WiFi em modo AP. Como e a PRIMEIRA coisa executada apos o boot (sem TFT e sem CC1101 iniciados em SPI ainda),
    // o radio sobe 100% limpo e estavel!
    WiFi.persistent(false);
    WiFi.setAutoConnect(false);
    WiFi.setAutoReconnect(false);
    
    WiFi.mode(WIFI_AP);
    delay(100);

    // Garante que o radio nao durma e use potencia maxima
    WiFi.setSleepMode(WIFI_NONE_SLEEP);
    WiFi.setOutputPower(20.5);

    // SSID com _AP para evitar cache do celular
    bool ok = WiFi.softAP("PortableRF_AP");

    // Aguarda o radio estabilizar (3 segundos como no teste diagnostico)
    delay(3000);

    // 4) Inicia o display TFT de forma segura, ja com o WiFi ativo
    tft.initR(INITR_144GREENTAB);
    tft.fillScreen(COLOR_BG);
    tft.drawRect(0, 0, 128, 128, 0x07FF);

    if (!ok) {
        tft.setCursor(10, 40);
        tft.setTextColor(0xF800);
        tft.print("ERRO WIFI AP!");
        tft.setCursor(10, 60);
        tft.print("Reiniciando...");
        net_set_ap_mode_flag(false);
        delay(2000);
        ESP.restart();
    }

    // 5) Inicia servidor HTTP
    configServer = new ESP8266WebServer(80);

    configServer->on("/", HTTP_GET, [&]() {
        configServer->send(200, "text/html", String(FPSTR(CONFIG_PAGE)));
    });

    configServer->on("/save", HTTP_POST, [&]() {
        String ssid  = configServer->arg("ssid");
        String pass  = configServer->arg("pass");
        String url   = configServer->arg("url");
        String token = configServer->arg("token");

        net_save_credentials(ssid.c_str(), pass.c_str(), url.c_str(), token.c_str());

        // Limpa flag de AP para o proximo boot ser normal
        net_set_ap_mode_flag(false);

        configServer->send(200, "text/html",
            "<html><body style='background:#111;color:#0f0;font-family:monospace;padding:20px'>"
            "<h2>Salvo! Reiniciando...</h2></body></html>");

        delay(1000);
        ESP.restart();
    });

    configServer->begin();

    // 6) Desenha interface visual de configuracao
    tft.fillScreen(COLOR_BG);
    tft.drawRect(0, 0, 128, 128, 0x07FF);

    tft.setCursor(15, 6);
    tft.setTextColor(0x07FF);
    tft.setTextSize(1);
    tft.print("MODO CONFIG AP");
    tft.drawLine(0, 18, 128, 18, 0x07FF);

    tft.setCursor(5, 26);
    tft.setTextColor(0x07E0);
    tft.print("AP ativo!");

    tft.setCursor(5, 42);
    tft.setTextColor(COLOR_TEXT);
    tft.print("Rede: PortableRF_AP");
    
    tft.setCursor(5, 56);
    tft.print("IP: 192.168.4.1");

    tft.setCursor(5, 75);
    tft.setTextColor(0xFFE0);
    tft.print("Acesse no celular");
    tft.setCursor(5, 87);
    tft.print("para configurar");

    tft.drawLine(0, 110, 128, 110, 0x07FF);
    tft.setCursor(5, 116);
    tft.setTextColor(0xF800);
    tft.print("BACK = Cancelar");

    // 7) Loop dedicado e infinito do Modo AP
    unsigned long lastFlash = 0;
    bool ledOn = false;
    
    while (true) {
        configServer->handleClient();
        
        // Pequena animacao pulsante para indicar que o aparelho esta rodando
        if (millis() - lastFlash > 1000) {
            lastFlash = millis();
            ledOn = !ledOn;
            tft.fillRect(115, 26, 6, 6, ledOn ? 0x07E0 : COLOR_BG);
        }

        // Verifica se o botao BACK foi pressionado para cancelar
        if (isBtnPressed(BTN_BACK)) {
            waitForBtnRelease(BTN_BACK);
            
            tft.fillScreen(COLOR_BG);
            tft.setCursor(10, 50);
            tft.setTextColor(0xFFE0);
            tft.print("Cancelando AP...");
            tft.setCursor(10, 70);
            tft.print("Reiniciando...");
            
            // Limpa flag de AP
            net_set_ap_mode_flag(false);
            delay(1000);
            ESP.restart();
        }
        
        delay(20);
    }
}

static void stopAPMode() {
    if (configServer) {
        configServer->stop();
        delete configServer;
        configServer = nullptr;
    }

    // IMPORTANTE: softAPdisconnect(true) chama enableAP(false) que,
    // quando o modo é WIFI_AP, faz mode(AP & ~AP) = mode(0) = WIFI_OFF!
    // Primeiro troca para STA, depois desliga o AP com false.
    WiFi.mode(WIFI_STA);
    WiFi.softAPdisconnect(false);

    // Restaura o CC1101 do sleep com reconfiguração completa dos pinos SPI
    ELECHOUSE_cc1101.setSpiPin(14, 12, 13, CC1101_CS);
    ELECHOUSE_cc1101.Init();
    ELECHOUSE_cc1101.setCCMode(1);
    ELECHOUSE_cc1101.setModulation(2);

    apModeActive = false;
}

static void drawWifiUI() {
    tft.fillScreen(COLOR_BG);
    tft.drawRect(0, 0, 128, 128, 0x07FF);

    tft.setCursor(10, 6);
    tft.setTextColor(0x07FF);
    tft.setTextSize(1);
    tft.print("CONFIG WIFI");
    tft.drawLine(0, 18, 128, 18, 0x07FF);

    NetworkStatus ns = net_status();

    tft.setCursor(5, 26);
    if (ns == NET_CONNECTED) {
        tft.setTextColor(0x07E0);
        tft.print("Conectado!");
        tft.setCursor(5, 40);
        tft.setTextColor(COLOR_TEXT);
        tft.print(net_ip());
        tft.setCursor(5, 54);
        tft.print(net_get_ssid());
    } else if (apModeActive) {
        tft.setTextColor(0xFFE0);
        tft.print("Modo Config AP");
        tft.setCursor(5, 40);
        tft.setTextColor(COLOR_TEXT);
        tft.print("Rede: PortableRF_AP");
        tft.setCursor(5, 54);
        tft.print("(aberta)");
        tft.setCursor(5, 68);
        tft.print("-> 192.168.4.1");
    } else {
        tft.setTextColor(0xF800);
        tft.print("Sem WiFi");
        tft.setCursor(5, 40);
        tft.setTextColor(COLOR_TEXT);
        if (!net_get_ssid().isEmpty()) {
            tft.print(net_get_ssid());
            tft.setCursor(5, 54);
            tft.print("Nao conectou.");
        } else {
            tft.print("Nao configurado");
        }
    }

    tft.drawLine(0, 110, 128, 110, 0x07FF);
    tft.setCursor(2, 116);
    tft.setTextColor(0x07FF);
    if (apModeActive) {
        tft.print("BACK=Fechar AP");
    } else if (ns == NET_CONNECTED) {
        tft.print("UP=Push OK=Pull");
    } else {
        tft.print("UP=Config AP");
    }
}

void wifi_config_setup() {
    mySwitch.disableReceive();
    redrawWifi = true;
    apModeActive = false;
}

void wifi_config_loop() {
    if (apModeActive && configServer) {
        configServer->handleClient();
        yield();
    }

    if (redrawWifi) {
        drawWifiUI();
        redrawWifi = false;
    }

    if (isBtnPressed(BTN_BACK)) {
        waitForBtnRelease(BTN_BACK);
        if (apModeActive) {
            stopAPMode();
        }
        currentState = STATE_MENU;
        return;
    }

    if (isBtnPressed(BTN_UP)) {
        waitForBtnRelease(BTN_UP);

        if (net_status() == NET_CONNECTED) {
            // Conectado: UP faz push do sinal mais recente
            tft.fillRect(0, 111, 128, 16, COLOR_BG);
            tft.setCursor(2, 116);
            tft.setTextColor(0xFFE0);
            tft.print("Enviando...");

            bool ok = (historyCount > 0) ? net_push_signal(0) : false;
            tft.fillRect(0, 111, 128, 16, COLOR_BG);
            tft.setCursor(2, 116);
            tft.setTextColor(ok ? 0x07E0 : 0xF800);
            tft.print(ok ? "Enviado!" : (historyCount == 0 ? "Lista vazia!" : "Falhou!"));
            delay(1000);
        } else {
            // Sem WiFi: UP liga/desliga o modo AP
            if (apModeActive) {
                stopAPMode();
            } else {
                startAPMode();
            }
        }
        redrawWifi = true;
    }

    if (isBtnPressed(BTN_OK)) {
        waitForBtnRelease(BTN_OK);

        if (net_status() == NET_CONNECTED) {
            tft.fillRect(0, 111, 128, 16, COLOR_BG);
            tft.setCursor(2, 116);
            tft.setTextColor(0xFFE0);
            tft.print("Buscando...");

            bool ok = net_pull_history();
            tft.fillRect(0, 111, 128, 16, COLOR_BG);
            tft.setCursor(2, 116);
            tft.setTextColor(ok ? 0x07E0 : 0xF800);
            tft.print(ok ? "Sync OK!" : "Falhou!");
            delay(1000);
        } else if (apModeActive) {
            stopAPMode();
        }
        redrawWifi = true;
    }
}
