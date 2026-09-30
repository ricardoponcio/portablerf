#pragma once
#include <WebServer.h>
#include "radio.h"
#include "auto_ping.h"
#include "event_log.h"

// Um rádio e o auto-ping dele, como a página enxerga.
struct RadioChannel {
    Radio& radio;
    AutoPing& autoPing;
};

// Servidor HTTP: página em "/" e API JSON usada por ela.
//   GET  /api/status             Wi-Fi e estado de cada rádio
//   POST /api/send?radio=&msg=   envia uma mensagem
//   POST /api/auto?radio=&ms=    liga (ms > 0) ou desliga (ms = 0) o auto-ping
//   GET  /api/log?since=<id>     eventos com id > since
class WebUi {
public:
    WebUi(EventLog& log, RadioChannel cc1101, RadioChannel lora)
        : log_(log), channels_{cc1101, lora} {}

    void begin();
    void handle();  // chamar no loop()

private:
    void handleRoot();
    void handleStatus();
    void handleSend();
    void handleAuto();
    void handleLog();

    // Resolve o parâmetro "radio"; responde 400 e devolve nullptr se inválido.
    RadioChannel* channelArg();
    static String jsonEscape(const char* s);

    WebServer server_{80};
    EventLog& log_;
    RadioChannel channels_[2];
};
