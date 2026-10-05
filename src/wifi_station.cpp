#include "wifi_station.h"
#include <WiFi.h>
#include "config.h"

// Scan passivo-curto: o padrão (~7 s) atrasava o boot; 120 ms por canal basta para achar o AP
static constexpr uint32_t SCAN_MS_PER_CHANNEL = 120;

void WifiStation::connect() {
    Serial.println("\n--- Conectando a rede Wi-Fi ---");
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);  // sem power save: página e ping respondem na hora

    Candidate list[count_];
    for (int round = 1; round <= WIFI_CONNECT_ATTEMPTS; round++) {
        size_t n = candidates(list);
        for (size_t i = 0; i < n; i++) {
            const WifiCredential& net = networks_[list[i].index];
            Serial.printf("Conectando em %s (rodada %d/%d)", net.ssid, round, WIFI_CONNECT_ATTEMPTS);
            if (tryConnect(list[i], WIFI_CONNECT_TIMEOUT_MS)) {
                Serial.println("\nWi-Fi Conectado!");
                Serial.print("Endereço IP obtido: ");
                Serial.println(WiFi.localIP());
                return;
            }
            Serial.printf("\nSem conexao apos %d s (status %d).\n", WIFI_CONNECT_TIMEOUT_MS / 1000, WiFi.status());
        }
    }

    Serial.println("Wi-Fi nao conectou, reiniciando a placa...");
    delay(100);  // deixa a serial esvaziar
    ESP.restart();
}

size_t WifiStation::candidates(Candidate* out) {
    WiFi.disconnect();  // o scan não roda direito com uma tentativa de conexão pendente
    int found = WiFi.scanNetworks(false, false, false, SCAN_MS_PER_CHANNEL);
    size_t n = 0;
    // Redes conhecidas que estão no ar, da mais forte para a mais fraca (o scan já vem
    // ordenado por RSSI). Guarda canal e BSSID: conectar direto no AP achado evita que o
    // begin() faça outro scan por conta própria (a 1ª tentativa após o scan falhava assim).
    for (int s = 0; s < found; s++) {
        for (size_t k = 0; k < count_; k++) {
            bool already = false;
            for (size_t j = 0; j < n; j++) already |= out[j].index == k;
            if (!already && WiFi.SSID(s) == networks_[k].ssid) {
                Serial.printf("Rede conhecida no ar: %s (%d dBm, canal %ld)\n", networks_[k].ssid, WiFi.RSSI(s),
                              (long)WiFi.channel(s));
                out[n].index = k;
                out[n].channel = WiFi.channel(s);
                memcpy(out[n].bssid, WiFi.BSSID(s), 6);
                n++;
            }
        }
    }
    WiFi.scanDelete();
    if (n > 0) return n;

    // Nenhuma apareceu (rede oculta, scan falhou...): tenta todas na ordem da lista
    Serial.printf("Scan nao achou redes conhecidas (%d redes no ar), tentando todas.\n", found);
    for (size_t k = 0; k < count_; k++) out[k] = {k, 0, {0}};
    return count_;
}

bool WifiStation::tryConnect(const Candidate& c, unsigned long timeoutMs) {
    const WifiCredential& net = networks_[c.index];
    // Derruba a tentativa anterior (se houver) para o driver começar do zero
    WiFi.disconnect();
    delay(100);
    if (c.channel) WiFi.begin(net.ssid, net.password, c.channel, c.bssid);
    else WiFi.begin(net.ssid, net.password);

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - start >= timeoutMs) return false;
        delay(500);
        Serial.print(".");
    }
    return true;
}
