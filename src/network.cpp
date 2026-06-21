#include "network.h"
#include "globals.h"

// ============================================================
// INCLUDE DE REDE - COMPLETAMENTE ISOLADO
// Nenhum arquivo fora deste .cpp sabe da existência do WiFi,
// HTTPClient ou EEPROM de rede. Se precisar remover a rede,
// basta deletar network.cpp e network.h.
// ============================================================
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <EEPROM.h>
#include <ArduinoJson.h>

// --- Endereços na EEPROM ---
// Usamos um bloco dedicado para não conflitar com outros dados
#define EEPROM_NET_MAGIC_ADDR   0
#define EEPROM_NET_SSID_ADDR    4
#define EEPROM_NET_PASS_ADDR    68
#define EEPROM_NET_URL_ADDR     132
#define EEPROM_NET_TOKEN_ADDR   196
#define EEPROM_AP_MODE_ADDR     260
#define EEPROM_NET_SIZE         264

// Valor mágico para saber se a EEPROM foi escrita por nós
#define EEPROM_NET_MAGIC        0xCAFE1234

// --- Estado interno ---
static NetworkStatus _status = NET_DISCONNECTED;
static String _ssid = "";
static String _password = "";
static String _apiUrl = "";
static String _apiToken = "";

// --- Helpers de EEPROM ---
static void eeprom_write_string(int addr, const char* str, int maxLen) {
    for (int i = 0; i < maxLen; i++) {
        EEPROM.write(addr + i, str[i]);
        if (str[i] == 0) break;
    }
    EEPROM.write(addr + maxLen - 1, 0); // Garante null terminator
}

static String eeprom_read_string(int addr, int maxLen) {
    String result = "";
    for (int i = 0; i < maxLen; i++) {
        char c = EEPROM.read(addr + i);
        if (c == 0) break;
        result += c;
    }
    return result;
}

// --- API Pública ---
bool net_load_credentials() {
    EEPROM.begin(EEPROM_NET_SIZE);
    
    uint32_t magic;
    EEPROM.get(EEPROM_NET_MAGIC_ADDR, magic);
    
    if (magic != EEPROM_NET_MAGIC) {
        EEPROM.end();
        return false; // Nenhuma credencial salva
    }
    
    _ssid     = eeprom_read_string(EEPROM_NET_SSID_ADDR,  64);
    _password = eeprom_read_string(EEPROM_NET_PASS_ADDR,  64);
    _apiUrl   = eeprom_read_string(EEPROM_NET_URL_ADDR,   64);
    _apiToken = eeprom_read_string(EEPROM_NET_TOKEN_ADDR, 64);
    
    EEPROM.end();
    return true;
}

void net_save_credentials(const char* ssid, const char* password, const char* apiUrl, const char* apiToken) {
    EEPROM.begin(EEPROM_NET_SIZE);
    
    uint32_t magic = EEPROM_NET_MAGIC;
    EEPROM.put(EEPROM_NET_MAGIC_ADDR, magic);
    
    eeprom_write_string(EEPROM_NET_SSID_ADDR,  ssid,     64);
    eeprom_write_string(EEPROM_NET_PASS_ADDR,  password, 64);
    eeprom_write_string(EEPROM_NET_URL_ADDR,   apiUrl,   64);
    eeprom_write_string(EEPROM_NET_TOKEN_ADDR, apiToken, 64);
    
    EEPROM.commit();
    EEPROM.end();
    
    _ssid     = ssid;
    _password = password;
    _apiUrl   = apiUrl;
    _apiToken = apiToken;
}

void net_clear_credentials() {
    EEPROM.begin(EEPROM_NET_SIZE);
    
    // Sobrescreve o magic number para indicar que não há credenciais salvas
    uint32_t magic = 0;
    EEPROM.put(EEPROM_NET_MAGIC_ADDR, magic);
    
    // Limpa os campos de string preenchendo com zeros
    for (int i = EEPROM_NET_SSID_ADDR; i < EEPROM_NET_SIZE; i++) {
        EEPROM.write(i, 0);
    }
    
    EEPROM.commit();
    EEPROM.end();
    
    _ssid     = "";
    _password = "";
    _apiUrl   = "";
    _apiToken = "";
    
    // Desconecta o WiFi e limpa dados de conexão do flash do SDK
    WiFi.disconnect(true);
    _status = NET_DISCONNECTED;
}

void net_init() {
    // Apenas configura flags — não mexe no modo WiFi
    // O SDK cuida do modo automaticamente com WiFi.begin() ou WiFi.softAP()
    WiFi.persistent(false);
    WiFi.setAutoConnect(false);
    WiFi.setAutoReconnect(false);

    if (!net_load_credentials()) {
        _status = NET_DISCONNECTED;
        return;
    }
    
    if (_ssid.isEmpty()) {
        _status = NET_DISCONNECTED;
        return;
    }
    
    _status = NET_CONNECTING;
    WiFi.begin(_ssid.c_str(), _password.c_str());
    
    // Tentativa não-bloqueante: aguarda no máximo 10 segundos
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
        delay(200);
        yield();
    }
    
    _status = (WiFi.status() == WL_CONNECTED) ? NET_CONNECTED : NET_DISCONNECTED;
}

void net_disconnect() {
    // IMPORTANTE: disconnect(true) chama enableSTA(false) que mata o rádio!
    // Usar disconnect(false) e manter o modo STA
    WiFi.disconnect(false);
    _status = NET_DISCONNECTED;
}

NetworkStatus net_status() {
    // Atualiza status real em tempo de execução
    if (_status == NET_CONNECTED && WiFi.status() != WL_CONNECTED) {
        _status = NET_DISCONNECTED;
    }
    return _status;
}

String net_ip() {
    if (net_status() == NET_CONNECTED) {
        return WiFi.localIP().toString();
    }
    return "Sem WiFi";
}

String net_get_ssid()    { return _ssid; }
String net_get_api_url() { return _apiUrl; }

bool net_get_ap_mode_flag() {
    EEPROM.begin(EEPROM_NET_SIZE);
    uint8_t flag = EEPROM.read(EEPROM_AP_MODE_ADDR);
    EEPROM.end();
    return (flag == 0xAB);
}

void net_set_ap_mode_flag(bool active) {
    EEPROM.begin(EEPROM_NET_SIZE);
    EEPROM.write(EEPROM_AP_MODE_ADDR, active ? 0xAB : 0x00);
    EEPROM.commit();
    EEPROM.end();
}

// ---------------------------------------------------------------
// POST: Envia um sinal da lista global para a API
// ---------------------------------------------------------------
bool net_push_signal(int signalIndex) {
    if (net_status() != NET_CONNECTED) return false;
    if (signalIndex < 0 || signalIndex >= historyCount) return false;

    SavedSignal& sig = signalHistory[signalIndex];

    // Monta o JSON dinamicamente com ArduinoJson
    // Capacidade para o array de RAW (300 uint16 * ~6 chars + overhead)
    DynamicJsonDocument doc(4096);
    doc["type"]         = sig.type;
    doc["freq"]         = sig.freq;
    doc["decodedValue"] = sig.decodedValue;
    doc["bitlength"]    = sig.bitlength;
    doc["rawCount"]     = sig.rawCount;
    
    if (sig.type == SIG_RAW && sig.rawCount > 0) {
        JsonArray arr = doc.createNestedArray("rawDurations");
        for (int i = 0; i < sig.rawCount; i++) {
            arr.add(sig.rawDurations[i]);
        }
    }

    String payload;
    serializeJson(doc, payload);

    BearSSL::WiFiClientSecure client;
    client.setInsecure(); // Sem validação de cert: economiza ~15KB de RAM

    HTTPClient http;
    String url = _apiUrl + "/rf/salvar";
    
    if (!http.begin(client, url)) return false;
    
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", "Bearer " + _apiToken);
    http.setTimeout(NET_API_TIMEOUT);
    
    int code = http.POST(payload);
    http.end();
    
    return (code == 200);
}

// ---------------------------------------------------------------
// GET: Baixa histórico da API e preenche o signalHistory global
// ---------------------------------------------------------------
bool net_pull_history() {
    if (net_status() != NET_CONNECTED) return false;

    BearSSL::WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    String url = _apiUrl + "/rf/historico?limit=5"; // Pede só os 5 mais recentes
    
    if (!http.begin(client, url)) return false;
    
    http.addHeader("Authorization", "Bearer " + _apiToken);
    http.setTimeout(NET_API_TIMEOUT);
    
    int code = http.GET();
    if (code != 200) {
        http.end();
        return false;
    }
    
    String response = http.getString();
    http.end();

    DynamicJsonDocument doc(8192);
    if (deserializeJson(doc, response) != DeserializationError::Ok) return false;

    JsonArray arr = doc.as<JsonArray>();
    historyCount = 0;
    
    for (JsonObject item : arr) {
        if (historyCount >= MAX_HISTORY) break;
        
        SavedSignal sig;
        sig.type         = (SignalType)(item["type"].as<int>());
        sig.freq         = item["freq"]         | 0.0;
        sig.decodedValue = item["decodedValue"] | 0;
        sig.bitlength    = item["bitlength"]    | 0;
        sig.rawCount     = item["rawCount"]     | 0;
        
        if (sig.type == SIG_RAW && sig.rawCount > 0) {
            JsonArray durations = item["rawDurations"];
            int i = 0;
            for (uint16_t d : durations) {
                if (i >= MAX_RAW_BUFFER) break;
                sig.rawDurations[i++] = d;
            }
        }
        
        signalHistory[historyCount++] = sig;
    }

    return true;
}
