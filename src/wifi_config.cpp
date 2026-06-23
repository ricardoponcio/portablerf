#include "globals.h"
#include "wifi_config.h"
#include "network.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SPI.h>

static bool redrawWifi = true;
static bool apModeActive = false;
static String apPassword = "";
static bool isConfirmingClear = false;

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
    
    tft.setCursor(10, 25);
    tft.setTextColor(0xFFE0);
    tft.setTextSize(1);
    tft.print("Iniciando AP...");
    tft.setCursor(10, 50);
    tft.setTextColor(COLOR_TEXT);
    tft.print("Aguarde...");

    // 1) Silencia hardware e desliga interrupções
    detachInterrupt(digitalPinToInterrupt(CC1101_GDO0));
    mySwitch.disableReceive();

    // 2) Coloca CC1101 em sleep e deseleciona da linha de chip select
    ELECHOUSE_cc1101.setSidle();
    ELECHOUSE_cc1101.goSleep();
    digitalWrite(CC1101_CS, HIGH);
    delay(100);

    // 3) Reseta o rádio WiFi completamente antes de iniciar o AP
    WiFi.disconnect(true);
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_OFF);
    delay(500); // Aguarda a pilha limpar

    WiFi.persistent(false);
    WiFi.setAutoConnect(false);
    WiFi.setAutoReconnect(false);
    
    WiFi.mode(WIFI_AP);
    delay(200);
    
    WiFi.setSleepMode(WIFI_NONE_SLEEP);
    WiFi.setOutputPower(20.5);
    
    // Gera senha aleatória de 8 dígitos (padrão WPA2 exige mínimo de 8 caracteres)
    randomSeed(micros());
    apPassword = String(random(10000000, 99999999));
    bool ok = WiFi.softAP("PortableRF_AP", apPassword.c_str());

    // 4) Aguarda 1 segundo para estabilização de transmissão
    delay(1000);

    if (!ok) {
        tft.fillScreen(COLOR_BG);
        tft.setCursor(10, 40);
        tft.setTextColor(0xF800);
        tft.print("AP FALHOU!");
        delay(2000);
        
        // Se falhar, restaura o hardware para o menu
        ELECHOUSE_cc1101.setSpiPin(14, 12, 13, CC1101_CS);
        ELECHOUSE_cc1101.Init();
        ELECHOUSE_cc1101.setCCMode(1);
        ELECHOUSE_cc1101.setModulation(2);
        return;
    }

    // 5) Inicializa servidor HTTP
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

        configServer->send(200, "text/html",
            "<html><body style='background:#111;color:#0f0;font-family:monospace;padding:20px'>"
            "<h2>Salvo! Reiniciando...</h2></body></html>");

        delay(1000);
        ESP.restart();
    });

    configServer->begin();
    apModeActive = true;
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

    // Reinicia a conexão STA de WiFi em background
    net_init();
}

static void drawConfirmClearUI() {
    tft.fillScreen(COLOR_BG);
    tft.drawRect(0, 0, 128, 128, 0xF800); // Borda vermelha de aviso

    tft.setCursor(10, 6);
    tft.setTextColor(0xF800);
    tft.setTextSize(1);
    tft.print("LIMPAR CONFIG");
    tft.drawLine(0, 18, 128, 18, 0xF800);

    tft.setTextColor(COLOR_TEXT);
    tft.setCursor(5, 30);
    tft.print("Deseja apagar as");
    tft.setCursor(5, 42);
    tft.print("credenciais WiFi");
    tft.setCursor(5, 54);
    tft.print("salvas no device?");

    tft.setTextColor(0xFFE0);
    tft.setCursor(5, 78);
    tft.print("Confirma exclusao?");
    
    tft.drawLine(0, 110, 128, 110, 0xF800);
    tft.setCursor(2, 116);
    tft.setTextColor(0x07FF);
    tft.print("OK=Confirm  BACK=Canc");
}

static void drawWifiUI() {
    if (isConfirmingClear) {
        drawConfirmClearUI();
        return;
    }
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
    } else if (ns == NET_CONNECTING) {
        tft.setTextColor(0xFFE0); // Amarelo
        tft.print("Conectando...");
        tft.setCursor(5, 40);
        tft.setTextColor(COLOR_TEXT);
        tft.print(net_get_ssid());
        tft.setCursor(5, 54);
        tft.print("Aguarde...");
    } else if (apModeActive) {
        tft.setTextColor(0xFFE0);
        tft.print("Modo Config AP");
        tft.setCursor(5, 40);
        tft.setTextColor(COLOR_TEXT);
        tft.print("Rede: PortableRF_AP");
        tft.setCursor(5, 54);
        tft.print("Senha: " + apPassword);
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
        tft.print("UP=Push OK=Pull DN=Rst");
    } else {
        if (!net_get_ssid().isEmpty()) {
            tft.print("UP=Config AP  DN=Reset");
        } else {
            tft.print("UP=Config AP");
        }
    }
}

void wifi_config_setup() {
    mySwitch.disableReceive();
    redrawWifi = true;
    apModeActive = false;
    isConfirmingClear = false;
}

void wifi_config_loop() {
    // Atualiza a tela automaticamente se o status de rede mudar
    static NetworkStatus lastNetStatus = NET_DISCONNECTED;
    NetworkStatus currentNetStatus = net_status();
    if (currentNetStatus != lastNetStatus) {
        lastNetStatus = currentNetStatus;
        redrawWifi = true;
    }

    if (isConfirmingClear) {
        if (redrawWifi) {
            drawConfirmClearUI();
            redrawWifi = false;
        }

        if (isBtnPressed(BTN_OK)) {
            waitForBtnRelease(BTN_OK);
            tft.fillScreen(COLOR_BG);
            tft.drawRect(0, 0, 128, 128, 0xF800);
            tft.setCursor(10, 45);
            tft.setTextColor(0xF800);
            tft.setTextSize(1);
            tft.print("Limpando configs...");
            tft.setCursor(10, 65);
            tft.print("Reiniciando...");
            
            net_clear_credentials();
            delay(1500);
            ESP.restart();
        }

        if (isBtnPressed(BTN_BACK)) {
            waitForBtnRelease(BTN_BACK);
            isConfirmingClear = false;
            redrawWifi = true;
        }
        return; // Ignora o resto se estiver confirmando clear
    }

    if (apModeActive && configServer) {
        configServer->handleClient();
        delay(20);
    }

    if (redrawWifi) {
        drawWifiUI();
        redrawWifi = false;
    }

    // 1) Botões digitais checados a cada ciclo (sem rate-limit, pois digitalRead não interfere no WiFi)
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
        } else if (!apModeActive) {
            // Sem WiFi e AP inativo: UP inicia o modo AP
            startAPMode();
        }
        redrawWifi = true;
        return;
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
        }
        redrawWifi = true;
        return;
    }

    if (isBtnPressed(BTN_DOWN)) {
        waitForBtnRelease(BTN_DOWN);
        if (!apModeActive && !net_get_ssid().isEmpty()) {
            isConfirmingClear = true;
            redrawWifi = true;
        }
        return;
    }

    // 2) Botão analógico BACK checado apenas a cada 100ms para proteger o ADC do WiFi
    static unsigned long lastButtonPoll = 0;
    if (millis() - lastButtonPoll >= 100) {
        lastButtonPoll = millis();
        if (isBtnPressed(BTN_BACK)) {
            waitForBtnRelease(BTN_BACK);
            if (apModeActive) {
                stopAPMode();
            }
            currentState = STATE_MENU;
            return;
        }
    }
}
