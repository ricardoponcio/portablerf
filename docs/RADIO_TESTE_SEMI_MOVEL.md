# Configuração de Pinos: Rádio Teste Semi-Móvel (ESP8266)

Este documento registra a pinagem final estabilizada para uso no **ESP8266 (NodeMCU)**, utilizando a Tela LCD e o Rádio CC1101 (sem LoRa).

## 1. Barramento SPI (Compartilhado)
Os pinos de comunicação principal. O MISO é exclusivo do rádio, pois a tela apenas recebe dados.
- **SCK:** `D5`
- **MOSI / SDA:** `D7` (Conectado tanto na tela quanto no rádio)
- **MISO:** `D6` (Conectado APENAS no CC1101)

## 2. Tela LCD (TFT SPI)
Pinos de controle para a interface visual.
- **CS (Chip Select):** `D8`
- **RST (Reset):** `D4`
- **A0 / DC (Data/Command):** `D3`

## 3. Rádio CC1101 (Módulo RF)
Pinos exclusivos de controle e interrupção para recepção de sinais.
- **CS (Chip Select):** `D1`
- **GDO0 (Pino de Interrupção):** `D2`

## 4. Botões (Arquitetura "À Prova de Balas" com Resistores em Série)
Para usar os pinos restantes (D0, A0, RX, TX) sem nenhum risco de queimar a placa caso o código mude, usaremos **1 Resistor de 1k EM SÉRIE** com cada botão. 
Essa montagem limita a corrente e blinda o chip contra curtos-circuitos.

A montagem física exata (na ordem do fio) é:

- **BACK (Pino A0):** `Pino A0` -> `Resistor de 1k` -> `Botão` -> **`3.3V`**
  *(Código: `analogRead(A0);` - Lê valor próximo a 1024 quando apertado)*
- **UP (Pino D0):** `Pino D0` -> `Resistor de 1k` -> `Botão` -> **`3.3V`**
  *(Código: `pinMode(D0, INPUT_PULLDOWN_16);` - Lê HIGH quando apertado)*
- **DOWN (Pino TX):** `Pino TX` -> `Resistor de 1k` -> `Botão` -> **`GND`**
  *(Código: `pinMode(TX, INPUT_PULLUP);` - Lê LOW quando apertado)*
- **OK (Pino RX):** `Pino RX` -> `LIGAÇÃO DIRETA (Sem Resistor)` -> `Botão` -> **`GND`**
  *(Código: `pinMode(RX, INPUT_PULLUP);` - Lê LOW quando apertado. Não usa resistor porque o RX briga com o chip USB).*
