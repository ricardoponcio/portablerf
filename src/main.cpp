#include <Arduino.h>
#include "config.h"
#include "pins.h"
#include "spi_bus.h"
#include "event_log.h"
#include "sniffer.h"
#include "cc1101_radio.h"
#include "lora_radio.h"
#include "auto_ping.h"
#include "wifi_station.h"
#include "web_ui.h"
#include "meshtastic.h"
#include "status_led.h"

EventLog eventLog;
Sniffer cc1101Sniff;
Sniffer loraSniff;
Cc1101Radio cc1101(eventLog, cc1101Sniff, CC1101_FREQ_MHZ);
LoRaRadio lora(eventLog, loraSniff, {LORA_FREQ_MHZ, LORA_BW_KHZ, LORA_SF, LORA_CR, LORA_SYNC_WORD, LORA_PREAMBLE},
               LORA_TCXO_V);
AutoPing cc1101Ping(cc1101);
AutoPing loraPing(lora);

WifiStation wifi(WIFI_NETWORKS);
MeshDecoder mesh;
WebUi web(eventLog, {cc1101, cc1101Ping, cc1101Sniff}, {lora, loraPing, loraSniff}, lora, mesh);
StatusLed led(LED_PIN, LED_BLINK_MS);

void setup() {
    Serial.begin(115200);
    delay(500);
    led.begin();

    wifi.connect();
    web.begin();  // antes dos rádios: a página abre mesmo se algum falhar

    SpiBus::begin();
    cc1101.begin();
    lora.beginAsync();
}

void loop() {
    web.handle();

    cc1101.poll();
    lora.poll();
    cc1101Ping.poll();
    loraPing.poll();

    led.update();
}
