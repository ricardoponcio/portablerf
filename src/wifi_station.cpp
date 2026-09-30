#include "wifi_station.h"
#include <WiFi.h>

void WifiStation::connect() {
    Serial.println("\n--- Conectando a rede Wi-Fi ---");
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);  // sem power save: página e ping respondem na hora
    WiFi.begin(ssid_, password_);

    Serial.print("Conectando em ");
    Serial.print(ssid_);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWi-Fi Conectado!");
    Serial.print("Endereço IP obtido: ");
    Serial.println(WiFi.localIP());
}
