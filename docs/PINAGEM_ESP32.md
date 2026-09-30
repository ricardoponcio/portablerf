# Pinagem ESP32: CC1101 + LoRa 900M22S (SX1262)

Pinagem em uso na branch `esp32-test`, sem tela TFT por enquanto (ESP32 + CC1101 + LoRa apenas). Definições em `include/pins.h`.

## Barramento SPI (compartilhado)

| Sinal | GPIO ESP32 | CC1101 | LoRa 900M22S (SX1262) |
| :--- | :--- | :--- | :--- |
| SCK  | GPIO 18 | SCK | SCK |
| MOSI | GPIO 25 | SI  | MOSI |
| MISO | GPIO 19 | SO  | MISO |

## CC1101 (exclusivos)

| Sinal | GPIO ESP32 | Pino no CC1101 |
| :--- | :--- | :--- |
| CS (Chip Select) | GPIO 21 | CSN |
| GDO0 (interrupção) | GPIO 22 | GDO0 |
| VCC | 3.3V | VCC |
| GND | GND | GND |

*(GDO2 do CC1101 fica sem uso — não precisa conectar.)*

## LoRa 900M22S – SX1262 (exclusivos)

| Sinal | GPIO ESP32 | Pino no LoRa |
| :--- | :--- | :--- |
| CS | GPIO 14 | NSS |
| RESET | GPIO 15 | RESET |
| DIO1 (interrupção) | GPIO 13 | DIO1 |
| BUSY | GPIO 27 | BUSY |
| VCC | 3.3V | VCC |
| GND | GND | GND |

> O SX1262 não tem DIO0 (isso é conceito do SX127x) — o pino de interrupção principal é o **DIO1**, e o **BUSY** é obrigatório: a lib precisa verificar esse pino antes/depois de cada transação SPI.
>
> DIO2 do módulo normalmente fica sem uso (controle interno de RF switch) — não conectar a nenhum GPIO.

## Total de pinos usados

**12 GPIOs**: 18, 19, 25, 21, 22, 15, 14, 13, 27.

## Notas de troubleshooting

- Os três periféricos (CC1101 e LoRa, e futuramente a tela) compartilham o mesmo barramento SPI — se o CS de um deles não estiver bem conectado (flutuando), o chip pode não soltar o barramento e derrubar a detecção dos outros dispositivos junto.
- Confirme GND comum entre ESP32, CC1101 e LoRa.
- GPIO 13, 14 e 15 são pinos de *strapping/JTAG* no ESP32 — evite deixá-los flutuando no boot (aqui já ficam conectados ao LoRa, o que é seguro).
- **GPIO 23 desta placa está danificado**: em saída, não consegue baixar a linha (fica em ~3.1V mesmo forçado em 0V). Por isso o MOSI foi movido para o GPIO 25. Não usar o GPIO 23.
