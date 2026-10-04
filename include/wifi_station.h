#pragma once

// Conecta o ESP32 como estação (cliente) numa rede Wi-Fi.
class WifiStation {
public:
    WifiStation(const char* ssid, const char* password) : ssid_(ssid), password_(password) {}

    // Bloqueia até conectar. Cada tentativa tem WIFI_CONNECT_TIMEOUT_MS; após
    // WIFI_CONNECT_ATTEMPTS tentativas sem sucesso, reinicia o ESP32.
    void connect();

private:
    bool tryConnect(unsigned long timeoutMs);

    const char* ssid_;
    const char* password_;
};
