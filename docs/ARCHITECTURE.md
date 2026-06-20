# Arquitetura de Software - PortableRF

A arquitetura do PortableRF foi desenvolvida pensando em extensibilidade e gerenciamento eficiente de memória no ESP8266, superando as limitações de interrupções concorrentes do rádio CC1101.

## 1. Máquina de Estados (State Machine)
O sistema opera baseado em uma máquina de estados finitos (`SystemState` em `globals.h`), gerenciada no `loop()` principal (`main.cpp`). 
*   Isso previne a execução de telas concorrentes e permite rotinas de `setup()` limpas quando o usuário transita entre telas.

## 2. A Estrutura Unificada `SavedSignal`
Para lidar com a baixa memória e a necessidade de retransmitir diversos protocolos, criamos a estrutura `SavedSignal`.
```cpp
struct SavedSignal {
    SignalType type;          // SIG_NONE, SIG_DECODED, SIG_RAW
    float freq;               // Frequencia em MHz
    long decodedValue;        // RC-Switch decodificado
    int bitlength;            // RC-Switch tamanho
    uint16_t rawDurations[MAX_RAW_BUFFER]; // Gravação de pulsos
    int rawCount;             // Qtd de pulsos gravados
};
```
Esta estrutura única alimenta o array global `signalHistory[5]`. 

### Vantagens dessa arquitetura:
A função `transmitSignal(SavedSignal &sig)` (em `globals.cpp`) age como um *Facade*. A tela de "Transmitir" não precisa saber como o sinal foi gravado. Ela apenas envia a estrutura para a função, que faz um switch baseando-se no `type` e configura o registrador CCMode do CC1101 de acordo (Mode 1 para Decoded via rc-switch, Mode 0 para bit-banging do RAW).

## 3. O Desafio do Sniffer RAW (CCMode e Interrupts)
Originalmente, a captura RAW falhava porque o CC1101 operava em `CCMode(1)` (Modo de pacote síncrono do rc-switch). Nesse modo, o pino GDO0 não oscila para refletir a portadora de RF do ar.
*   **Solução:** Ao entrar em `STATE_RAW_RF`, o sistema força `ELECHOUSE_cc1101.setCCMode(0)`, tornando a saída assíncrona (bit-banging).
*   Para não travar o loop principal com enquetes (`digitalRead`), a captura RAW utiliza `attachInterrupt()` acoplado a uma rotina de tratamento de interrupção executada diretamente na RAM (`ICACHE_RAM_ATTR rawInterruptHandler`).
*   **Filtro Antirruído:** Em OOK, sem sinal válido, o controle automático de ganho (AGC) exibe ruído branco, acionando a interrupção milhares de vezes por segundo. O filtro embutido na `rawInterruptHandler` descarta pulsos `< 60us`, impedindo o estouro do limite do buffer (300) por lixo eletromagnético antes do sinal real ser emitido pelo controle.
