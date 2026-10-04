#include "web_page.h"

const char WEB_PAGE_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Rádios</title>
    <style>
        :root {
            --bg-color: #0f172a;
            --card-bg: #1e293b;
            --inner-bg: #0f172a;
            --border: #334155;
            --primary: #38bdf8;
            --text: #f8fafc;
            --text-secondary: #94a3b8;
            --success: #22c55e;
            --fail: #ef4444;
            --tx: #f59e0b;
        }
        * { box-sizing: border-box; margin: 0; padding: 0; font-family: 'Segoe UI', sans-serif; }
        [hidden] { display: none !important; }
        body { background-color: var(--bg-color); color: var(--text); padding: 16px; }
        .wrap { max-width: 1100px; margin: 0 auto; display: flex; flex-direction: column; gap: 12px; }
        header h1 { color: var(--primary); font-size: 1.4rem; }
        .sub { color: var(--text-secondary); font-size: 0.85rem; margin-top: 2px; }
        .card { background-color: var(--card-bg); border: 1px solid var(--border); border-radius: 12px; padding: 14px 16px; }
        .badge { padding: 3px 10px; border-radius: 10px; font-size: 0.75rem; font-weight: 600; background: rgba(148,163,184,.2); color: var(--text-secondary); white-space: nowrap; }
        .ok { background-color: rgba(34, 197, 94, 0.2); color: var(--success); }
        .bad { background-color: rgba(239, 68, 68, 0.2); color: var(--fail); }
        .step { flex: none; width: 22px; height: 22px; border-radius: 50%; background: rgba(56,189,248,.15); color: var(--primary);
                font-size: .75rem; font-weight: 700; display: inline-flex; align-items: center; justify-content: center; }
        .head { display: flex; align-items: center; gap: 10px; flex-wrap: wrap; }
        .head b { font-size: 1rem; }
        .sum { color: var(--text-secondary); font-size: .85rem; margin-left: auto; display: flex; gap: 14px; flex-wrap: wrap; }
        .sum b { color: var(--text); font-weight: 600; }

        /* Painéis de configuração (accordion) */
        details.acc > summary { list-style: none; cursor: pointer; user-select: none; }
        details.acc > summary::-webkit-details-marker { display: none; }
        details.acc > summary .head::before { content: '▸'; color: var(--text-secondary); transition: transform .15s; display: inline-block; }
        details.acc[open] > summary .head::before { transform: rotate(90deg); }
        details.acc[open] > summary { padding-bottom: 12px; margin-bottom: 14px; border-bottom: 1px solid var(--border); }

        .row { display: flex; gap: 8px; margin-bottom: 10px; align-items: center; flex-wrap: wrap; }
        input[type=text], input[type=number] { flex: 1; min-width: 0; background: var(--inner-bg); border: 1px solid var(--border); color: var(--text); border-radius: 8px; padding: 8px 10px; font-size: 0.95rem; }
        input:disabled { opacity: .45; }
        button { background: var(--primary); color: #0f172a; border: 0; border-radius: 8px; padding: 8px 14px; font-weight: 600; cursor: pointer; }
        button:disabled { opacity: .4; cursor: default; }
        button.small { font-size: .75rem; padding: 4px 10px; }
        button.ghost { background: transparent; color: var(--text-secondary); border: 1px solid var(--border); }
        select { background: var(--inner-bg); color: var(--text); border: 1px solid var(--border); border-radius: 8px; padding: 6px; }
        .auto { display: flex; align-items: center; gap: 8px; font-size: 0.85rem; color: var(--text-secondary); }
        .section { font-size: .8rem; font-weight: 600; color: var(--text-secondary); text-transform: uppercase; letter-spacing: .04em; margin: 16px 0 8px; }
        .cfg { display: grid; grid-template-columns: repeat(auto-fit, minmax(120px, 1fr)); gap: 8px; margin-bottom: 10px; }
        .cfg label { display: flex; flex-direction: column; gap: 4px; font-size: 0.75rem; color: var(--text-secondary); }
        .cfg label.wide { grid-column: span 2; }
        .cfg input, .cfg select { width: 100%; padding: 6px 8px; font-size: 0.9rem; }
        .mesh-ch { display: grid; grid-template-columns: 1fr 1.4fr 70px; gap: 8px; margin-bottom: 6px; align-items: center; }
        .mesh-ch input { padding: 6px 8px; font-size: .9rem; }
        .mesh-ch span { font-size: .75rem; color: var(--text-secondary); font-family: ui-monospace, monospace; }
        .hint { font-size: 0.78rem; color: var(--text-secondary); margin: 4px 0 10px; line-height: 1.4; }
        .note { color: var(--tx); }

        /* Gráficos de sniff */
        .chart-card canvas { width: 100%; height: 170px; display: block; background: var(--inner-bg); border-radius: 8px; margin-top: 10px; }
        .legend { display: flex; flex-wrap: wrap; gap: 14px; margin-top: 8px; font-size: 0.75rem; color: var(--text-secondary); }
        .legend i { display: inline-block; width: 10px; height: 10px; border-radius: 2px; margin-right: 5px; vertical-align: -1px; }

        /* Log */
        .tabs { display: flex; gap: 4px; }
        .tabs button { background: transparent; color: var(--text-secondary); border: 1px solid transparent; }
        .tabs button.active { background: var(--inner-bg); color: var(--text); border-color: var(--border); }
        .tools { margin-left: auto; display: flex; gap: 10px; align-items: center; flex-wrap: wrap; }
        .tbl { width: 100%; border-collapse: collapse; font-size: 0.85rem; }
        .tbl th { text-align: left; color: var(--text-secondary); font-weight: 500; padding: 6px; border-bottom: 1px solid var(--border); position: sticky; top: 0; background: var(--card-bg); }
        .tbl td { padding: 6px; border-bottom: 1px solid #1f2a3d; font-variant-numeric: tabular-nums; vertical-align: top; }
        .tbl td.msg { word-break: break-word; }
        .tbl td.msg .proto { color: var(--primary); display: block; font-size: .8rem; }
        .tbl td.msg .app { display: block; }
        .tbl td.msg .muted { color: var(--text-secondary); display: block; font-size: .8rem; }
        .tbl td.msg .mono, .mono { font-family: ui-monospace, monospace; }
        .tbl td.msg .hex { color: var(--text-secondary); font-size: 0.78rem; font-family: ui-monospace, monospace; word-break: break-all; display: block; }
        .tbl tr.dup td { opacity: .55; }
        .dir-tx { color: var(--tx); font-weight: 600; }
        .dir-rx { color: var(--success); font-weight: 600; }
        .scroll { max-height: 460px; overflow-y: auto; margin-top: 10px; }
        .empty { color: var(--text-secondary); text-align: center; padding: 20px; }
        a { color: var(--primary); }

        @media (max-width: 640px) {
            .cfg label.wide { grid-column: 1 / -1; }
            .mesh-ch { grid-template-columns: 1fr 1fr; }
            .mesh-ch span { grid-column: 1 / -1; }
            .sum { margin-left: 0; width: 100%; }
        }
    </style>
</head>
<body>
<div class="wrap">
    <header>
        <h1>ESP32 Rádios</h1>
        <p class="sub" id="wifi">Carregando...</p>
    </header>

    <!-- 1 · Configuração do CC1101 -->
    <details class="card acc" id="acc-cc1101" open>
        <summary>
            <div class="head">
                <span class="step">1</span><b>CC1101</b>
                <span class="badge" id="cc1101-badge">...</span>
                <span class="sum">
                    <span><b id="cc1101-sum-freq">--</b></span>
                    <span>TX <b id="cc1101-tx">0</b></span>
                    <span>RX <b id="cc1101-rx">0</b></span>
                    <span>Último RSSI <b id="cc1101-rssi">--</b></span>
                </span>
            </div>
        </summary>
        <div class="cfg">
            <label>Frequência
                <select id="cc1101-freq" onchange="setFreq('cc1101')">
                    <option value="315.00">315 MHz</option>
                    <option value="433.92">433,92 MHz</option>
                    <option value="868.00">868 MHz</option>
                    <option value="915.00">915 MHz</option>
                </select>
            </label>
        </div>
        <div class="row">
            <input type="text" id="cc1101-msg" maxlength="60" placeholder="Mensagem (até 60 caracteres)">
            <button onclick="send('cc1101')">Enviar</button>
        </div>
        <label class="auto">
            <input type="checkbox" id="cc1101-auto" onchange="setAuto('cc1101')"> Auto-ping a cada
            <select id="cc1101-int" onchange="setAuto('cc1101')">
                <option value="1000">1 s</option><option value="2000" selected>2 s</option><option value="5000">5 s</option>
            </select>
        </label>
    </details>

    <!-- 2 · Sniff do CC1101 -->
    <div class="card chart-card">
        <div class="head">
            <span class="step">2</span><b>Sniff CC1101</b>
            <span class="badge" id="sniff-cc1101-freq">--</span>
            <span class="sum">
                <span>Agora <b id="sniff-cc1101-now">--</b></span>
                <span>Pico 10 s <b id="sniff-cc1101-max">--</b></span>
                <span>Piso <b id="sniff-cc1101-floor">--</b></span>
                <button class="small ghost" id="sniff-cc1101-pause" onclick="toggleSniff('cc1101')">Pausar</button>
            </span>
        </div>
        <canvas id="sniff-cc1101-chart"></canvas>
        <div class="legend">
            <span><i style="background:var(--primary)"></i>Pico de RSSI a cada 20 ms</span>
            <span><i style="background:var(--success)"></i>Pacote OK</span>
            <span><i style="background:var(--fail)"></i>Sync casou, CRC ruim</span>
            <span><i style="background:var(--tx)"></i>TX</span>
        </div>
    </div>

    <!-- 3 · Configuração do LoRa -->
    <details class="card acc" id="acc-lora" open>
        <summary>
            <div class="head">
                <span class="step">3</span><b>LoRa SX1262</b>
                <span class="badge" id="lora-badge">...</span>
                <span class="sum">
                    <span><b id="lora-sum-mode">--</b></span>
                    <span>TX <b id="lora-tx">0</b></span>
                    <span>RX <b id="lora-rx">0</b></span>
                    <span>Último RSSI <b id="lora-rssi">--</b></span>
                    <span>SNR <b id="lora-snr">--</b></span>
                </span>
            </div>
        </summary>
        <div class="cfg">
            <label class="wide">Modo
                <select id="lora-preset" onchange="loraPreset()">
                    <option value="p2p">Placa-placa (privado)</option>
                    <option value="lorawan">LoRaWAN AU915 (escuta)</option>
                    <option value="mesh-anz">Meshtastic LongFast · ANZ (escuta)</option>
                    <option value="mesh-us">Meshtastic LongFast · US (escuta)</option>
                    <option value="custom">Personalizado</option>
                </select>
            </label>
            <label class="wide" id="lora-ch-wrap" hidden>Canal AU915 (uplink 125 kHz)
                <select id="lora-ch" onchange="loraChannel()"></select>
            </label>
            <label>Freq. (MHz) <input type="number" id="lora-f" step="0.001" oninput="loraCustom()"></label>
            <label>SF
                <select id="lora-sf" onchange="loraCustom()">
                    <option>5</option><option>6</option><option>7</option><option>8</option>
                    <option>9</option><option>10</option><option>11</option><option>12</option>
                </select>
            </label>
            <label>BW (kHz)
                <select id="lora-bw" onchange="loraCustom()">
                    <option value="62.5">62,5</option><option value="125.0">125</option>
                    <option value="250.0">250</option><option value="500.0">500</option>
                </select>
            </label>
            <label>CR
                <select id="lora-cr" onchange="loraCustom()">
                    <option value="5">4/5</option><option value="6">4/6</option>
                    <option value="7">4/7</option><option value="8">4/8</option>
                </select>
            </label>
            <label>Sync word (hex) <input type="text" id="lora-sync" maxlength="2" oninput="loraCustom()"></label>
            <label>Preâmbulo <input type="number" id="lora-pre" min="1" oninput="loraCustom()"></label>
        </div>
        <div class="row">
            <button onclick="applyLora()">Aplicar</button>
            <span class="auto" id="lora-cfg-msg"></span>
        </div>
        <p class="hint" id="lora-hint"></p>

        <div class="section">Canais Meshtastic (para decifrar)</div>
        <p class="hint">Nome e PSK em base64, como no app. O padrão público é <span class="mono">LongFast</span> /
            <span class="mono">AQ==</span>. Pacotes de canais privados só abrem com a chave deles; mensagens diretas
            (criptografia por nó) não dá para ler.</p>
        <div id="mesh-channels"></div>
        <div class="row">
            <button onclick="saveMesh()">Salvar canais</button>
            <span class="auto" id="mesh-msg"></span>
        </div>

        <div class="section">Transmitir</div>
        <p class="hint note" id="lora-listen-note" hidden>Modo escuta: enviar aqui jogaria pacotes inválidos na rede, então o envio fica desligado.</p>
        <div class="row">
            <input type="text" id="lora-msg" maxlength="60" placeholder="Mensagem (até 60 caracteres)">
            <button id="lora-send" onclick="send('lora')">Enviar</button>
        </div>
        <label class="auto">
            <input type="checkbox" id="lora-auto" onchange="setAuto('lora')"> Auto-ping a cada
            <select id="lora-int" onchange="setAuto('lora')">
                <option value="1000">1 s</option><option value="2000" selected>2 s</option><option value="5000">5 s</option>
            </select>
        </label>
    </details>

    <!-- 4 · Sniff do LoRa -->
    <div class="card chart-card">
        <div class="head">
            <span class="step">4</span><b>Sniff LoRa</b>
            <span class="badge" id="sniff-lora-freq">--</span>
            <span class="sum">
                <span>Agora <b id="sniff-lora-now">--</b></span>
                <span>Pico 10 s <b id="sniff-lora-max">--</b></span>
                <span>Piso <b id="sniff-lora-floor">--</b></span>
                <button class="small ghost" id="sniff-lora-pause" onclick="toggleSniff('lora')">Pausar</button>
            </span>
        </div>
        <canvas id="sniff-lora-chart"></canvas>
        <div class="legend">
            <span><i style="background:var(--primary)"></i>Pico de RSSI a cada 20 ms</span>
            <span><i style="background:var(--success)"></i>Pacote OK</span>
            <span><i style="background:var(--fail)"></i>CRC/cabeçalho ruim</span>
            <span><i style="background:var(--tx)"></i>TX</span>
            <span>LoRa decodifica até ~20 dB abaixo do piso: pacote OK sem pico visível é normal.</span>
        </div>
    </div>

    <!-- 5 · Log -->
    <div class="card">
        <div class="head">
            <span class="step">5</span>
            <div class="tabs">
                <button class="active" id="tab-log" onclick="showTab('log')">Pacotes</button>
                <button id="tab-nodes" onclick="showTab('nodes')">Nós Meshtastic (<span id="nodes-count">0</span>)</button>
            </div>
            <div class="tools">
                <select id="log-filter" onchange="applyFilter()">
                    <option value="">Todos os rádios</option><option value="cc1101">Só CC1101</option><option value="lora">Só LoRa</option>
                </select>
                <label class="auto"><input type="checkbox" id="hide-dup" onchange="applyFilter()"> Ocultar repetidos</label>
                <label class="auto"><input type="checkbox" id="hex-mode" onchange="rerenderMsgs()"> Hex</label>
                <button class="small ghost" onclick="clearLog()">Limpar</button>
            </div>
        </div>
        <div class="scroll" id="pane-log">
            <table class="tbl">
                <thead><tr><th>Tempo</th><th>Rádio</th><th></th><th>Mensagem</th><th>RSSI</th><th>SNR</th><th>Status</th></tr></thead>
                <tbody id="log"><tr><td colspan="7" class="empty">Nenhum pacote ainda</td></tr></tbody>
            </table>
        </div>
        <div class="scroll" id="pane-nodes" hidden>
            <table class="tbl">
                <thead><tr><th>Nó</th><th>Nome</th><th>Modelo / papel</th><th>Visto</th><th>Saltos</th><th>RSSI / SNR</th><th>Bateria</th><th>Posição</th><th>Pacotes</th></tr></thead>
                <tbody id="nodes"><tr><td colspan="9" class="empty">Nenhum nó ouvido ainda (precisa do modo Meshtastic no LoRa)</td></tr></tbody>
            </table>
        </div>
    </div>
</div>
<script>
    const $ = id => document.getElementById(id);
    // localStorage pode estar bloqueado (aba anônima etc.): tudo aqui é só conveniência
    const store = {
        get(k, d) { try { const v = localStorage.getItem(k); return v === null ? d : JSON.parse(v); } catch (e) { return d; } },
        set(k, v) { try { localStorage.setItem(k, JSON.stringify(v)); } catch (e) {} },
    };

    let lastId = 0;
    const counts = { cc1101: { tx: 0, rx: 0 }, lora: { tx: 0, rx: 0 } };

    // Accordions lembram se estavam abertos
    for (const id of ['acc-cc1101', 'acc-lora']) {
        const d = $(id);
        d.open = store.get(id, true);
        d.addEventListener('toggle', () => store.set(id, d.open));
    }

    function badge(id, text, cls) {
        const el = $(id);
        el.textContent = text;
        el.className = 'badge ' + cls;
    }

    function refreshStatus() {
        fetch('/api/status').then(r => r.json()).then(d => {
            $('wifi').textContent = d.ssid + ' · ' + d.ip + ' · ' + d.wifi_rssi + ' dBm';
            for (const [name, r] of Object.entries(d.radios)) {
                if (r.pending) badge(name + '-badge', 'INICIALIZANDO', '');
                else if (r.ok) badge(name + '-badge', 'OK', 'ok');
                else badge(name + '-badge', 'FALHA (' + r.state + ')', 'bad');
                // Mantém o auto-ping em sincronia com o firmware (ex.: após reiniciar a placa)
                const cb = $(name + '-auto');
                if (document.activeElement !== cb) cb.checked = r.auto_ms > 0;
            }
            const cc = d.radios.cc1101;
            if (document.activeElement !== $('cc1101-freq')) $('cc1101-freq').value = cc.freq.toFixed(2);
            $('cc1101-sum-freq').textContent = $('sniff-cc1101-freq').textContent = cc.freq.toFixed(2) + ' MHz';
            const cfg = d.radios.lora.cfg;
            if (cfg) {
                loraCfg = cfg;
                $('sniff-lora-freq').textContent = loraSummary(cfg);
                $('lora-sum-mode').textContent = presetLabel(matchPreset(cfg)) + ' · ' + cfg.freq.toFixed(3) + ' MHz SF' + cfg.sf;
                updateListenMode(cfg);
                if (!loraDirty) showLoraCfg(cfg);
            }
            if (!meshDirty) showMeshChannels(d.mesh);
        }).catch(() => {});
    }

    function send(radio) {
        const input = $(radio + '-msg');
        const msg = input.value.trim();
        if (!msg) return;
        fetch('/api/send?radio=' + radio + '&msg=' + encodeURIComponent(msg), { method: 'POST' })
            .then(r => r.json()).then(d => { if (d.ok) input.value = ''; refreshLog(); });
    }

    function setAuto(radio) {
        const on = $(radio + '-auto').checked;
        const ms = on ? $(radio + '-int').value : 0;
        fetch('/api/auto?radio=' + radio + '&ms=' + ms, { method: 'POST' });
    }

    function setFreq(radio) {
        const sel = $(radio + '-freq');
        fetch('/api/freq?radio=' + radio + '&mhz=' + sel.value, { method: 'POST' })
            .then(r => r.json()).then(d => {
                sel.value = d.freq.toFixed(2);
                if (!d.ok) alert('Falha ao trocar frequência (' + d.state + ')');
                sel.blur();
            });
    }

    // ---------- Configuração do LoRa ----------
    // Uplinks LoRaWAN AU915 (plano usado no Brasil): 64 canais de 125 kHz a partir de 915,2 MHz.
    // Um gateway de verdade escuta 8 canais e todos os SF ao mesmo tempo; o SX1262 escuta 1 canal e 1 SF.
    const AU915_CH = n => +(915.2 + 0.2 * n).toFixed(1);
    const LORA_PRESETS = {
        p2p:        { label: 'Placa-placa', freq: 915.0, bw: 125, sf: 9, cr: 7, sync: 0x12, pre: 8,
                      hint: 'Config padrão para conversar com outra placa igual a esta.' },
        lorawan:    { label: 'LoRaWAN', freq: AU915_CH(8), bw: 125, sf: 7, cr: 5, sync: 0x34, pre: 8,
                      hint: 'Escuta um canal e um SF por vez. Dispositivos perto usam SF7–SF9, longe SF10–SF12: troque o SF e o canal ' +
                            '(a TTN usa a sub-banda 2, canais 8–15). O payload é criptografado; aparece só o cabeçalho.' },
        'mesh-anz': { label: 'Meshtastic ANZ', freq: 919.875, bw: 250, sf: 11, cr: 5, sync: 0x2B, pre: 16,
                      hint: 'Canal padrão LongFast da região ANZ (915–928 MHz), comum no Brasil.' },
        'mesh-us':  { label: 'Meshtastic US', freq: 906.875, bw: 250, sf: 11, cr: 5, sync: 0x2B, pre: 16,
                      hint: 'Canal padrão LongFast da região US (902–928 MHz).' },
    };
    const presetLabel = k => LORA_PRESETS[k] ? LORA_PRESETS[k].label : 'Personalizado';
    let loraDirty = false;  // usuário mexendo nos campos: não sobrescreve com o status
    let loraCfg = null;

    (function fillAu915() {
        const sel = $('lora-ch');
        for (let n = 0; n < 64; n++) {
            const o = document.createElement('option');
            o.value = n;
            o.textContent = 'Canal ' + n + ' · ' + AU915_CH(n).toFixed(1) + ' MHz (sub-banda ' + (Math.floor(n / 8) + 1) + ')';
            sel.appendChild(o);
        }
    })();

    function loraSummary(c) {
        return c.freq.toFixed(3) + ' MHz · SF' + c.sf + ' · ' + c.bw + ' kHz · sync 0x' + c.sync.toString(16).toUpperCase();
    }

    function matchPreset(c) {
        for (const [k, p] of Object.entries(LORA_PRESETS)) {
            const sameRadio = p.bw === c.bw && p.cr === c.cr && p.sync === c.sync && p.pre === c.pre;
            // LoRaWAN aceita qualquer canal AU915 e qualquer SF
            if (k === 'lorawan' && sameRadio) return k;
            if (sameRadio && p.sf === c.sf && Math.abs(p.freq - c.freq) < 0.0005) return k;
        }
        return 'custom';
    }

    function showLoraCfg(c) {
        $('lora-f').value = c.freq.toFixed(3);
        $('lora-sf').value = c.sf;
        $('lora-bw').value = c.bw.toFixed(1);
        $('lora-cr').value = c.cr;
        $('lora-sync').value = c.sync.toString(16).toUpperCase().padStart(2, '0');
        $('lora-pre').value = c.pre;
        const preset = matchPreset(c);
        $('lora-preset').value = preset;
        const ch = Math.round((c.freq - 915.2) / 0.2);
        if (preset === 'lorawan' && ch >= 0 && ch < 64) $('lora-ch').value = ch;
        updateLoraHint();
    }

    function updateLoraHint() {
        const preset = $('lora-preset').value;
        $('lora-ch-wrap').hidden = preset !== 'lorawan';
        $('lora-hint').textContent = LORA_PRESETS[preset] ? LORA_PRESETS[preset].hint : '';
    }

    // LoRaWAN e Meshtastic: só escuta (o que esta placa enviaria não segue o protocolo)
    function updateListenMode(c) {
        const listen = c.sync === 0x34 || c.sync === 0x2B;
        $('lora-listen-note').hidden = !listen;
        for (const id of ['lora-msg', 'lora-send', 'lora-auto', 'lora-int']) $(id).disabled = listen;
    }

    function loraPreset() {
        loraDirty = true;
        const p = LORA_PRESETS[$('lora-preset').value];
        if (p) showLoraCfg(p);
        else updateLoraHint();
    }

    function loraChannel() {
        loraDirty = true;
        $('lora-f').value = AU915_CH(+$('lora-ch').value).toFixed(3);
    }

    function loraCustom() {
        loraDirty = true;
        const sel = $('lora-preset');
        // Trocar SF/canal no LoRaWAN continua sendo LoRaWAN; o resto vira "Personalizado"
        if (sel.value !== 'lorawan' || ['lora-sync', 'lora-bw', 'lora-cr', 'lora-pre'].includes(document.activeElement.id)) {
            sel.value = 'custom';
            updateLoraHint();
        }
    }

    function applyLora() {
        const q = new URLSearchParams({
            freq: $('lora-f').value, sf: $('lora-sf').value, bw: $('lora-bw').value,
            cr: $('lora-cr').value, sync: $('lora-sync').value, pre: $('lora-pre').value,
        });
        const msg = $('lora-cfg-msg');
        msg.textContent = 'Aplicando...';
        msg.style.color = '';
        fetch('/api/lora?' + q, { method: 'POST' }).then(r => r.json()).then(d => {
            msg.textContent = d.ok ? 'Aplicado.' : 'Recusado pelo rádio (' + d.state + '), mantida a anterior.';
            msg.style.color = d.ok ? 'var(--success)' : 'var(--fail)';
            loraDirty = false;
            loraCfg = d.cfg;
            showLoraCfg(d.cfg);
            updateListenMode(d.cfg);
            // Entrou em modo escuta com auto-ping ligado: desliga para não poluir a rede
            if ((d.cfg.sync === 0x34 || d.cfg.sync === 0x2B) && $('lora-auto').checked) {
                $('lora-auto').checked = false;
                setAuto('lora');
            }
            setTimeout(() => { msg.textContent = ''; }, 4000);
        }).catch(() => { msg.textContent = 'Sem resposta da placa.'; });
    }

    // ---------- Canais Meshtastic ----------
    let meshDirty = false;

    (function buildMeshRows() {
        const box = $('mesh-channels');
        for (let i = 0; i < 4; i++) {
            const row = document.createElement('div');
            row.className = 'mesh-ch';
            row.innerHTML = '<input type="text" id="mesh-n' + i + '" placeholder="Nome do canal" maxlength="11">' +
                            '<input type="text" id="mesh-k' + i + '" placeholder="PSK (base64)" class="mono">' +
                            '<span id="mesh-h' + i + '"></span>';
            box.appendChild(row);
        }
        box.addEventListener('input', () => { meshDirty = true; });
    })();

    function showMeshChannels(list) {
        for (let i = 0; i < 4; i++) {
            const c = list[i];
            $('mesh-n' + i).value = c ? c.name : '';
            $('mesh-k' + i).value = c ? c.psk : '';
            $('mesh-h' + i).textContent = c ? 'hash 0x' + c.hash.toString(16).padStart(2, '0').toUpperCase() : '';
        }
    }

    function saveMesh() {
        const q = new URLSearchParams();
        for (let i = 0; i < 4; i++) { q.set('n' + i, $('mesh-n' + i).value); q.set('k' + i, $('mesh-k' + i).value); }
        const msg = $('mesh-msg');
        fetch('/api/mesh?' + q, { method: 'POST' }).then(r => r.json()).then(d => {
            msg.textContent = d.ok ? 'Salvo. Vale para os pacotes já no log também.' : 'PSK inválido (precisa ser base64 de até 32 bytes).';
            msg.style.color = d.ok ? 'var(--success)' : 'var(--fail)';
            if (d.ok) { meshDirty = false; showMeshChannels(d.mesh); reloadLog(); }
            setTimeout(() => { msg.textContent = ''; }, 5000);
        }).catch(() => { msg.textContent = 'Sem resposta da placa.'; });
    }

    // ---------- Protobuf mínimo (wire format) ----------
    function pb(b) {
        const out = [];
        let i = 0;
        // BigInt: int32 negativo vem como varint de 64 bits e não cabe num Number
        const varint = () => {
            let r = 0n, s = 0n, x;
            do { if (i >= b.length) throw new Error('fim'); x = b[i++]; r |= BigInt(x & 0x7f) << s; s += 7n; } while (x & 0x80);
            return r <= BigInt(Number.MAX_SAFE_INTEGER) ? Number(r) : r;
        };
        const take = n => { if (i + n > b.length) throw new Error('fim'); const v = b.slice(i, i + n); i += n; return v; };
        while (i < b.length) {
            const key = varint();
            if (typeof key !== 'number') throw new Error('chave');
            const f = Math.floor(key / 8), t = key & 7;
            if (f === 0) throw new Error('campo 0');
            if (t === 0) out.push({ f, t, v: varint() });
            else if (t === 1) out.push({ f, t, v: take(8) });
            else if (t === 2) out.push({ f, t, v: take(varint()) });
            else if (t === 5) out.push({ f, t, v: take(4) });
            else throw new Error('tipo ' + t);
        }
        return out;
    }
    const pbGet = (fs, n) => fs.find(x => x.f === n);
    const dv = v => new DataView(Uint8Array.from(v).buffer);
    const u32 = x => !x ? undefined : x.t === 5 ? dv(x.v).getUint32(0, true) : typeof x.v === 'bigint' ? Number(BigInt.asUintN(32, x.v)) : x.v;
    const i32f = x => !x ? undefined : x.t === 5 ? dv(x.v).getInt32(0, true) : x.v;  // sfixed32
    const f32 = x => !x || x.t !== 5 ? undefined : dv(x.v).getFloat32(0, true);
    const int = x => !x ? undefined : typeof x.v === 'bigint' ? Number(BigInt.asIntN(64, x.v)) : x.v;
    const str = x => !x || x.t !== 2 ? '' : new TextDecoder().decode(Uint8Array.from(x.v));
    const sub = x => !x || x.t !== 2 ? [] : pb(x.v);
    const fixed32List = x => {  // repeated fixed32 empacotado
        if (!x || x.t !== 2) return [];
        const r = [];
        for (let i = 0; i + 4 <= x.v.length; i += 4) r.push(dv(x.v.slice(i, i + 4)).getUint32(0, true));
        return r;
    };

    // ---------- Meshtastic ----------
    const BROADCAST = 0xFFFFFFFF;
    const PORTS = { 1: 'Texto', 3: 'Posição', 4: 'Info do nó', 5: 'Roteamento', 6: 'Admin', 7: 'Texto comprimido',
                    8: 'Waypoint', 10: 'Sensor de detecção', 32: 'Resposta', 33: 'Túnel IP', 34: 'Paxcounter',
                    64: 'Serial', 65: 'Store & Forward', 66: 'Teste de alcance', 67: 'Telemetria', 68: 'ZPS',
                    70: 'Traceroute', 71: 'Vizinhos', 72: 'ATAK', 73: 'Map report', 256: 'Privado', 257: 'ATAK' };
    const HW = { 1: 'TLORA_V2', 2: 'TLORA_V1', 3: 'TLORA_V2_1_1P6', 4: 'TBEAM', 5: 'HELTEC_V2_0', 6: 'TBEAM_V0P7',
                 7: 'T_ECHO', 8: 'TLORA_V1_1P3', 9: 'RAK4631', 10: 'HELTEC_V2_1', 11: 'HELTEC_V1',
                 39: 'DIY_V1', 43: 'HELTEC_V3', 44: 'HELTEC_WSL_V3', 48: 'HELTEC_WIRELESS_TRACKER', 50: 'T_DECK',
                 255: 'PRIVATE_HW' };
    const ROLES = ['CLIENT', 'CLIENT_MUTE', 'ROUTER', 'ROUTER_CLIENT', 'REPEATER', 'TRACKER', 'SENSOR', 'TAK',
                   'CLIENT_HIDDEN', 'LOST_AND_FOUND', 'TAK_TRACKER', 'ROUTER_LATE'];
    const ROUTING_ERR = { 0: 'ACK', 1: 'sem rota', 2: 'NAK', 3: 'timeout', 4: 'sem interface', 5: 'máx. retransmissões',
                          6: 'sem canal', 7: 'grande demais', 8: 'sem resposta', 9: 'limite de duty cycle',
                          32: 'requisição inválida', 33: 'não autorizado', 34: 'falha PKI', 35: 'chave pública desconhecida' };
    const hwName = n => HW[n] || (n !== undefined ? 'hw #' + n : '');
    const nodeId = n => n === BROADCAST ? 'todos' : '!' + (n >>> 0).toString(16).padStart(8, '0');

    function meshHeader(b) {
        if (b.length < 16) return null;
        const flags = b[12];
        return { to: le32(b, 0), from: le32(b, 4), id: le32(b, 8), hopLimit: flags & 7, hopStart: flags >> 5,
                 wantAck: !!(flags & 8), viaMqtt: !!(flags & 16), chHash: b[13], relay: b[15] };
    }

    const fmtCoord = (lat, lon) => lat.toFixed(5) + ', ' + lon.toFixed(5);
    const n1 = (v, d = 1) => v === undefined ? '' : v.toFixed(d);

    // Decodifica o protobuf Data decifrado. Devolve { port, text, node } (node = dados para a tabela de nós).
    function meshData(plain) {
        const data = pb(plain);
        const port = pbGet(data, 1) ? pbGet(data, 1).v : 0;
        const p = pbGet(data, 2);
        const payload = p ? p.v : [];
        const name = PORTS[port] || 'porta ' + port;
        const node = {};
        let text = name;
        try {
            if (port === 1 || port === 66) {
                const msg = new TextDecoder().decode(Uint8Array.from(payload));
                const emoji = u32(pbGet(data, 8));
                text = (emoji ? 'Reação' : name) + ': “' + msg + '”' + (u32(pbGet(data, 7)) ? ' (resposta)' : '');
            } else if (port === 3) {
                const f = pb(payload);
                const lat = i32f(pbGet(f, 1)), lon = i32f(pbGet(f, 2));
                if (lat !== undefined && lon !== undefined && (lat || lon)) {
                    node.lat = lat / 1e7; node.lon = lon / 1e7;
                    const alt = int(pbGet(f, 3)), sats = int(pbGet(f, 19));
                    if (alt !== undefined) node.alt = alt;
                    text = name + ': ' + fmtCoord(node.lat, node.lon) + (alt !== undefined ? ' · ' + alt + ' m' : '') +
                           (sats ? ' · ' + sats + ' satélites' : '');
                } else text = name + ': sem fix';
            } else if (port === 4) {
                const f = pb(payload);
                node.long = str(pbGet(f, 2)); node.short = str(pbGet(f, 3));
                node.hw = hwName(int(pbGet(f, 5)));
                const role = int(pbGet(f, 7));
                node.role = ROLES[role || 0] || 'papel ' + role;
                text = name + ': ' + node.long + ' (' + node.short + ') · ' + node.hw + ' · ' + node.role;
            } else if (port === 5) {
                const f = pb(payload);
                const err = pbGet(f, 3);
                text = name + ': ' + (err ? ROUTING_ERR[err.v] || 'erro ' + err.v : pbGet(f, 1) ? 'pedido de rota' : pbGet(f, 2) ? 'resposta de rota' : 'ACK');
            } else if (port === 67) {
                const f = pb(payload);
                const parts = [];
                const dm = sub(pbGet(f, 2));
                if (dm.length) {
                    const batt = u32(pbGet(dm, 1)), volt = f32(pbGet(dm, 2));
                    if (batt !== undefined) { node.batt = batt; parts.push(batt > 100 ? 'na tomada' : 'bateria ' + batt + '%'); }
                    if (volt !== undefined) { node.volt = volt; parts.push(n1(volt, 2) + ' V'); }
                    const cu = f32(pbGet(dm, 3)), at = f32(pbGet(dm, 4)), up = u32(pbGet(dm, 5));
                    if (cu !== undefined) parts.push('uso do canal ' + n1(cu) + '%');
                    if (at !== undefined) parts.push('TX ' + n1(at) + '%');
                    if (up !== undefined) parts.push('ligado há ' + Math.round(up / 3600) + ' h');
                }
                const env = sub(pbGet(f, 3));
                if (env.length) {
                    const t = f32(pbGet(env, 1)), h = f32(pbGet(env, 2)), pr = f32(pbGet(env, 3));
                    if (t !== undefined) parts.push(n1(t) + ' °C');
                    if (h !== undefined) parts.push(n1(h, 0) + '% umidade');
                    if (pr !== undefined) parts.push(n1(pr, 0) + ' hPa');
                }
                const ls = sub(pbGet(f, 6));
                if (ls.length) {
                    const on = u32(pbGet(ls, 7)), tot = u32(pbGet(ls, 8));
                    if (on !== undefined) parts.push(on + '/' + (tot || '?') + ' nós online');
                    const rx = u32(pbGet(ls, 5)), bad = u32(pbGet(ls, 6));
                    if (rx !== undefined) parts.push(rx + ' RX (' + (bad || 0) + ' ruins)');
                }
                if (pbGet(f, 5)) parts.push('métricas de energia');
                text = name + (parts.length ? ': ' + parts.join(' · ') : '');
            } else if (port === 70) {
                const f = pb(payload);
                const route = fixed32List(pbGet(f, 1)).map(nodeName);
                const back = fixed32List(pbGet(f, 3)).map(nodeName);
                text = name + (route.length ? ': ida via ' + route.join(' → ') : ': direto') + (back.length ? ' · volta via ' + back.join(' → ') : '');
            } else if (port === 71) {
                const f = pb(payload);
                const nb = f.filter(x => x.f === 4).map(x => {
                    const g = pb(x.v);
                    return nodeName(u32(pbGet(g, 1))) + ' (' + n1(f32(pbGet(g, 2))) + ' dB)';
                });
                text = name + ': ' + (nb.length ? nb.join(', ') : 'nenhum');
            } else if (port === 73) {
                const f = pb(payload);
                node.long = str(pbGet(f, 1)); node.short = str(pbGet(f, 2));
                node.role = ROLES[int(pbGet(f, 3)) || 0]; node.hw = hwName(int(pbGet(f, 4)));
                const lat = i32f(pbGet(f, 9)), lon = i32f(pbGet(f, 10));
                if (lat && lon) { node.lat = lat / 1e7; node.lon = lon / 1e7; }
                text = name + ': ' + node.long + ' (' + node.short + ') · fw ' + str(pbGet(f, 5)) +
                       (node.lat !== undefined ? ' · ' + fmtCoord(node.lat, node.lon) : '') +
                       (pbGet(f, 13) ? ' · ' + pbGet(f, 13).v + ' nós por perto' : '');
            } else if (port === 8) {
                const f = pb(payload);
                const lat = i32f(pbGet(f, 2)), lon = i32f(pbGet(f, 3));
                text = name + ': ' + str(pbGet(f, 6)) + (lat && lon ? ' · ' + fmtCoord(lat / 1e7, lon / 1e7) : '');
            } else {
                text = name + ' (' + payload.length + ' bytes)';
            }
        } catch (e) {
            text = name + ' (conteúdo não decodificado)';
        }
        return { port, text, node };
    }

    // ---------- Tabela de nós Meshtastic (montada no navegador a partir do que foi ouvido) ----------
    const nodes = new Map(Object.entries(store.get('mesh-nodes', {})).map(([k, v]) => [+k, v]));
    const seenPackets = new Map();  // "from-id" -> vezes ouvido no log atual (a malha retransmite o mesmo pacote)
    const countedPackets = new Set();  // "from-id" já contados na tabela (o log pode ser recarregado)

    function nodeName(num) {
        if (num === undefined) return '?';
        const n = nodes.get(num);
        return nodeId(num) + (n && n.short ? ' (' + n.short + ')' : '');
    }

    function noteNode(num, patch) {
        const n = nodes.get(num) || { packets: 0 };
        for (const [k, v] of Object.entries(patch)) if (v !== undefined && v !== '') n[k] = v;
        nodes.set(num, n);
    }

    function saveNodes() {
        store.set('mesh-nodes', Object.fromEntries(nodes));
        renderNodes();
    }

    function ago(t) {
        const s = Math.round((Date.now() - t) / 1000);
        return s < 60 ? s + ' s' : s < 3600 ? Math.round(s / 60) + ' min' : Math.round(s / 3600) + ' h';
    }

    function renderNodes() {
        $('nodes-count').textContent = nodes.size;
        if ($('pane-nodes').hidden) return;
        const tbody = $('nodes');
        tbody.innerHTML = '';
        if (!nodes.size) {
            tbody.innerHTML = '<tr><td colspan="9" class="empty">Nenhum nó ouvido ainda (precisa do modo Meshtastic no LoRa)</td></tr>';
            return;
        }
        const sorted = [...nodes.entries()].sort((a, b) => (b[1].seen || 0) - (a[1].seen || 0));
        for (const [num, n] of sorted) {
            const tr = document.createElement('tr');
            const cells = [
                nodeId(num) + (n.short ? ' · ' + n.short : ''),
                n.long || '',
                [n.hw, n.role].filter(Boolean).join(' · '),
                n.seen ? ago(n.seen) : '',
                n.hops === undefined ? '' : n.hops === 0 ? 'direto' : n.hops,
                n.rssi === undefined ? '' : n.rssi + ' dBm / ' + n1(n.snr) + ' dB',
                n.batt === undefined ? '' : n.batt > 100 ? 'tomada' : n.batt + '%' + (n.volt ? ' · ' + n1(n.volt, 2) + ' V' : ''),
                '',
                n.packets,
            ];
            cells.forEach((c, i) => {
                const td = document.createElement('td');
                if (i === 0) td.className = 'mono';
                if (i === 7 && n.lat !== undefined) {
                    const a = document.createElement('a');
                    a.href = 'https://www.openstreetmap.org/?mlat=' + n.lat + '&mlon=' + n.lon + '#map=15/' + n.lat + '/' + n.lon;
                    a.target = '_blank';
                    a.rel = 'noopener';
                    a.textContent = fmtCoord(n.lat, n.lon);
                    td.appendChild(a);
                } else td.textContent = c;
                tr.appendChild(td);
            });
            tbody.appendChild(tr);
        }
    }

    // ---------- Decodificação de cabeçalhos ----------
    function bytesOf(hex) {
        const b = [];
        for (let i = 0; i < hex.length; i += 2) b.push(parseInt(hex.substr(i, 2), 16));
        return b;
    }
    const le32 = (b, i) => (b[i] | b[i + 1] << 8 | b[i + 2] << 16 | b[i + 3] << 24) >>> 0;
    const h8 = n => n.toString(16).toUpperCase().padStart(8, '0');
    const euiOf = (b, i) => b.slice(i, i + 8).reverse().map(x => x.toString(16).toUpperCase().padStart(2, '0')).join('');

    function decodeLoRaWAN(b) {
        if (b.length < 5) return null;
        const types = ['Join Request', 'Join Accept', 'Unconfirmed Up', 'Unconfirmed Down',
                       'Confirmed Up', 'Confirmed Down', 'Rejoin', 'Proprietário'];
        const mtype = b[0] >> 5;
        if ((b[0] & 3) !== 0) return 'LoRaWAN? versão desconhecida (MHDR ' + b[0].toString(16) + ')';
        let s = 'LoRaWAN ' + types[mtype];
        if (mtype === 0 && b.length === 23) {
            s += ' · JoinEUI ' + euiOf(b, 1) + ' · DevEUI ' + euiOf(b, 9);
        } else if (mtype >= 2 && mtype <= 5 && b.length >= 12) {
            const foptsLen = b[5] & 0x0F;
            const fcnt = b[6] | b[7] << 8;
            s += ' · DevAddr ' + h8(le32(b, 1)) + ' · FCnt ' + fcnt;
            if (b.length > 8 + foptsLen + 4) s += ' · FPort ' + b[8 + foptsLen];
            if (b[5] & 0x80) s += ' · ADR';
        }
        return s;
    }

    function mostlyBinary(b) {
        if (!b.length) return false;
        const printable = b.filter(x => x >= 0x20 && x < 0x7F).length;
        return printable / b.length < 0.85;
    }

    // Uma vez por evento: cabeçalho, decifrado, duplicata e tabela de nós
    function analyze(e) {
        const b = bytesOf(e.hex);
        e.bytes = b;
        if (e.tx) return;
        if (e.proto === 'lorawan') { e.protoLine = decodeLoRaWAN(b); return; }
        if (e.proto !== 'meshtastic') return;
        const h = meshHeader(b);
        if (!h) { e.protoLine = 'Meshtastic? pacote curto demais'; return; }
        e.mesh = h;
        const key = h.from + '-' + h.id;
        const times = (seenPackets.get(key) || 0) + 1;
        seenPackets.set(key, times);
        e.dup = times > 1;
        let decoded = null;
        if (e.plain) {
            // Hash do canal bateu mas o protobuf não abre: outro canal com o mesmo hash, ou chave errada
            try { decoded = meshData(bytesOf(e.plain)); }
            catch (err) { decoded = { text: 'Decifrado com ' + e.chan + ', mas o conteúdo não é válido (chave errada?)', node: {} }; }
        }
        e.decoded = decoded;
        if (!e.dup && !countedPackets.has(key)) {
            countedPackets.add(key);
            const hops = h.hopStart ? h.hopStart - h.hopLimit : undefined;
            noteNode(h.from, Object.assign({ seen: Date.now(), hops },
                hops === 0 ? { rssi: e.rssi, snr: e.snr } : {}, decoded ? decoded.node : {}));
            nodes.get(h.from).packets++;
            saveNodes();
        }
    }

    function renderMsg(td) {
        const e = td._event;
        const b = e.bytes;
        const hexMode = $('hex-mode').checked;
        td.textContent = '';
        const add = (cls, text) => {
            const s = document.createElement('span');
            s.className = cls;
            s.textContent = text;
            td.appendChild(s);
        };
        if (e.mesh) {
            const h = e.mesh;
            add('proto', 'Meshtastic' + (e.chan ? ' · ' + e.chan : '') + ' · de ' + nodeName(h.from) + ' para ' + nodeName(h.to) +
                ' · saltos ' + (h.hopStart - h.hopLimit) + '/' + h.hopStart + (h.wantAck ? ' · pede ACK' : '') +
                (h.viaMqtt ? ' · via MQTT' : '') + (e.dup ? ' · repetido (relay 0x' + h.relay.toString(16).padStart(2, '0') + ')' : ''));
            if (e.decoded) add('app', e.decoded.text);
            else if (h.chHash === 0 && h.to !== BROADCAST) add('muted', 'Mensagem direta com criptografia por nó (PKI): não dá para ler.');
            else add('muted', 'Canal desconhecido (hash 0x' + h.chHash.toString(16).padStart(2, '0') + '): adicione nome + PSK em Canais Meshtastic.');
            if (hexMode) add('hex', e.hex.replace(/(..)(?!$)/g, '$1 ') + ' (' + b.length + ' B)');
            return;
        }
        if (e.protoLine) add('proto', e.protoLine);
        if (hexMode || e.protoLine || mostlyBinary(b)) add('hex', e.hex.replace(/(..)(?!$)/g, '$1 ') + ' (' + b.length + ' B)');
        else add('app', e.msg);
    }

    function rerenderMsgs() {
        document.querySelectorAll('#log td.msg').forEach(renderMsg);
    }

    // Canais mudaram: pede o log de novo para vir decifrado com as chaves novas
    function reloadLog() {
        lastId = 0;
        $('log').innerHTML = '';
        seenPackets.clear();
        resetCounts();
        refreshLog();
    }

    function fmtTime(ms) {
        const s = ms / 1000;
        const m = Math.floor(s / 60);
        return m + ':' + (s % 60).toFixed(1).padStart(4, '0');
    }

    function applyFilter() {
        const radio = $('log-filter').value, hideDup = $('hide-dup').checked;
        for (const tr of $('log').children) {
            if (!tr.dataset.radio) continue;
            tr.hidden = (radio && tr.dataset.radio !== radio) || (hideDup && tr.classList.contains('dup'));
        }
    }

    function refreshLog() {
        fetch('/api/log?since=' + lastId).then(r => r.json()).then(d => {
            // next menor que o último id visto: a placa reiniciou, recomeça a contagem
            if (d.next <= lastId) { lastId = 0; return; }
            if (!d.events.length) {
                if (!$('log').children.length) $('log').innerHTML = '<tr><td colspan="7" class="empty">Nenhum pacote ainda</td></tr>';
                return;
            }
            const tbody = $('log');
            const empty = tbody.querySelector('.empty');
            if (empty) empty.parentElement.remove();
            for (const e of d.events) {
                lastId = Math.max(lastId, e.id);
                analyze(e);
                const radio = e.radio;
                const isLora = radio === 'lora';
                counts[radio][e.tx ? 'tx' : 'rx']++;
                if (!e.tx) {
                    $(radio + '-rssi').textContent = e.rssi + ' dBm';
                    if (isLora) $('lora-snr').textContent = e.snr.toFixed(1) + ' dB';
                }
                const tr = document.createElement('tr');
                tr.dataset.radio = radio;
                if (e.dup) tr.className = 'dup';
                const cells = [
                    fmtTime(e.ms),
                    isLora ? 'LoRa' : 'CC1101',
                    e.tx ? '↑ TX' : '↓ RX',
                    '',
                    e.tx ? '' : e.rssi + ' dBm',
                    e.tx || !isLora ? '' : e.snr.toFixed(1),
                    e.ok ? 'OK' : (e.tx ? 'FALHA' : 'CRC RUIM'),
                ];
                cells.forEach((c, i) => {
                    const td = document.createElement('td');
                    td.textContent = c;
                    if (i === 2) td.className = e.tx ? 'dir-tx' : 'dir-rx';
                    if (i === 3) { td.className = 'msg'; td._event = e; renderMsg(td); }
                    if (i === 6) td.style.color = e.ok ? 'var(--success)' : 'var(--fail)';
                    tr.appendChild(td);
                });
                tbody.insertBefore(tr, tbody.firstChild);
            }
            while (tbody.children.length > 300) tbody.removeChild(tbody.lastChild);
            updateCounts();
            applyFilter();
        }).catch(() => {});
    }

    function updateCounts() {
        for (const r of ['cc1101', 'lora']) {
            $(r + '-tx').textContent = counts[r].tx;
            $(r + '-rx').textContent = counts[r].rx;
        }
    }

    function resetCounts() {
        for (const r of ['cc1101', 'lora']) counts[r].tx = counts[r].rx = 0;
        updateCounts();
    }

    function clearLog() {
        $('log').innerHTML = '<tr><td colspan="7" class="empty">Nenhum pacote ainda</td></tr>';
        resetCounts();
        if (!$('pane-nodes').hidden && confirm('Apagar também a tabela de nós?')) {
            nodes.clear();
            countedPackets.clear();
            saveNodes();
        }
    }

    function showTab(tab) {
        $('pane-log').hidden = tab !== 'log';
        $('pane-nodes').hidden = tab !== 'nodes';
        $('tab-log').classList.toggle('active', tab === 'log');
        $('tab-nodes').classList.toggle('active', tab === 'nodes');
        renderNodes();
    }

    // ---------- Sniff: RSSI de cada rádio no tempo, sem decodificar ----------
    const SNIFF_N = 500;  // 10 s com janelas de 20 ms
    const SNIFF_TOP = -20, SNIFF_BOTTOM = -125;
    const sniff = { cc1101: { id: 0, r: [], f: [], paused: false }, lora: { id: 0, r: [], f: [], paused: false } };

    function refreshSniff() {
        for (const name of Object.keys(sniff)) {
            const s = sniff[name];
            if (s.paused) continue;
            fetch('/api/sniff?radio=' + name + '&since=' + s.id).then(r => r.json()).then(d => {
                // next menor que o último id visto: a placa reiniciou, recomeça o histórico
                if (d.next <= s.id) { s.id = 0; s.r = []; s.f = []; return; }
                s.r.push(...d.r);
                s.f.push(...d.f);
                s.id = d.next - 1;
                if (s.r.length > SNIFF_N) {
                    s.r.splice(0, s.r.length - SNIFF_N);
                    s.f.splice(0, s.f.length - SNIFF_N);
                }
                drawSniff(name);
            }).catch(() => {});
        }
    }

    function toggleSniff(name) {
        const s = sniff[name];
        s.paused = !s.paused;
        $('sniff-' + name + '-pause').textContent = s.paused ? 'Continuar' : 'Pausar';
    }

    function drawSniff(name) {
        const { r: data, f: flags } = sniff[name];
        const cv = $('sniff-' + name + '-chart');
        const dpr = window.devicePixelRatio || 1;
        const w = cv.clientWidth, h = cv.clientHeight;
        if (cv.width !== w * dpr || cv.height !== h * dpr) { cv.width = w * dpr; cv.height = h * dpr; }
        const ctx = cv.getContext('2d');
        ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
        ctx.clearRect(0, 0, w, h);
        const css = getComputedStyle(document.documentElement);
        const color = n => css.getPropertyValue(n).trim();
        const left = 40, right = w - 8, top = 14, bottom = h - 8;
        const y = v => top + (SNIFF_TOP - Math.max(SNIFF_BOTTOM, Math.min(SNIFF_TOP, v))) / (SNIFF_TOP - SNIFF_BOTTOM) * (bottom - top);
        const x = i => right - (SNIFF_N - 1 - i) * (right - left) / (SNIFF_N - 1);

        // Grade de 20 em 20 dBm
        ctx.font = '11px sans-serif';
        ctx.fillStyle = color('--text-secondary');
        ctx.strokeStyle = color('--border');
        ctx.lineWidth = 1;
        for (let v = SNIFF_TOP; v >= SNIFF_BOTTOM; v -= 20) {
            ctx.beginPath(); ctx.moveTo(left, y(v)); ctx.lineTo(right, y(v)); ctx.stroke();
            ctx.fillText(v, 4, y(v) + 4);
        }
        if (!data.length) return;

        // Alinha à direita: a amostra mais nova fica na borda
        const off = SNIFF_N - data.length;
        const prim = color('--primary');
        ctx.beginPath();
        data.forEach((v, i) => i ? ctx.lineTo(x(off + i), y(v)) : ctx.moveTo(x(off + i), y(v)));
        ctx.strokeStyle = prim;
        ctx.lineWidth = 1.5;
        ctx.stroke();
        ctx.lineTo(x(SNIFF_N - 1), bottom); ctx.lineTo(x(off), bottom); ctx.closePath();
        ctx.globalAlpha = 0.15; ctx.fillStyle = prim; ctx.fill(); ctx.globalAlpha = 1;

        // Marcas de pacote no topo
        const marks = [[2, '--tx'], [1, '--success'], [4, '--fail']];
        flags.forEach((f, i) => {
            if (!f) return;
            for (const [bit, c] of marks) {
                if (!(f & bit)) continue;
                ctx.fillStyle = color(c);
                ctx.fillRect(x(off + i) - 1.5, 2, 3, 9);
                ctx.globalAlpha = 0.25;
                ctx.fillRect(x(off + i) - 0.5, top, 1, bottom - top);
                ctx.globalAlpha = 1;
            }
        });

        const sorted = [...data].sort((a, b) => a - b);
        $('sniff-' + name + '-now').textContent = data[data.length - 1] + ' dBm';
        $('sniff-' + name + '-max').textContent = sorted[sorted.length - 1] + ' dBm';
        // Piso = mediana: os picos de transmissão quase nunca ocupam metade do tempo
        $('sniff-' + name + '-floor').textContent = sorted[Math.floor(sorted.length / 2)] + ' dBm';
    }

    for (const r of ['cc1101', 'lora']) {
        $(r + '-msg').addEventListener('keydown', e => { if (e.key === 'Enter') send(r); });
    }
    refreshStatus();
    refreshLog();
    renderNodes();
    setInterval(refreshStatus, 2000);
    setInterval(refreshLog, 700);
    setInterval(refreshSniff, 250);
    setInterval(() => { if (!$('pane-nodes').hidden) renderNodes(); }, 5000);
    window.addEventListener('resize', () => Object.keys(sniff).forEach(drawSniff));
</script>
</body>
</html>
)rawliteral";
