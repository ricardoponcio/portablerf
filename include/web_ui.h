#pragma once
#include <WebServer.h>
#include "radio.h"
#include "lora_radio.h"
#include "auto_ping.h"
#include "event_log.h"
#include "sniffer.h"
#include "meshtastic.h"

// Um rádio, o auto-ping e o sniff dele, como a página enxerga.
struct RadioChannel {
    Radio& radio;
    AutoPing& autoPing;
    Sniffer& sniffer;
};

// Servidor HTTP: página em "/" e API JSON usada por ela.
//   GET  /api/status                    Wi-Fi e estado de cada rádio (+ config do LoRa)
//   POST /api/send?radio=&msg=          envia uma mensagem
//   POST /api/auto?radio=&ms=           liga (ms > 0) ou desliga (ms = 0) o auto-ping
//   POST /api/freq?radio=&mhz=          troca a frequência
//   POST /api/lora?freq=&bw=&sf=&cr=&sync=&pre=   troca a config do LoRa (o que faltar fica igual)
//   POST /api/mesh?n0=&k0=&n1=&k1=...   canais Meshtastic para decifrar (nome + PSK base64)
//   GET  /api/log?since=<id>            eventos com id > since (Meshtastic vem decifrado em "plain")
//   GET  /api/sniff?radio=&since=<id>   picos de RSSI com id > since
class WebUi {
public:
    WebUi(EventLog& log, RadioChannel cc1101, RadioChannel lora, LoRaRadio& loraRadio, MeshDecoder& mesh)
        : log_(log), channels_{cc1101, lora}, lora_(loraRadio), mesh_(mesh) {}

    void begin();
    void handle();  // chamar no loop()

private:
    void handleRoot();
    void handleStatus();
    void handleSend();
    void handleAuto();
    void handleFreq();
    void handleLora();
    void handleMesh();
    void handleLog();
    void handleSniff();

    // Resolve o parâmetro "radio"; responde 400 e devolve nullptr se inválido.
    RadioChannel* channelArg();
    String loraConfigJson() const;
    String meshChannelsJson() const;
    static String jsonEscape(const char* s);

    WebServer server_{80};
    EventLog& log_;
    RadioChannel channels_[2];
    LoRaRadio& lora_;
    MeshDecoder& mesh_;
};
