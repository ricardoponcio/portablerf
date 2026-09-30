#pragma once

// Conecta o ESP32 como estação (cliente) numa rede Wi-Fi.
class WifiStation {
public:
    WifiStation(const char* ssid, const char* password) : ssid_(ssid), password_(password) {}

    // Bloqueia até conectar.
    void connect();

private:
    const char* ssid_;
    const char* password_;
};
