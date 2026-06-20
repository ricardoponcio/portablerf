# Manual do Usuário - PortableRF

Bem-vindo ao manual de operação do PortableRF. Esta ferramenta possui um menu navegável com 6 funções principais. Navegue usando as setas `UP` e `DOWN`, confirme com `OK` e retorne ao menu com `BACK`.

## 1. Sniffer RF (Decodificador de Código Fixo)
Esta tela varre o ambiente em busca de sinais de RF que utilizam protocolos abertos e conhecidos (como chips PT2262, EV1527, etc).

*   **Como usar:** Ao entrar na tela, ele começará a escanear a frequência atual (padrão 433MHz).
*   **Navegação:** Pressione `UP` ou `DOWN` para mudar a banda de frequência (315, 433, 868, 915).
*   **Captura:** Ao pressionar um controle remoto compatível, a tela vai parar a varredura, e mostrará no rodapé o código decodificado em HEX (ex: `COD:A1B2C3`).
*   **Replay Rápido:** Enquanto o código estiver na tela, você pode pressionar `OK` para retransmiti-lo imediatamente.
*   **Memória:** A captura bem-sucedida é enviada automaticamente para o histórico global.

## 2. Sniffer RAW (Captura Bruta)
Utilizado para controles que o "Sniffer RF" não entende (como controles de portão modernos com Rolling Code). Ele não tenta decodificar a senha, apenas grava os pulsos altos e baixos de energia no ar.

*   **Como usar:** Selecione a frequência correta do controle com `DOWN`.
*   **Gravar:** Pressione `UP`. A tela ficará vermelha com o aviso `ESCUTANDO...`. Aperte e segure o botão do seu controle remoto próximo ao aparelho.
*   **Sucesso:** Quando detectar o sinal, ele gravará os pulsos usando um filtro de ruído inteligente. Se for bem-sucedido, aparecerá `Capturado: XXX` (onde XXX é o número de pulsos).
*   **Replay:** Com um sinal capturado em memória, pressione `OK` para dar o "play" e retransmitir exatamente o que foi ouvido. Isso realiza um *Replay Attack*.
*   **Atenção:** Se você mudar de frequência enquanto uma captura estiver na tela, a captura RAW será "escondida" (para evitar que você a transmita na frequência errada).

## 3. Radar RF (Analisador de Espectro)
Exibe um gráfico de barras animado mostrando a força do sinal (RSSI) nas redondezas de uma frequência escolhida.
*   **Uso:** Excelente para descobrir em qual frequência um controle desconhecido opera. Aperte o controle e veja em qual banda o "pico" sobe. Também serve para rastrear *jammers* na vizinhança.

## 4. Transmitir (Lista Global)
Um histórico centralizado das suas últimas 5 capturas.
*   Sempre que você captura um sinal válido no *Sniffer RF* ou no *Sniffer RAW*, ele vai para o topo dessa lista.
*   A lista exibe a frequência, o tipo do sinal (`HEX` para decodificado ou `RAW(pulsos)` para brutos).
*   **Como usar:** Navegue para cima e para baixo e pressione `OK` no item desejado. O PortableRF se encarregará de configurar o rádio e retransmitir o sinal da maneira correta, seja ele puro ou decodificado.

## 5. Jammer RF
Modo de transmissão contínua que "suja" uma frequência específica com ruído constante de alta potência, impedindo que outros aparelhos comuniquem nessa banda.
*   **Como usar:** Escolha a frequência e aperte `OK` para ligar. Aperte `OK` novamente para desligar.
*   *Uso estrito para testes de robustez de sistemas próprios.*
