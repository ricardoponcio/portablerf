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
        body { background-color: var(--bg-color); color: var(--text); padding: 16px; }
        .wrap { max-width: 960px; margin: 0 auto; }
        h1 { color: var(--primary); font-size: 1.5rem; }
        .sub { color: var(--text-secondary); font-size: 0.85rem; margin: 4px 0 16px; }
        .grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(280px, 1fr)); gap: 16px; }
        .card { background-color: var(--card-bg); border: 1px solid var(--border); border-radius: 12px; padding: 16px; }
        .card h2 { font-size: 1.05rem; display: flex; justify-content: space-between; align-items: center; margin-bottom: 12px; }
        .badge { padding: 3px 10px; border-radius: 10px; font-size: 0.75rem; font-weight: 600; background: rgba(148,163,184,.2); color: var(--text-secondary); }
        .ok { background-color: rgba(34, 197, 94, 0.2); color: var(--success); }
        .bad { background-color: rgba(239, 68, 68, 0.2); color: var(--fail); }
        .row { display: flex; gap: 8px; margin-bottom: 10px; }
        input[type=text] { flex: 1; min-width: 0; background: var(--inner-bg); border: 1px solid var(--border); color: var(--text); border-radius: 8px; padding: 8px 10px; font-size: 0.95rem; }
        button { background: var(--primary); color: #0f172a; border: 0; border-radius: 8px; padding: 8px 14px; font-weight: 600; cursor: pointer; }
        button:disabled { opacity: .4; cursor: default; }
        select { background: var(--inner-bg); color: var(--text); border: 1px solid var(--border); border-radius: 8px; padding: 6px; }
        .auto { display: flex; align-items: center; gap: 8px; font-size: 0.85rem; color: var(--text-secondary); }
        .stats { display: flex; gap: 16px; margin-top: 12px; font-size: 0.85rem; color: var(--text-secondary); }
        .stats b { color: var(--text); }
        .log { margin-top: 16px; }
        .log h2 { justify-content: space-between; }
        .tbl { width: 100%; border-collapse: collapse; font-size: 0.85rem; }
        .tbl th { text-align: left; color: var(--text-secondary); font-weight: 500; padding: 6px; border-bottom: 1px solid var(--border); }
        .tbl td { padding: 6px; border-bottom: 1px solid #1f2a3d; font-variant-numeric: tabular-nums; }
        .tbl td.msg { font-family: ui-monospace, monospace; word-break: break-all; }
        .dir-tx { color: var(--tx); font-weight: 600; }
        .dir-rx { color: var(--success); font-weight: 600; }
        .scroll { max-height: 420px; overflow-y: auto; }
        .empty { color: var(--text-secondary); text-align: center; padding: 20px; }
    </style>
</head>
<body>
<div class="wrap">
    <h1>ESP32 Rádios</h1>
    <p class="sub" id="wifi">Carregando...</p>

    <div class="grid">
        <div class="card" data-radio="cc1101">
            <h2>CC1101 · 433,92 MHz <span class="badge" id="cc1101-badge">...</span></h2>
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
            <div class="stats">
                <span>TX <b id="cc1101-tx">0</b></span>
                <span>RX <b id="cc1101-rx">0</b></span>
                <span>Último RSSI <b id="cc1101-rssi">--</b></span>
            </div>
        </div>

        <div class="card" data-radio="lora">
            <h2>LoRa SX1262 · 915 MHz <span class="badge" id="lora-badge">...</span></h2>
            <div class="row">
                <input type="text" id="lora-msg" maxlength="60" placeholder="Mensagem (até 60 caracteres)">
                <button onclick="send('lora')">Enviar</button>
            </div>
            <label class="auto">
                <input type="checkbox" id="lora-auto" onchange="setAuto('lora')"> Auto-ping a cada
                <select id="lora-int" onchange="setAuto('lora')">
                    <option value="1000">1 s</option><option value="2000" selected>2 s</option><option value="5000">5 s</option>
                </select>
            </label>
            <div class="stats">
                <span>TX <b id="lora-tx">0</b></span>
                <span>RX <b id="lora-rx">0</b></span>
                <span>Último RSSI <b id="lora-rssi">--</b></span>
                <span>SNR <b id="lora-snr">--</b></span>
            </div>
        </div>
    </div>

    <div class="card log">
        <h2>Log <button onclick="clearLog()" style="font-size:.75rem;padding:4px 10px">Limpar</button></h2>
        <div class="scroll">
            <table class="tbl">
                <thead><tr><th>Tempo</th><th>Rádio</th><th></th><th>Mensagem</th><th>RSSI</th><th>SNR</th><th>Status</th></tr></thead>
                <tbody id="log"><tr><td colspan="7" class="empty">Nenhum pacote ainda</td></tr></tbody>
            </table>
        </div>
    </div>
</div>
<script>
    let lastId = 0;
    const counts = { cc1101: { tx: 0, rx: 0 }, lora: { tx: 0, rx: 0 } };

    function badge(id, text, cls) {
        const el = document.getElementById(id);
        el.textContent = text;
        el.className = 'badge ' + cls;
    }

    function refreshStatus() {
        fetch('/api/status').then(r => r.json()).then(d => {
            document.getElementById('wifi').textContent = d.ssid + ' · ' + d.ip + ' · ' + d.wifi_rssi + ' dBm';
            for (const [name, r] of Object.entries(d.radios)) {
                if (r.pending) badge(name + '-badge', 'INICIALIZANDO', '');
                else if (r.ok) badge(name + '-badge', 'OK', 'ok');
                else badge(name + '-badge', 'FALHA (' + r.state + ')', 'bad');
                // Mantém o auto-ping em sincronia com o firmware (ex.: após reiniciar a placa)
                const cb = document.getElementById(name + '-auto');
                if (document.activeElement !== cb) cb.checked = r.auto_ms > 0;
            }
        }).catch(() => {});
    }

    function send(radio) {
        const input = document.getElementById(radio + '-msg');
        const msg = input.value.trim();
        if (!msg) return;
        fetch('/api/send?radio=' + radio + '&msg=' + encodeURIComponent(msg), { method: 'POST' })
            .then(r => r.json()).then(d => { if (d.ok) input.value = ''; refreshLog(); });
    }

    function setAuto(radio) {
        const on = document.getElementById(radio + '-auto').checked;
        const ms = on ? document.getElementById(radio + '-int').value : 0;
        fetch('/api/auto?radio=' + radio + '&ms=' + ms, { method: 'POST' });
    }

    function fmtTime(ms) {
        const s = ms / 1000;
        const m = Math.floor(s / 60);
        return m + ':' + (s % 60).toFixed(1).padStart(4, '0');
    }

    function refreshLog() {
        fetch('/api/log?since=' + lastId).then(r => r.json()).then(d => {
            if (!d.events.length) return;
            const tbody = document.getElementById('log');
            const empty = tbody.querySelector('.empty');
            if (empty) empty.parentElement.remove();
            for (const e of d.events) {
                lastId = Math.max(lastId, e.id);
                const radio = e.radio;
                const isLora = radio === 'lora';
                counts[radio][e.tx ? 'tx' : 'rx']++;
                if (!e.tx) {
                    document.getElementById(radio + '-rssi').textContent = e.rssi + ' dBm';
                    if (isLora) document.getElementById('lora-snr').textContent = e.snr.toFixed(1) + ' dB';
                }
                const tr = document.createElement('tr');
                const cells = [
                    fmtTime(e.ms),
                    isLora ? 'LoRa' : 'CC1101',
                    e.tx ? '↑ TX' : '↓ RX',
                    e.msg,
                    e.tx ? '' : e.rssi + ' dBm',
                    e.tx || !isLora ? '' : e.snr.toFixed(1),
                    e.ok ? 'OK' : (e.tx ? 'FALHA' : 'CRC RUIM'),
                ];
                cells.forEach((c, i) => {
                    const td = document.createElement('td');
                    td.textContent = c;
                    if (i === 2) td.className = e.tx ? 'dir-tx' : 'dir-rx';
                    if (i === 3) td.className = 'msg';
                    if (i === 6) td.style.color = e.ok ? 'var(--success)' : 'var(--fail)';
                    tr.appendChild(td);
                });
                tbody.insertBefore(tr, tbody.firstChild);
            }
            while (tbody.children.length > 200) tbody.removeChild(tbody.lastChild);
            for (const r of ['cc1101', 'lora']) {
                document.getElementById(r + '-tx').textContent = counts[r].tx;
                document.getElementById(r + '-rx').textContent = counts[r].rx;
            }
        }).catch(() => {});
    }

    function clearLog() {
        document.getElementById('log').innerHTML = '<tr><td colspan="7" class="empty">Nenhum pacote ainda</td></tr>';
        for (const r of ['cc1101', 'lora']) {
            counts[r].tx = counts[r].rx = 0;
            document.getElementById(r + '-tx').textContent = 0;
            document.getElementById(r + '-rx').textContent = 0;
        }
    }

    for (const r of ['cc1101', 'lora']) {
        document.getElementById(r + '-msg').addEventListener('keydown', e => { if (e.key === 'Enter') send(r); });
    }
    refreshStatus();
    refreshLog();
    setInterval(refreshStatus, 2000);
    setInterval(refreshLog, 700);
</script>
</body>
</html>
)rawliteral";
