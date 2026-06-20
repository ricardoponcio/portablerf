# PortableRF

PortableRF é uma ferramenta de bolso para análise e interação com sinais de rádio frequência (RF) Sub-GHz, construída com um ESP8266, um módulo transceptor CC1101 e um display TFT colorido ST7735.

Com um design inspirado em ferramentas hackers de RF (como o Flipper Zero), o PortableRF permite escanear frequências comuns (315MHz, 433MHz, 868MHz, 915MHz), analisar espectro de ruído, capturar sinais OOK/ASK (tanto protocolos conhecidos quanto capturas RAW para *rolling codes*) e retransmitir esses sinais.

## 🚀 Funcionalidades Principais

*   **Sniffer RF (Decodificador):** Usa a biblioteca `rc-switch` para capturar e decodificar sinais de controles remotos de código fixo (Fixed Code). Ideal para alarmes de casa, tomadas sem fio e ventiladores.
*   **Sniffer RAW (Gravador Bruto):** Escuta o ar usando interrupções de hardware para gravar a "fita" exata de microssegundos do sinal, contornando a necessidade de decodificação. Útil para capturar e fazer *replay attacks* em controles modernos (*Rolling Codes* simples).
*   **Radar RF (Analisador de Espectro):** Varre uma faixa de frequências ao redor da frequência base selecionada e desenha um gráfico de barras em tempo real na tela, permitindo visualizar ruídos e encontrar a frequência exata de um controle remoto.
*   **Transmitir (Histórico Global):** Mantém as últimas 5 capturas (sejam elas RAW ou Decodificadas) na memória, permitindo selecionar qualquer uma delas e retransmitir.
*   **Jammer RF:** Transmite sinal contínuo na frequência selecionada para inundar o canal de RF (apenas para fins educacionais e testes de blindagem).

## 🛠️ Hardware Necessário

*   Microcontrolador: **ESP8266 (NodeMCU)**
*   Módulo RF: **CC1101 (Sub-GHz)**
*   Display: **TFT LCD 1.44" ou 1.8" (Controlador ST7735)**
*   **4 Botões Push-Button** (UP, DOWN, OK, BACK)

*(Para a pinagem exata do projeto, consulte a documentação física em `docs/RADIO_TESTE_SEMI_MOVEL.md`).*

## 📚 Documentação Completa

Para detalhes aprofundados sobre como usar a ferramenta e como o código funciona internamente, consulte a pasta `docs/`:

1.  [Manual do Usuário](docs/USER_MANUAL.md) - Guia passo-a-passo de como operar os menus e capturar controles.
2.  [Arquitetura do Software](docs/ARCHITECTURE.md) - Detalhes sobre a máquina de estados, interrupções e a lista global de sinais.
3.  [Documentação de Hardware](docs/RADIO_TESTE_SEMI_MOVEL.md) - Pinagem e diagrama de conexões.

## 💻 Compilação e Upload

Este projeto foi desenvolvido utilizando o **PlatformIO**.

1.  Abra a pasta do projeto no VSCode com a extensão PlatformIO instalada.
2.  O arquivo `platformio.ini` cuidará de baixar todas as dependências (Adafruit GFX, ST7735, SmartRC-CC1101, rc-switch).
3.  Conecte o seu NodeMCU via USB.
4.  Clique em **Build** e depois em **Upload**.

---
*Aviso: Este projeto tem fins puramente educacionais de pesquisa em segurança de radiofrequência. A retransmissão de sinais (replay attacks) ou uso de jammer em equipamentos que não lhe pertencem pode violar leis locais.*
