#pragma once
#include "wifi_station.h"

// --- Wi-Fi ---
// Redes conhecidas. A cada rodada faz um scan e tenta as que estão no ar, da mais forte
// para a mais fraca; se o scan não achar nenhuma (ex.: rede oculta), tenta todas na ordem.
static const WifiCredential WIFI_NETWORKS[] = {
    {"A54 de Ricardo", "delorean_1058"},
    {"4Work", "delorean_1058"},
    // {"OutraRede", "senha"},
};
// Se a placa reinicia enquanto estava conectada, o roteador ainda acha que a sessão antiga
// existe e recusa a nova (motivo 202, AUTH_FAIL) até ficar ~15 s sem tentativas; insistir
// antes disso reinicia a contagem dele. Então: uma tentativa por rede com este timeout,
// depois a próxima; esgotadas as rodadas, reinicia a placa.
#define WIFI_CONNECT_TIMEOUT_MS 15000
#define WIFI_CONNECT_ATTEMPTS   4   // rodadas pela lista inteira

// --- Rádios ---
#define CC1101_FREQ_MHZ 433.92
// Configuração inicial do LoRa (modo "placa-placa"); a página troca em tempo de execução
#define LORA_FREQ_MHZ   915.0
#define LORA_BW_KHZ     125.0
#define LORA_SF         9
#define LORA_CR         7       // 4/7
#define LORA_SYNC_WORD  0x12    // privada (RadioLib); LoRaWAN usa 0x34, Meshtastic 0x2B
#define LORA_PREAMBLE   8
// Este 900M22S usa cristal comum, sem TCXO: precisa ser 0 (o default do RadioLib, 1.6V,
// liga o DIO3 como fonte de TCXO e a calibração falha com XOSC_START_ERR / -707)
#define LORA_TCXO_V     0

// FIFO do CC1101 tem 64 bytes: 1 de tamanho + payload + 2 de status → payload máx. 61.
// Usa o mesmo limite no LoRa para as mensagens serem intercambiáveis entre os testes.
#define MAX_MSG_LEN     60

// --- Interface ---
#define LED_BLINK_MS    500
#define EVENT_LOG_SIZE  40
// Bytes guardados por pacote no log: o LoRa recebe até 255 (LoRaWAN, Meshtastic...)
#define EVENT_MAX_BYTES 255

// --- Sniff (gráfico de RSSI do CC1101) ---
// Pico de RSSI por janela; SNIFF_SIZE janelas = 10 s de histórico
#define SNIFF_BUCKET_MS 20
#define SNIFF_SIZE      500
