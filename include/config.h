#pragma once

// --- Wi-Fi ---
#define WIFI_SSID       "4Work"
#define WIFI_PASSWORD   "delorean_1058"

// --- Rádios ---
#define CC1101_FREQ_MHZ 433.92
#define LORA_FREQ_MHZ   915.0
// Este 900M22S usa cristal comum, sem TCXO: precisa ser 0 (o default do RadioLib, 1.6V,
// liga o DIO3 como fonte de TCXO e a calibração falha com XOSC_START_ERR / -707)
#define LORA_TCXO_V     0

// FIFO do CC1101 tem 64 bytes: 1 de tamanho + payload + 2 de status → payload máx. 61.
// Usa o mesmo limite no LoRa para as mensagens serem intercambiáveis entre os testes.
#define MAX_MSG_LEN     60

// --- Interface ---
#define LED_BLINK_MS    500
#define EVENT_LOG_SIZE  40
