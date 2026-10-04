#include "wifi_station.h"
#include <WiFi.h>
#include "config.h"

void WifiStation::connect() {
    Serial.println("\n--- Conectando a rede Wi-Fi ---");
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);  // sem power save: página e ping respondem na hora

    for (int attempt = 1; attempt <= WIFI_CONNECT_ATTEMPTS; attempt++) {
        Serial.printf("Conectando em %s (tentativa %d/%d)", ssid_, attempt, WIFI_CONNECT_ATTEMPTS);
        if (tryConnect(WIFI_CONNECT_TIMEOUT_MS)) {
            Serial.println("\nWi-Fi Conectado!");
            Serial.print("Endereço IP obtido: ");
            Serial.println(WiFi.localIP());
            return;
        }
        Serial.printf("\nSem conexao apos %d s (status %d).\n", WIFI_CONNECT_TIMEOUT_MS / 1000, WiFi.status());
    }

    Serial.println("Wi-Fi nao conectou, reiniciando a placa...");
    delay(100);  // deixa a serial esvaziar
    ESP.restart();
}

bool WifiStation::tryConnect(unsigned long timeoutMs) {
    // Derruba a tentativa anterior (se houver) para o driver começar do zero
    WiFi.disconnect();
    delay(100);
    WiFi.begin(ssid_, password_);

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - start >= timeoutMs) return false;
        delay(500);
        Serial.print(".");
    }
    return true;
}
