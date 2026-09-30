#pragma once

// --- Pinagem ESP32 (30 pinos) ---
// Baseado em docs/projeto_esp32_multiradio.md
// Barramento SPI compartilhado entre Tela, CC1101 e LoRa

// --- SPI (compartilhado) ---
#define SPI_SCK     18
#define SPI_MOSI    25
#define SPI_MISO    19

// --- Tela LCD (TFT SPI) ---
#define TFT_CS      5
#define TFT_DC      2
#define TFT_RST     4

// --- Rádio CC1101 (Sub-GHz, 433MHz) ---
#define CC1101_CS      21
#define CC1101_GDO0    22

// --- Rádio LoRa (900M22S, SX1262, SPI) ---
#define LORA_CS     14
#define LORA_RST    15
#define LORA_DIO1   13
#define LORA_BUSY   27
// DIO2 do módulo normalmente fica sem uso (controle interno de RF switch)

// --- LED onboard (D2) ---
// Atenção: é o mesmo GPIO do TFT_DC acima; quando a tela entrar, um dos dois muda.
#define LED_PIN     2
