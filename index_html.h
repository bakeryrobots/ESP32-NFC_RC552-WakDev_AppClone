#ifndef INDEX_HTML_H
#define INDEX_HTML_H

#include <Arduino.h>

const char INDEX_HTML[] PROGMEM = R"HTML(
<!DOCTYPE html>
<html lang="id">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>NFC Tools — ESP32 & RC522</title>
  <style>
    :root {
      --bg: #0f172a;
      --card: #1e293b;
      --card-border: #334155;
      --text: #f8fafc;
      --text-muted: #94a3b8;
      --primary: #38bdf8;
      --primary-hover: #0ea5e9;
      --success: #10b981;
      --danger: #ef4444;
      --warning: #f59e0b;
      --radius: 12px;
    }
    * { box-sizing: border-box; margin: 0; padding: 0; font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; }
    body { background-color: var(--bg); color: var(--text); padding: 20px 10px; display: flex; justify-content: center; }
    .container { width: 100%; max-width: 680px; }
    .header { text-align: center; margin-bottom: 24px; }
    .header h1 { font-size: 1.8rem; font-weight: 700; color: var(--primary); display: flex; align-items: center; justify-content: center; gap: 8px; }
    .header p { color: var(--text-muted); font-size: 0.9rem; margin-top: 4px; }
    .badge-bar { display: flex; justify-content: center; gap: 10px; margin-top: 10px; }
    .badge { font-size: 0.75rem; padding: 4px 10px; border-radius: 999px; background: rgba(56, 189, 248, 0.15); color: var(--primary); font-weight: 600; border: 1px solid rgba(56, 189, 248, 0.3); }

    .tabs { display: flex; gap: 6px; background: rgba(30, 41, 59, 0.8); padding: 6px; border-radius: var(--radius); margin-bottom: 18px; border: 1px solid var(--card-border); }
    .tab-btn { flex: 1; padding: 10px 8px; border: none; background: transparent; color: var(--text-muted); font-weight: 600; font-size: 0.85rem; border-radius: 8px; cursor: pointer; transition: all 0.2s; text-align: center; }
    .tab-btn.active { background: var(--primary); color: #0f172a; box-shadow: 0 2px 8px rgba(56, 189, 248, 0.3); }

    .panel { display: none; }
    .panel.active { display: block; }

    .card { background: var(--card); border: 1px solid var(--card-border); border-radius: var(--radius); padding: 20px; margin-bottom: 16px; box-shadow: 0 4px 20px rgba(0,0,0,0.25); }
    .card-title { font-size: 1.1rem; font-weight: 600; margin-bottom: 14px; display: flex; align-items: center; gap: 8px; color: #fff; }
    
    .info-row { display: flex; justify-content: space-between; padding: 9px 0; border-bottom: 1px solid rgba(51, 65, 85, 0.5); font-size: 0.9rem; }
    .info-row:last-child { border-bottom: none; }
    .info-label { color: var(--text-muted); }
    .info-val { font-weight: 600; text-align: right; color: var(--text); word-break: break-all; }
    
    .ndef-preview { background: rgba(15, 23, 42, 0.6); border: 1px dashed var(--primary); border-radius: 8px; padding: 14px; margin-top: 12px; }
    .ndef-url { font-size: 1.05rem; font-weight: 600; color: var(--primary); word-break: break-all; display: block; margin: 4px 0 8px 0; }
    
    .form-group { margin-bottom: 14px; }
    label { display: block; font-size: 0.85rem; font-weight: 600; color: var(--text-muted); margin-bottom: 6px; }
    input[type="text"], select { width: 100%; padding: 11px 14px; background: rgba(15, 23, 42, 0.8); border: 1px solid var(--card-border); border-radius: 8px; color: var(--text); font-size: 0.95rem; outline: none; transition: 0.2s; }
    input[type="text"]:focus, select:focus { border-color: var(--primary); box-shadow: 0 0 0 2px rgba(56, 189, 248, 0.2); }
    
    .btn { display: inline-flex; align-items: center; justify-content: center; gap: 8px; width: 100%; padding: 12px; border-radius: 8px; border: none; font-size: 0.95rem; font-weight: 600; cursor: pointer; transition: all 0.2s; }
    .btn-primary { background: var(--primary); color: #0f172a; }
    .btn-primary:hover { background: var(--primary-hover); }
    .btn-danger { background: rgba(239, 68, 68, 0.15); color: var(--danger); border: 1px solid var(--danger); }
    .btn-danger:hover { background: var(--danger); color: #fff; }
    .btn-warning { background: rgba(245, 158, 11, 0.15); color: var(--warning); border: 1px solid var(--warning); }
    .btn-warning:hover { background: var(--warning); color: #0f172a; }
    .btn-outline { background: transparent; color: var(--primary); border: 1px solid var(--primary); }
    .btn-outline:hover { background: rgba(56, 189, 248, 0.1); }
    
    .alert { padding: 12px 14px; border-radius: 8px; font-size: 0.88rem; margin-top: 14px; display: none; line-height: 1.4; }
    .alert-success { background: rgba(16, 185, 129, 0.15); border: 1px solid var(--success); color: var(--success); }
    .alert-error { background: rgba(239, 68, 68, 0.15); border: 1px solid var(--danger); color: var(--danger); }
    .alert-info { background: rgba(56, 189, 248, 0.15); border: 1px solid var(--primary); color: var(--primary); }
    
    .tip-box { background: rgba(15, 23, 42, 0.7); border-left: 4px solid var(--primary); padding: 10px 14px; font-size: 0.8rem; color: var(--text-muted); border-radius: 0 8px 8px 0; margin-top: 10px; line-height: 1.5; }
    
    .dump-table { width: 100%; border-collapse: collapse; font-family: monospace; font-size: 0.78rem; margin-top: 10px; }
    .dump-table th, .dump-table td { padding: 6px 8px; border: 1px solid var(--card-border); text-align: left; }
    .dump-table th { background: rgba(15, 23, 42, 0.8); color: var(--primary); }
    .dump-table tr:nth-child(even) { background: rgba(15, 23, 42, 0.4); }

    .autoscan-bar {
      display: flex;
      justify-content: space-between;
      align-items: center;
      background: rgba(15, 23, 42, 0.7);
      padding: 10px 14px;
      border-radius: 8px;
      margin-bottom: 14px;
      border: 1px solid var(--card-border);
    }
  </style>
</head>
<body>
  <div class="container">
    <div class="header">
      <h1>📡 NFC Tools — ESP32</h1>
      <p>Reader & Writer 13.56 MHz (NTAG & MIFARE Classic)</p>
      <div class="badge-bar">
        <span class="badge">RC522: Siap</span>
        <span class="badge">Auto-Scan & Debounce: Aktif</span>
      </div>
    </div>

    <!-- Navigasi Tab -->
    <div class="tabs">
      <button class="tab-btn active" onclick="openTab('read')">🔍 Baca Tag</button>
      <button class="tab-btn" onclick="openTab('write')">✍️ Tulis Tag</button>
      <button class="tab-btn" onclick="openTab('tools')">🛠️ Format & Alat</button>
      <button class="tab-btn" onclick="openTab('dump')">📊 Hex Dump</button>
    </div>

    <!-- TAB 1: BACA TAG -->
    <div id="panel-read" class="panel active">
      <div class="card">
        <div class="card-title">Informasi Kartu Terdeteksi</div>
        
        <!-- Kontrol Auto Scan & Debounce -->
        <div class="autoscan-bar">
          <div style="display:flex; align-items:center; gap:8px;">
            <input type="checkbox" id="autoscan-toggle" onchange="toggleAutoScan()" style="width:18px; height:18px; cursor:pointer;" checked>
            <label for="autoscan-toggle" style="margin:0; cursor:pointer; color:var(--text); font-size:0.9rem;">⚡ Mode Auto Scan</label>
          </div>
          <div style="display:flex; align-items:center; gap:6px;">
            <span style="font-size:0.8rem; color:var(--text-muted);">Debounce:</span>
            <select id="autoscan-debounce" onchange="changeDebounce()" style="width:auto; padding:4px 8px; font-size:0.8rem;">
              <option value="2000">2 Detik</option>
              <option value="3000" selected>3 Detik</option>
              <option value="5000">5 Detik</option>
              <option value="10000">10 Detik</option>
            </select>
          </div>
        </div>

        <div id="read-content">
          <div class="info-row"><span class="info-label">Status</span><span class="info-val" id="card-status">Tempelkan kartu ke reader RC522...</span></div>
          <div class="info-row"><span class="info-label">UID / Serial</span><span class="info-val" id="card-uid">-</span></div>
          <div class="info-row"><span class="info-label">Tipe Kartu</span><span class="info-val" id="card-type">-</span></div>
          <div class="info-row"><span class="info-label">Kapasitas User</span><span class="info-val" id="card-size">-</span></div>
        </div>

        <div id="ndef-box" style="display:none;" class="ndef-preview">
          <span style="font-size: 0.75rem; text-transform: uppercase; letter-spacing: 0.05em; color: var(--text-muted);" id="ndef-tag-type">Isi NDEF</span>
          <span class="ndef-url" id="ndef-display-payload">-</span>
          <a id="ndef-open-link" href="#" target="_blank" class="btn btn-outline" style="text-decoration:none; padding: 6px 12px; font-size: 0.82rem; width: auto; display: inline-flex;">Buka Link di Browser ↗</a>
        </div>

        <button class="btn btn-primary" style="margin-top: 16px;" onclick="scanTagManual()">🔄 Scan Manual Sekali</button>
        <div id="scan-alert" class="alert"></div>
      </div>

      <div class="tip-box">
        💡 <strong>Mode Auto Scan Aktif:</strong> Dekatkan kartu atau gantungan kunci NFC ke reader RC522. Kartu akan otomatis terdeteksi tanpa perlu menekan tombol, dan jeda debounce mencegah pembacaan berulang-ulang saat kartu sedang menempel.
      </div>
    </div>

    <!-- TAB 2: TULIS TAG -->
    <div id="panel-write" class="panel">
      <div class="card">
        <div class="card-title">Tulis Rekaman NDEF ke Kartu</div>
        
        <div class="form-group">
          <label>Tipe Rekaman</label>
          <select id="write-type" onchange="onTypeChange()">
            <option value="url">🌐 Link Website (URL)</option>
            <option value="text">📝 Teks Biasa</option>
            <option value="tel">📞 Nomor Telepon</option>
            <option value="mail">✉️ Alamat Email</option>
          </select>
        </div>

        <div class="form-group" id="group-prefix">
          <label>Prefix URL</label>
          <select id="write-prefix">
            <option value="https://">https:// (Aman / Rekomendasi)</option>
            <option value="https://www.">https://www.</option>
            <option value="http://">http://</option>
            <option value="http://www.">http://www.</option>
          </select>
        </div>

        <div class="form-group">
          <label id="input-label">Alamat Website</label>
          <input type="text" id="write-val" placeholder="contoh: google.com atau linktr.ee/nama">
        </div>

        <button class="btn btn-primary" onclick="writeTag()">💾 Tulis ke Kartu (Write)</button>
        <div id="write-alert" class="alert"></div>
      </div>

      <div class="tip-box">
        ⚡ <strong>Auto-Format:</strong> Jika kartu MIFARE Classic belum berformat NDEF, sistem akan otomatis memformatnya dengan standar NFC Forum MAD1 sehingga tidak akan muncul "bad sector" lagi!
      </div>
    </div>

    <!-- TAB 3: TOOLS & FORMAT -->
    <div id="panel-tools" class="panel">
      <div class="card">
        <div class="card-title">Format NDEF Tag</div>
        <p style="font-size:0.88rem; color:var(--text-muted); margin-bottom: 14px;">
          Menyiapkan kartu agar sesuai standar NFC Forum. Diperlukan untuk kartu MIFARE Classic agar terbaca sempurna di NFC Tools PC/Mac & Android tanpa error bad sector.
        </p>
        <button class="btn btn-warning" onclick="formatTag()">⚡ Format Kartu ke NDEF</button>
        <div id="format-alert" class="alert"></div>
      </div>

      <div class="card">
        <div class="card-title">Reset ke Setelan Pabrik (Erase)</div>
        <p style="font-size:0.88rem; color:var(--text-muted); margin-bottom: 14px;">
          Menghapus seluruh data NDEF dan mengembalikan kunci sektor MIFARE Classic ke setelan awal pabrik (FFFFFFFFFFFF).
        </p>
        <button class="btn btn-danger" onclick="eraseTag()">🗑️ Hapus & Factory Reset</button>
        <div id="erase-alert" class="alert"></div>
      </div>
    </div>

    <!-- TAB 4: HEX DUMP -->
    <div id="panel-dump" class="panel">
      <div class="card">
        <div class="card-title">Hex Memory Dump</div>
        <button class="btn btn-outline" onclick="loadDump()">📥 Ambil Dump Memori Lengkap</button>
        <div id="dump-container" style="overflow-x: auto; margin-top: 14px;"></div>
      </div>
    </div>

  </div>

  <script>
    let autoScanTimer = null;
    let lastWebScannedUid = "";
    let lastWebScanTime = 0;
    let debounceMs = 3000;

    function openTab(id) {
      document.querySelectorAll('.tab-btn').forEach(b => b.classList.remove('active'));
      document.querySelectorAll('.panel').forEach(p => p.classList.remove('active'));
      event.target.classList.add('active');
      document.getElementById('panel-' + id).classList.add('active');
    }

    function onTypeChange() {
      const ty = document.getElementById('write-type').value;
      const grpPrefix = document.getElementById('group-prefix');
      const lbl = document.getElementById('input-label');
      const inp = document.getElementById('write-val');

      if (ty === 'url') {
        grpPrefix.style.display = 'block';
        lbl.innerText = 'Alamat Website';
        inp.placeholder = 'contoh: instagram.com/profil';
      } else if (ty === 'text') {
        grpPrefix.style.display = 'none';
        lbl.innerText = 'Teks NDEF';
        inp.placeholder = 'Ketik teks di sini...';
      } else if (ty === 'tel') {
        grpPrefix.style.display = 'none';
        lbl.innerText = 'Nomor Telepon';
        inp.placeholder = '+628123456789';
      } else if (ty === 'mail') {
        grpPrefix.style.display = 'none';
        lbl.innerText = 'Alamat Email';
        inp.placeholder = 'user@example.com';
      }
    }

    function showAlert(elemId, type, msg) {
      const el = document.getElementById(elemId);
      el.className = 'alert alert-' + type;
      el.innerHTML = msg;
      el.style.display = 'block';
    }

    function toggleAutoScan() {
      const enabled = document.getElementById('autoscan-toggle').checked;
      if (enabled) {
        startAutoScan();
        showAlert('scan-alert', 'info', '⚡ Auto Scan aktif. Tempelkan kartu ke reader...');
      } else {
        stopAutoScan();
        showAlert('scan-alert', 'info', 'Auto Scan dinonaktifkan.');
      }
      fetch('/api/autoscan', {
        method: 'POST',
        headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
        body: 'enabled=' + (enabled ? '1' : '0')
      });
    }

    function changeDebounce() {
      debounceMs = parseInt(document.getElementById('autoscan-debounce').value) || 3000;
      fetch('/api/autoscan', {
        method: 'POST',
        headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
        body: 'debounce=' + (debounceMs / 1000)
      });
    }

    function startAutoScan() {
      if (autoScanTimer) clearInterval(autoScanTimer);
      autoScanTimer = setInterval(autoScanLoop, 600);
    }

    function stopAutoScan() {
      if (autoScanTimer) {
        clearInterval(autoScanTimer);
        autoScanTimer = null;
      }
    }

    async function autoScanLoop() {
      if (!document.getElementById('autoscan-toggle').checked) return;
      if (!document.getElementById('panel-read').classList.contains('active')) return;

      try {
        const res = await fetch('/api/scan');
        const d = await res.json();
        const now = Date.now();
        if (d.detected) {
          if (d.uid !== lastWebScannedUid || (now - lastWebScanTime >= debounceMs)) {
            lastWebScannedUid = d.uid;
            lastWebScanTime = now;
            renderCardData(d, true);
          }
        } else {
          lastWebScannedUid = "";
        }
      } catch (e) {}
    }

    async function scanTagManual() {
      showAlert('scan-alert', 'info', '⏳ Membaca kartu pada modul RC522...');
      try {
        const res = await fetch('/api/scan');
        const d = await res.json();
        if (!d.detected) {
          showAlert('scan-alert', 'error', '❌ Tag tidak terdeteksi. Dekatkan kartu rata di tengah modul RC522!');
          document.getElementById('card-status').innerText = 'Tidak terdeteksi';
          document.getElementById('card-uid').innerText = '-';
          document.getElementById('card-type').innerText = '-';
          document.getElementById('card-size').innerText = '-';
          document.getElementById('ndef-box').style.display = 'none';
          return;
        }
        renderCardData(d, false);
      } catch (e) {
        showAlert('scan-alert', 'error', '❌ Gagal berkomunikasi dengan ESP32.');
      }
    }

    function renderCardData(d, isAuto) {
      showAlert('scan-alert', 'success', (isAuto ? '⚡ [Auto Scan] ' : '✅ ') + 'Kartu berhasil dibaca!');
      document.getElementById('card-status').innerText = 'Terhubung';
      document.getElementById('card-uid').innerText = d.uid;
      document.getElementById('card-type').innerText = d.type;
      document.getElementById('card-size').innerText = d.size + ' byte';

      if (d.ndef_found) {
        document.getElementById('ndef-box').style.display = 'block';
        document.getElementById('ndef-tag-type').innerText = 'Rekaman: ' + d.ndef_type;
        document.getElementById('ndef-display-payload').innerText = d.ndef_payload;
        const openLink = document.getElementById('ndef-open-link');
        if (d.ndef_type === 'URI') {
          openLink.style.display = 'inline-flex';
          openLink.href = d.ndef_payload;
        } else {
          openLink.style.display = 'none';
        }
      } else {
        document.getElementById('ndef-box').style.display = 'none';
      }
    }

    async function writeTag() {
      const ty = document.getElementById('write-type').value;
      const rawVal = document.getElementById('write-val').value.trim();
      if (!rawVal) {
        showAlert('write-alert', 'error', '⚠️ Harap isi data terlebih dahulu!');
        return;
      }
      let finalVal = rawVal;
      if (ty === 'url') {
        const pfx = document.getElementById('write-prefix').value;
        if (!finalVal.startsWith('http://') && !finalVal.startsWith('https://')) {
          finalVal = pfx + finalVal;
        }
      }
      showAlert('write-alert', 'info', '⏳ Menulis data ke kartu... Jangan geser kartu!');
      try {
        const res = await fetch('/api/write', {
          method: 'POST',
          headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
          body: 'type=' + encodeURIComponent(ty) + '&val=' + encodeURIComponent(finalVal)
        });
        const d = await res.json();
        if (d.success) {
          showAlert('write-alert', 'success', '🎉 <strong>Berhasil!</strong> ' + d.message);
        } else {
          showAlert('write-alert', 'error', '❌ Gagal: ' + d.message);
        }
      } catch (e) {
        showAlert('write-alert', 'error', '❌ Terjadi kesalahan komunikasi.');
      }
    }

    async function formatTag() {
      if (!confirm('Format kartu ini ke standar NFC Forum NDEF?')) return;
      showAlert('format-alert', 'info', '⏳ Sedang memformat kartu...');
      try {
        const res = await fetch('/api/format', { method: 'POST' });
        const d = await res.json();
        showAlert('format-alert', d.success ? 'success' : 'error', d.message);
      } catch (e) {
        showAlert('format-alert', 'error', '❌ Terjadi kesalahan komunikasi.');
      }
    }

    async function eraseTag() {
      if (!confirm('PERINGATAN: Kartu akan direset dan seluruh data dihapus. Lanjutkan?')) return;
      showAlert('erase-alert', 'info', '⏳ Sedang mereset kartu...');
      try {
        const res = await fetch('/api/erase', { method: 'POST' });
        const d = await res.json();
        showAlert('erase-alert', d.success ? 'success' : 'error', d.message);
      } catch (e) {
        showAlert('erase-alert', 'error', '❌ Terjadi kesalahan komunikasi.');
      }
    }

    async function loadDump() {
      const c = document.getElementById('dump-container');
      c.innerHTML = '<p style="color:var(--text-muted); padding:10px;">⏳ Membaca dump memori...</p>';
      try {
        const res = await fetch('/api/dump');
        const d = await res.json();
        if (!d.success) {
          c.innerHTML = '<p style="color:var(--danger); padding:10px;">❌ ' + d.message + '</p>';
          return;
        }
        let html = '<table class="dump-table"><thead><tr><th>No</th><th>Hex Data</th><th>ASCII</th><th>Label</th></tr></thead><tbody>';
        for (let row of d.rows) {
          html += `<tr><td>${row.idx}</td><td>${row.hex}</td><td>${row.ascii}</td><td><small>${row.label || ''}</small></td></tr>`;
        }
        html += '</tbody></table>';
        c.innerHTML = html;
      } catch (e) {
        c.innerHTML = '<p style="color:var(--danger); padding:10px;">❌ Gagal memuat dump memori.</p>';
      }
    }

    window.addEventListener('load', () => {
      startAutoScan();
    });
  </script>
</body>
</html>
)HTML";

#endif
