# Planejamento do Projeto: ESP32 Multi-Rádio e Áudio (Fase 1)

Este documento centraliza as definições de hardware e o plano de ação para a sua nova placa ESP32 (30 pinos), que será o cérebro da nossa estação portátil de RF e Walkie-Talkie.

## 🎯 Objetivo Inicial
Criar um dispositivo multifuncional capaz de operar como Analisador de Espectro Sub-1GHz (via CC1101), fazer comunicação de longo alcance (via LoRa) e oferecer uma funcionalidade de rádio-comunicador (Walkie-Talkie) usando áudio compactado. Tudo isso operado em uma Tela LCD maior.

---

## 🛠️ Lista de Hardware (BOM)

- **Microcontrolador:** ESP32 NodeMCU (Versão de 30 Pinos)
- **Alimentação Principal:** Módulo de Recarregamento USB-C com 2x Baterias 18650
- **Rádio Curto Alcance:** Módulo CC1101 (433MHz)
- **Rádio Longo Alcance:** Módulo LoRa (ex: SX1276 ou SX1262)
- **Interface Visual:** Tela LCD SPI (Tamanho Maior)
- **Entrada de Áudio:** Microfone de Eletreto (Módulo com pré-amplificador MAX4466)
- **Saída de Áudio:** Caixinha de Som (Mono) + Amplificador PAM8403

> [!WARNING]
> **Sobre o Amplificador de Áudio (PAM8403)**
> - **Por que usar?** O pino de áudio do ESP32 entrega correntes minúsculas (~12mA). Uma caixinha exige muita corrente (~300mA+). O PAM8403 é o "músculo" necessário para fazer o alto-falante vibrar sem fritar o ESP32.
> - **Alimentação Segura:** NUNCA alimente o PAM8403 pelo pino `3.3V` do ESP32 (causaria *Brownout*/Reinicialização). A alimentação (5V/GND) dele deve vir **diretamente da saída do seu módulo das baterias 18650** (ou em último caso do pino `VIN` do ESP).
> - **Ligação Mono (Uma Caixa):** O módulo é Estéreo (Left e Right). Você pode usar apenas um lado (ex: L) para ligar a sua única caixinha e deixar o lado (R) completamente vazio. **JAMAIS junte o positivo do lado L com o positivo do lado R** para tentar "somar a potência", pois isso causará curto-circuito interno e destruirá o chip.

---

## 🔌 Tabela de Mapeamento de Pinos (Pinout)

Nesta configuração, otimizamos o uso do microcontrolador compartilhando o barramento SPI entre os três principais periféricos (Tela, CC1101 e LoRa).

| Módulo | Função | Sugestão de Pino no ESP32 | Notas |
| :--- | :--- | :--- | :--- |
| **Barramento SPI** | SCK (Clock) | `GPIO 18` | Compartilhado entre Tela, LoRa e CC1101 |
| | MOSI (Data Out) | `GPIO 23` | Compartilhado entre Tela, LoRa e CC1101 |
| | MISO (Data In) | `GPIO 19` | Compartilhado entre Tela, LoRa e CC1101 |
| **Tela LCD** | CS (Chip Select) | `GPIO 5` | Exclusivo da Tela |
| | DC (Data/Command)| `GPIO 2` | Exclusivo da Tela |
| | RST (Reset) | `GPIO 4` | Exclusivo da Tela |
| **Rádio CC1101** | CS (Chip Select) | `GPIO 21` | Exclusivo do CC1101 |
| | GDO0 (Interrupção)| `GPIO 22` | Necessário para leitura de códigos e pacotes |
| **Rádio LoRa** | CS (Chip Select) | `GPIO 15` | Exclusivo do LoRa |
| | DIO0 (Interrupção)| `GPIO 13` | Necessário para aviso de pacotes recebidos |
| | RST (Reset) | `GPIO 14` | Exclusivo do LoRa |
| **Microfone** | OUT (Analógico) | `GPIO 34` ou `35`| Pino exclusivo para leitura Analógica (ADC) |
| **Alto-falante** | IN (Para PAM8403)| `GPIO 25` | Pino especial (DAC 1) que converte número em som |

> [!TIP]
> **Total de Pinos Utilizados:** **13 Pinos**. 
> Você ainda tem cerca de **8 pinos livres** para instalar componentes no futuro (Cartão SD, GPS, Encoders, Sensores ambientais, etc).

---

## 🚀 Próximos Passos e Desafios

### 1. Codificação de Voz via LoRa
A banda de frequência do LoRa é projetada para enviar pequenos textos (telemetria), não voz. Para fazer o Walkie-Talkie funcionar via LoRa, o ESP32 precisará gravar a sua voz pelo microfone, usar um software chamado **Codec2** para "espremer" e destruir a qualidade da voz até virar um dado bem pequeno, enviá-lo, e do outro lado o ESP32 descompacta esse pacote para tocar no PAM8403. O som ficará similar a comunicações de pilotos de caça/astronautas.

### 2. O Firmware Multi-Tarefa
Ao contrário do ESP8266, o ESP32 possui o **FreeRTOS** nativo e dois núcleos. Para que o analisador de rádio não perca pacotes enquanto o ESP32 está processando áudio na tela, nós vamos designar **1 Núcleo inteiramente para o rádio e áudio**, e o **outro Núcleo exclusivamente para atualizar a tela e os gráficos**.

---
*Documento vivo - Poderá ser atualizado conforme a evolução das ideias de hardware.*
