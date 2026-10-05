#pragma once
#include <stddef.h>
#include <stdint.h>

struct WifiCredential {
    const char* ssid;
    const char* password;
};

// Conecta o ESP32 como estação (cliente) numa das redes Wi-Fi conhecidas.
class WifiStation {
public:
    template <size_t N>
    explicit WifiStation(const WifiCredential (&networks)[N]) : networks_(networks), count_(N) {}

    // Bloqueia até conectar. Cada tentativa tem WIFI_CONNECT_TIMEOUT_MS; após
    // WIFI_CONNECT_ATTEMPTS rodadas pela lista sem sucesso, reinicia o ESP32.
    void connect();

private:
    // Uma rede a tentar; channel = 0 quando não veio do scan (o driver procura sozinho)
    struct Candidate {
        size_t index;
        int32_t channel;
        uint8_t bssid[6];
    };

    // Redes a tentar nesta rodada, na ordem; devolve quantas
    size_t candidates(Candidate* out);
    bool tryConnect(const Candidate& c, unsigned long timeoutMs);

    const WifiCredential* networks_;
    size_t count_;
};
