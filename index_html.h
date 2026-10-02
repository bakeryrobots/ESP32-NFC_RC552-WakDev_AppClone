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
      --bg: #f4f5f7;
      --card-bg: #ffffff;
      --border: #e2e8f0;
      --border-hover: #cbd5e1;
      --text: #0f172a;
      --text-muted: #64748b;
      --text-subtle: #94a3b8;
      --orange: #f6821f;
      --orange-hover: #ea6c00;
      --orange-bg: #fff7ed;
      --orange-border: #fed7aa;
      --success-bg: #ecfdf5;
      --success-border: #a7f3d0;
      --success-text: #065f46;
      --danger-bg: #fef2f2;
      --danger-border: #fecaca;
      --danger-text: #991b1b;
      --warning-bg: #fffbeb;
      --warning-border: #fde68a;
      --warning-text: #92400e;
      --info-bg: #eff6ff;
      --info-border: #bfdbfe;
      --info-text: #1e40af;
      --radius: 8px;
    }
    * { box-sizing: border-box; margin: 0; padding: 0; font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, "Helvetica Neue", Arial, sans-serif; }
    body { background-color: var(--bg); color: var(--text); padding: 24px 16px; display: flex; justify-content: center; }
    .container { width: 100%; max-width: 720px; }
    
    /* Top Header */
    .top-bar { display: flex; justify-content: space-between; align-items: center; margin-bottom: 20px; padding-bottom: 16px; border-bottom: 1px solid var(--border); }
    .brand { display: flex; align-items: center; gap: 10px; }
    .brand-logo { width: 14px; height: 14px; background: var(--orange); border-radius: 3px; display: inline-block; }
    .brand h1 { font-size: 1.25rem; font-weight: 700; color: var(--text); letter-spacing: -0.01em; }
    .brand-sub { font-size: 0.8rem; color: var(--text-muted); margin-left: 2px; }
    .badge-bar { display: flex; gap: 8px; }
    .badge { font-size: 0.72rem; padding: 3px 8px; border-radius: 4px; font-weight: 600; text-transform: uppercase; letter-spacing: 0.04em; }
    .badge-ready { background: var(--success-bg); color: var(--success-text); border: 1px solid var(--success-border); }
    .badge-orange { background: var(--orange-bg); color: var(--orange); border: 1px solid var(--orange-border); }

    /* Navigasi Tab ala Cloudflare */
    .tabs { display: flex; gap: 4px; background: #eaedf1; padding: 4px; border-radius: var(--radius); margin-bottom: 20px; border: 1px solid var(--border); overflow-x: auto; }
    .tab-btn { flex: 1; min-width: 105px; padding: 9px 12px; border: none; background: transparent; color: var(--text-muted); font-weight: 600; font-size: 0.84rem; border-radius: 6px; cursor: pointer; transition: all 0.15s; text-align: center; white-space: nowrap; }
    .tab-btn:hover { color: var(--text); }
    .tab-btn.active { background: #ffffff; color: var(--orange); box-shadow: 0 1px 3px rgba(0,0,0,0.06); }

    .panel { display: none; }
    .panel.active { display: block; }

    /* Card Box */
    .card { background: var(--card-bg); border: 1px solid var(--border); border-radius: var(--radius); padding: 22px; margin-bottom: 18px; box-shadow: 0 1px 3px rgba(0,0,0,0.04); }
    .card-title { font-size: 1.05rem; font-weight: 700; margin-bottom: 14px; color: var(--text); display: flex; justify-content: space-between; align-items: center; }
    .card-desc { font-size: 0.86rem; color: var(--text-muted); margin-bottom: 16px; line-height: 1.5; }

    /* Baris Informasi */
    .info-table { width: 100%; border-collapse: collapse; margin-bottom: 14px; }
    .info-table tr { border-bottom: 1px solid #f1f5f9; }
    .info-table tr:last-child { border-bottom: none; }
    .info-table td { padding: 9px 0; font-size: 0.88rem; }
    .info-label { color: var(--text-muted); width: 40%; }
    .info-val { font-weight: 600; text-align: right; color: var(--text); word-break: break-all; }

    /* Pill status */
    .pill { display: inline-block; padding: 2px 8px; border-radius: 4px; font-size: 0.78rem; font-weight: 600; }
    .pill-green { background: var(--success-bg); color: var(--success-text); border: 1px solid var(--success-border); }
    .pill-amber { background: var(--warning-bg); color: var(--warning-text); border: 1px solid var(--warning-border); }
    .pill-red { background: var(--danger-bg); color: var(--danger-text); border: 1px solid var(--danger-border); }
    .pill-gray { background: #f1f5f9; color: var(--text-muted); border: 1px solid #e2e8f0; }
    .pill-orange { background: var(--orange-bg); color: var(--orange); border: 1px solid var(--orange-border); }

    /* Form & Input */
    .form-group { margin-bottom: 15px; }
    label { display: block; font-size: 0.82rem; font-weight: 600; color: var(--text); margin-bottom: 6px; }
    .label-hint { font-weight: 400; color: var(--text-muted); margin-left: 4px; font-size: 0.78rem; }
    input[type="text"], input[type="password"], select { width: 100%; padding: 10px 12px; background: #ffffff; border: 1px solid var(--border); border-radius: 6px; color: var(--text); font-size: 0.9rem; outline: none; transition: 0.15s; }
    input[type="text"]:focus, input[type="password"]:focus, select:focus { border-color: var(--orange); box-shadow: 0 0 0 2px rgba(246, 130, 31, 0.15); }
    .input-row { display: flex; gap: 10px; }

    /* Buttons */
    .btn { display: inline-flex; align-items: center; justify-content: center; gap: 6px; padding: 10px 16px; border-radius: 6px; font-size: 0.88rem; font-weight: 600; cursor: pointer; transition: all 0.15s; border: none; }
    .btn-block { width: 100%; }
    .btn-primary { background: var(--orange); color: #ffffff; }
    .btn-primary:hover { background: var(--orange-hover); }
    .btn-secondary { background: #ffffff; color: var(--text); border: 1px solid var(--border); }
    .btn-secondary:hover { background: #f8fafc; border-color: var(--border-hover); }
    .btn-danger { background: #ffffff; color: var(--danger-text); border: 1px solid var(--danger-border); }
    .btn-danger:hover { background: var(--danger-bg); border-color: var(--danger-text); }
    .btn-warning { background: #ffffff; color: var(--warning-text); border: 1px solid var(--warning-border); }
    .btn-warning:hover { background: var(--warning-bg); }

    /* Alert Box */
    .alert { padding: 11px 14px; border-radius: 6px; font-size: 0.85rem; margin-top: 14px; display: none; line-height: 1.45; }
    .alert-success { background: var(--success-bg); border: 1px solid var(--success-border); color: var(--success-text); }
    .alert-error { background: var(--danger-bg); border: 1px solid var(--danger-border); color: var(--danger-text); }
    .alert-info { background: var(--info-bg); border: 1px solid var(--info-border); color: var(--info-text); }
    .alert-warning { background: var(--warning-bg); border: 1px solid var(--warning-border); color: var(--warning-text); }

    /* Notice / Guide Box */
    .notice { background: #f8fafc; border-left: 3px solid var(--orange); padding: 12px 14px; font-size: 0.82rem; color: var(--text-muted); border-radius: 0 6px 6px 0; margin-top: 14px; line-height: 1.5; }
    .notice strong { color: var(--text); }

    /* Auto Scan Control */
    .control-bar { display: flex; justify-content: space-between; align-items: center; background: #f8fafc; padding: 10px 14px; border-radius: 6px; margin-bottom: 16px; border: 1px solid var(--border); }
    .control-item { display: flex; align-items: center; gap: 8px; }

    /* Dump Table */
    .dump-wrapper { overflow-x: auto; max-height: 480px; border: 1px solid var(--border); border-radius: 6px; margin-top: 12px; }
    .dump-table { width: 100%; border-collapse: collapse; font-family: ui-monospace, SFMono-Regular, Menlo, Monaco, Consolas, monospace; font-size: 0.78rem; background: #ffffff; }
    .dump-table th, .dump-table td { padding: 6px 10px; border-bottom: 1px solid #f1f5f9; text-align: left; }
    .dump-table th { background: #f8fafc; color: var(--text-muted); font-weight: 600; position: sticky; top: 0; z-index: 1; }
    .dump-table tr:hover { background: #f8fafc; }

    /* NDEF Payload Box */
    .ndef-preview { background: #f8fafc; border: 1px solid var(--border); border-radius: 6px; padding: 14px; margin-top: 12px; }
    .ndef-type { font-size: 0.74rem; text-transform: uppercase; font-weight: 700; color: var(--text-muted); letter-spacing: 0.05em; }
    .ndef-payload { font-size: 0.95rem; font-weight: 600; color: var(--orange); word-break: break-all; margin: 6px 0 10px 0; }
  </style>
</head>
<body>
  <div class="container">
    <!-- Header -->
    <div class="top-bar">
      <div class="brand">
        <span class="brand-logo"></span>
        <h1>NFC Tools</h1>
        <span class="brand-sub">ESP32 & MFRC522</span>
      </div>
      <div class="badge-bar">
        <span class="badge badge-ready">Reader Ready</span>
        <span class="badge badge-orange" id="header-autoscan-badge">Auto-Scan Aktif</span>
      </div>
    </div>

    <!-- Navigasi Tab -->
    <div class="tabs">
      <button class="tab-btn active" onclick="openTab('read')">Baca Tag</button>
      <button class="tab-btn" onclick="openTab('write')">Tulis Tag</button>
      <button class="tab-btn" onclick="openTab('security')">Keamanan</button>
      <button class="tab-btn" onclick="openTab('tools')">Format & Reset</button>
      <button class="tab-btn" onclick="openTab('dump')">Hex Dump</button>
    </div>

    <!-- TAB 1: BACA TAG -->
    <div id="panel-read" class="panel active">
      <div class="card">
        <div class="card-title">
          <span>Informasi Kartu</span>
          <span id="card-presence-pill" class="pill pill-gray">Menunggu Kartu...</span>
        </div>
        
        <!-- Kontrol Auto Scan -->
        <div class="control-bar">
          <div class="control-item">
            <input type="checkbox" id="autoscan-toggle" onchange="toggleAutoScan()" style="width:16px; height:16px; cursor:pointer;" checked>
            <label for="autoscan-toggle" style="margin:0; cursor:pointer; font-size:0.86rem; font-weight:600;">Mode Auto Scan</label>
          </div>
          <div class="control-item">
            <span style="font-size:0.8rem; color:var(--text-muted);">Debounce:</span>
            <select id="autoscan-debounce" onchange="changeDebounce()" style="width:auto; padding:4px 8px; font-size:0.8rem;">
              <option value="2000">2 Detik</option>
              <option value="3000" selected>3 Detik</option>
              <option value="5000">5 Detik</option>
              <option value="10000">10 Detik</option>
            </select>
          </div>
        </div>

        <table class="info-table">
          <tr>
            <td class="info-label">UID / Serial</td>
            <td class="info-val" id="card-uid">-</td>
          </tr>
          <tr>
            <td class="info-label">Tipe Chip</td>
            <td class="info-val" id="card-type">-</td>
          </tr>
          <tr>
            <td class="info-label">Kapasitas Pengguna</td>
            <td class="info-val" id="card-size">-</td>
          </tr>
          <tr>
            <td class="info-label">Status Proteksi</td>
            <td class="info-val" id="card-prot">-</td>
          </tr>
          <tr>
            <td class="info-label">Sesi Autentikasi</td>
            <td class="info-val" id="card-auth-session">-</td>
          </tr>
        </table>

        <div id="ndef-box" style="display:none;" class="ndef-preview">
          <div class="ndef-type" id="ndef-tag-type">Rekaman NDEF</div>
          <div class="ndef-payload" id="ndef-display-payload">-</div>
          <a id="ndef-open-link" href="#" target="_blank" class="btn btn-secondary" style="text-decoration:none; font-size:0.8rem; padding:6px 12px; display:none;">Buka Link di Browser</a>
        </div>

        <button class="btn btn-secondary btn-block" style="margin-top:16px;" onclick="scanTagManual()">Pindai Manual (Scan Sekali)</button>
        <div id="scan-alert" class="alert"></div>
      </div>

      <div class="notice">
        <strong>Auto Scan Aktif:</strong> Tempelkan kartu atau stiker NFC ke atas sensor RC522. Sistem akan membaca kartu secara otomatis tanpa menekan tombol. Jeda debounce mencegah pembacaan berulang-ulang saat kartu tetap menempel.
      </div>
    </div>

    <!-- TAB 2: TULIS TAG -->
    <div id="panel-write" class="panel">
      <div class="card">
        <div class="card-title">Tulis Rekaman NDEF</div>
        <p class="card-desc">Tulis link website (URL) atau teks agar otomatis terbuka saat kartu ditempelkan ke smartphone (iPhone & Android).</p>
        
        <div class="form-group">
          <label>Tipe Data</label>
          <select id="write-type" onchange="onTypeChange()">
            <option value="url">Link Website (URL)</option>
            <option value="text">Teks Biasa</option>
            <option value="tel">Nomor Telepon</option>
            <option value="mail">Alamat Email</option>
          </select>
        </div>

        <div class="form-group" id="group-prefix">
          <label>Protokol URL</label>
          <select id="write-prefix">
            <option value="https://">https:// (Aman / Standar)</option>
            <option value="https://www.">https://www.</option>
            <option value="http://">http://</option>
            <option value="http://www.">http://www.</option>
          </select>
        </div>

        <div class="form-group">
          <label id="input-label">Alamat Website <span class="label-hint">(tanpa https:// jika sudah dipilih)</span></label>
          <input type="text" id="write-val" placeholder="contoh: google.com atau instagram.com/profil">
        </div>

        <button class="btn btn-primary btn-block" onclick="writeTag()">Tulis ke Kartu</button>
        <div id="write-alert" class="alert"></div>
      </div>

      <div class="notice">
        <strong>Proteksi Password:</strong> Jika kartu dilindungi password, proses tulis akan ditolak. Buka tab <strong>Keamanan</strong> untuk menginput password autentikasi terlebih dahulu.
      </div>
    </div>

    <!-- TAB 3: KEAMANAN & PASSWORD -->
    <div id="panel-security" class="panel">
      <!-- Status Keamanan -->
      <div class="card">
        <div class="card-title">
          <span>Status Keamanan Chip</span>
          <button class="btn btn-secondary" style="padding:4px 10px; font-size:0.78rem;" onclick="loadSecurityInfo()">Perbarui Status</button>
        </div>
        <table class="info-table">
          <tr>
            <td class="info-label">Dukungan Password</td>
            <td class="info-val" id="sec-supports">-</td>
          </tr>
          <tr>
            <td class="info-label">Proteksi Password</td>
            <td class="info-val" id="sec-pwd-status">-</td>
          </tr>
          <tr>
            <td class="info-label">Mode Proteksi</td>
            <td class="info-val" id="sec-prot-mode">-</td>
          </tr>
          <tr>
            <td class="info-label">Mulai Halaman (AUTH0)</td>
            <td class="info-val" id="sec-auth0">-</td>
          </tr>
          <tr>
            <td class="info-label">Kunci Permanen (Read-Only)</td>
            <td class="info-val" id="sec-locked">-</td>
          </tr>
          <tr>
            <td class="info-label">Sesi Autentikasi Saat Ini</td>
            <td class="info-val" id="sec-session">-</td>
          </tr>
        </table>
        <div id="security-overview-alert" class="alert"></div>
      </div>

      <!-- Buka Kunci / Autentikasi Sesi -->
      <div class="card">
        <div class="card-title">Autentikasi Sesi (Buka Kunci)</div>
        <p class="card-desc">Jika kartu dilindungi password, masukkan password di bawah ini untuk membuka akses baca dan tulis pada sesi ini.</p>
        <div class="form-group">
          <label>Password Kartu <span class="label-hint">(4 karakter teks atau 8 digit HEX)</span></label>
          <div class="input-row">
            <input type="text" id="auth-pwd" placeholder="contoh: 1234 atau A1B2C3D4">
            <button class="btn btn-primary" style="white-space:nowrap;" onclick="authTag()">Buka Kunci</button>
          </div>
        </div>
        <button class="btn btn-secondary btn-block" onclick="unauthTag()">Kunci Kembali Sesi (Logout)</button>
        <div id="auth-alert" class="alert"></div>
      </div>

      <!-- Pasang Password Baru -->
      <div class="card">
        <div class="card-title">Pasang Password Baru (Set Password)</div>
        <p class="card-desc">Pasang proteksi password pada chip NTAG213/215/216. Setelah dipasang, kartu tidak dapat diubah atau ditimpa tanpa password ini.</p>
        
        <div class="form-group">
          <label>Password Baru <span class="label-hint">(4 karakter teks atau 8 digit HEX)</span></label>
          <input type="text" id="new-pwd" placeholder="contoh: 1234 atau A1B2C3D4">
        </div>

        <div class="form-group">
          <label>Tipe Proteksi</label>
          <select id="new-pwd-mode">
            <option value="w">Hanya Proteksi Tulis (Write-Only) — Direkomendasikan</option>
            <option value="rw">Proteksi Baca & Tulis (Read & Write)</option>
          </select>
          <div style="font-size:0.78rem; color:var(--text-muted); margin-top:4px;">
            Pilihan <strong>Write-Only</strong> memungkinkan kartu tetap dapat dibaca/di-tap oleh semua HP, tetapi data di dalamnya tidak dapat diubah atau dihapus oleh pihak lain.
          </div>
        </div>

        <div class="form-group">
          <label>Mulai Proteksi dari Halaman (AUTH0)</label>
          <input type="text" id="new-pwd-auth0" value="4" placeholder="4">
          <div style="font-size:0.78rem; color:var(--text-muted); margin-top:4px;">
            Nilai 4 melindungi seluruh area memori data pengguna NDEF.
          </div>
        </div>

        <button class="btn btn-primary btn-block" onclick="setPwdTag()">Pasang Password</button>
        <div id="setpwd-alert" class="alert"></div>
      </div>

      <!-- Hapus / Reset Password -->
      <div class="card">
        <div class="card-title">Hapus / Reset Password (Remove Password)</div>
        <p class="card-desc">
          Menghapus proteksi password dan mengembalikan kartu ke kondisi bebas tanpa password.
        </p>
        <div class="notice" style="margin-top:0; margin-bottom:14px;">
          <strong>Syarat Penghapusan:</strong> Kartu harus sudah dibuka kuncinya dengan password lama melalui menu <em>Autentikasi Sesi</em> di atas. Sesuai spesifikasi pabrikan NXP NTAG, kartu yang terkunci password tidak dapat direset tanpa mengetahui password yang benar!
        </div>
        <button class="btn btn-warning btn-block" onclick="removePwdTag()">Hapus Proteksi Password</button>
        <div id="removepwd-alert" class="alert"></div>
      </div>

      <!-- Kunci Permanen (Read-Only) -->
      <div class="card">
        <div class="card-title">Kunci Permanen (Permanent Lock / OTP)</div>
        <p class="card-desc">
          Mengunci kartu menjadi Read-Only secara permanen di tingkat sirkuit terpadu chip (One-Time Programmable). Tindakan ini <strong>tidak dapat dibatalkan</strong> seumur hidup kartu.
        </p>
        <div class="form-group">
          <label>Konfirmasi Penguncian <span class="label-hint">(Ketik teks YES dengan huruf besar)</span></label>
          <div class="input-row">
            <input type="text" id="lock-confirm" placeholder="Ketik YES">
            <button class="btn btn-danger" style="white-space:nowrap;" onclick="lockTag()">Kunci Permanen</button>
          </div>
        </div>
        <div id="lock-alert" class="alert"></div>
      </div>
    </div>

    <!-- TAB 4: FORMAT & RESET -->
    <div id="panel-tools" class="panel">
      <div class="card">
        <div class="card-title">Format Standar NFC Forum NDEF</div>
        <p class="card-desc">
          Menginisialisasi kartu agar siap digunakan untuk URL dan teks standar NFC Forum. Diperlukan untuk kartu MIFARE Classic (MAD1 Sektor 0 + Kunci NFC Forum D3F7D3F7D3F7) agar tidak terdeteksi sebagai bad sector di aplikasi NFC Tools PC/Mac dan HP.
        </p>
        <button class="btn btn-primary btn-block" onclick="formatTag()">Format Kartu ke NDEF</button>
        <div id="format-alert" class="alert"></div>
      </div>

      <div class="card">
        <div class="card-title">Reset ke Setelan Pabrik (Factory Reset)</div>
        <p class="card-desc">
          Menghapus seluruh rekaman data NDEF dan mengembalikan kunci sektor MIFARE Classic ke setelan awal pabrik (FFFFFFFFFFFF).
        </p>
        <button class="btn btn-danger btn-block" onclick="eraseTag()">Reset & Kosongkan Kartu</button>
        <div id="erase-alert" class="alert"></div>
      </div>
    </div>

    <!-- TAB 5: HEX DUMP -->
    <div id="panel-dump" class="panel">
      <div class="card">
        <div class="card-title">
          <span>Hex Memory Dump</span>
          <button class="btn btn-secondary" style="padding:4px 10px; font-size:0.78rem;" onclick="loadDump()">Ambil Dump Memori</button>
        </div>
        <p class="card-desc">Menampilkan seluruh isi blok atau halaman memori fisik kartu dalam format Hexadecimal dan ASCII.</p>
        <div id="dump-container">
          <p style="color:var(--text-muted); font-size:0.85rem; padding:10px 0;">Klik tombol di atas untuk membaca dump memori kartu.</p>
        </div>
      </div>
    </div>
  </div>

  <script>
    let activeTab = 'read';
    let autoScanTimer = null;
    let lastWebScannedUid = "";
    let lastWebScanTime = 0;
    let debounceMs = 3000;

    function openTab(tabId) {
      activeTab = tabId;
      document.querySelectorAll('.tab-btn').forEach(b => b.classList.remove('active'));
      document.querySelectorAll('.panel').forEach(p => p.classList.remove('active'));

      const btnIdx = ['read', 'write', 'security', 'tools', 'dump'].indexOf(tabId);
      if (btnIdx >= 0) document.querySelectorAll('.tab-btn')[btnIdx].classList.add('active');
      const targetPanel = document.getElementById('panel-' + tabId);
      if (targetPanel) targetPanel.classList.add('active');

      if (tabId === 'security') {
        loadSecurityInfo();
      }
    }

    function onTypeChange() {
      const ty = document.getElementById('write-type').value;
      const grpPrefix = document.getElementById('group-prefix');
      const lbl = document.getElementById('input-label');
      const input = document.getElementById('write-val');

      if (ty === 'url') {
        grpPrefix.style.display = 'block';
        lbl.innerText = 'Alamat Website';
        input.placeholder = 'contoh: google.com atau linktr.ee/nama';
      } else if (ty === 'text') {
        grpPrefix.style.display = 'none';
        lbl.innerText = 'Teks Pesan';
        input.placeholder = 'contoh: Selamat Datang di Booth Kami';
      } else if (ty === 'tel') {
        grpPrefix.style.display = 'none';
        lbl.innerText = 'Nomor Telepon';
        input.placeholder = 'contoh: +6281234567890';
      } else if (ty === 'mail') {
        grpPrefix.style.display = 'none';
        lbl.innerText = 'Alamat Email';
        input.placeholder = 'contoh: info@perusahaan.com';
      }
    }

    function showAlert(elemId, type, msg) {
      const el = document.getElementById(elemId);
      if (!el) return;
      el.className = 'alert alert-' + type;
      el.innerHTML = msg;
      el.style.display = 'block';
    }

    function toggleAutoScan() {
      const enabled = document.getElementById('autoscan-toggle').checked;
      document.getElementById('header-autoscan-badge').innerText = enabled ? 'Auto-Scan Aktif' : 'Auto-Scan Nonaktif';
      document.getElementById('header-autoscan-badge').className = 'badge ' + (enabled ? 'badge-orange' : 'badge-ready');
      if (enabled) {
        startAutoScan();
        showAlert('scan-alert', 'info', '[Info] Auto Scan aktif. Tempelkan kartu ke reader.');
      } else {
        stopAutoScan();
        showAlert('scan-alert', 'info', '[Info] Auto Scan dinonaktifkan.');
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
      if (activeTab !== 'read') return;

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
          document.getElementById('card-presence-pill').className = 'pill pill-gray';
          document.getElementById('card-presence-pill').innerText = 'Menunggu Kartu...';
        }
      } catch (e) {}
    }

    async function scanTagManual() {
      showAlert('scan-alert', 'info', '[Info] Membaca kartu pada modul RC522...');
      try {
        const res = await fetch('/api/scan');
        const d = await res.json();
        if (!d.detected) {
          showAlert('scan-alert', 'error', '[Gagal] Tag tidak terdeteksi. Dekatkan kartu rata di tengah modul RC522.');
          document.getElementById('card-presence-pill').className = 'pill pill-gray';
          document.getElementById('card-presence-pill').innerText = 'Tidak Terdeteksi';
          document.getElementById('card-uid').innerText = '-';
          document.getElementById('card-type').innerText = '-';
          document.getElementById('card-size').innerText = '-';
          document.getElementById('card-prot').innerText = '-';
          document.getElementById('card-auth-session').innerText = '-';
          document.getElementById('ndef-box').style.display = 'none';
          return;
        }
        renderCardData(d, false);
      } catch (e) {
        showAlert('scan-alert', 'error', '[Error] Gagal berkomunikasi dengan ESP32.');
      }
    }

    function renderCardData(d, isAuto) {
      showAlert('scan-alert', 'success', (isAuto ? '[Auto Scan] ' : '[Sukses] ') + 'Kartu terdeteksi dan berhasil dibaca.');
      document.getElementById('card-presence-pill').className = 'pill pill-green';
      document.getElementById('card-presence-pill').innerText = 'Terhubung';
      document.getElementById('card-uid').innerText = d.uid;
      document.getElementById('card-type').innerText = d.type;
      document.getElementById('card-size').innerText = d.size + ' byte';

      // Status Proteksi
      let protHtml = '<span class="pill pill-green">Bebas Tanpa Proteksi</span>';
      if (d.is_locked) {
        protHtml = '<span class="pill pill-red">Terkunci Permanen (Read-Only)</span>';
      } else if (d.pwd_active) {
        protHtml = '<span class="pill pill-amber">Proteksi Password (' + (d.prot_mode === 'rw' ? 'Baca & Tulis' : 'Tulis Saja') + ')</span>';
      }
      document.getElementById('card-prot').innerHTML = protHtml;

      // Status Sesi
      let authHtml = '<span class="pill pill-gray">Tidak Diperlukan</span>';
      if (d.pwd_active) {
        if (d.authenticated) {
          authHtml = '<span class="pill pill-green">Terbuka (Terautentikasi)</span>';
        } else {
          authHtml = '<span class="pill pill-amber">Terkunci (Belum Diautentikasi)</span>';
        }
      }
      document.getElementById('card-auth-session').innerHTML = authHtml;

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
        showAlert('write-alert', 'error', '[Peringatan] Harap isi data terlebih dahulu.');
        return;
      }
      let finalVal = rawVal;
      if (ty === 'url') {
        const pfx = document.getElementById('write-prefix').value;
        if (!finalVal.startsWith('http://') && !finalVal.startsWith('https://')) {
          finalVal = pfx + finalVal;
        }
      }
      showAlert('write-alert', 'info', '[Info] Menulis data ke kartu... Jangan geser kartu.');
      try {
        const res = await fetch('/api/write', {
          method: 'POST',
          headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
          body: 'type=' + encodeURIComponent(ty) + '&val=' + encodeURIComponent(finalVal)
        });
        const d = await res.json();
        if (d.success) {
          showAlert('write-alert', 'success', '[Sukses] ' + d.message);
        } else {
          showAlert('write-alert', 'error', '[Gagal] ' + d.message);
        }
      } catch (e) {
        showAlert('write-alert', 'error', '[Error] Terjadi kesalahan komunikasi dengan reader.');
      }
    }

    async function formatTag() {
      if (!confirm('Format kartu ini ke standar NFC Forum NDEF?')) return;
      showAlert('format-alert', 'info', '[Info] Sedang memformat kartu...');
      try {
        const res = await fetch('/api/format', { method: 'POST' });
        const d = await res.json();
        showAlert('format-alert', d.success ? 'success' : 'error', (d.success ? '[Sukses] ' : '[Gagal] ') + d.message);
      } catch (e) {
        showAlert('format-alert', 'error', '[Error] Terjadi kesalahan komunikasi.');
      }
    }

    async function eraseTag() {
      if (!confirm('PERINGATAN: Seluruh data pada kartu akan dihapus dan kunci sektor direset. Lanjutkan?')) return;
      showAlert('erase-alert', 'info', '[Info] Sedang mereset kartu...');
      try {
        const res = await fetch('/api/erase', { method: 'POST' });
        const d = await res.json();
        showAlert('erase-alert', d.success ? 'success' : 'error', (d.success ? '[Sukses] ' : '[Gagal] ') + d.message);
      } catch (e) {
        showAlert('erase-alert', 'error', '[Error] Terjadi kesalahan komunikasi.');
      }
    }

    async function loadDump() {
      const c = document.getElementById('dump-container');
      c.innerHTML = '<p style="color:var(--text-muted); font-size:0.85rem; padding:10px 0;">Membaca dump memori dari chip...</p>';
      try {
        const res = await fetch('/api/dump');
        const d = await res.json();
        if (!d.success) {
          c.innerHTML = '<p style="color:var(--danger-text); font-size:0.85rem; padding:10px 0;">[Gagal] ' + d.message + '</p>';
          return;
        }
        let html = '<div class="dump-wrapper"><table class="dump-table"><thead><tr><th>No</th><th>Hex Data</th><th>ASCII</th><th>Keterangan</th></tr></thead><tbody>';
        for (let row of d.rows) {
          html += `<tr><td>${row.idx}</td><td>${row.hex}</td><td>${row.ascii}</td><td>${row.label || ''}</td></tr>`;
        }
        html += '</tbody></table></div>';
        c.innerHTML = html;
      } catch (e) {
        c.innerHTML = '<p style="color:var(--danger-text); font-size:0.85rem; padding:10px 0;">[Error] Gagal memuat dump memori.</p>';
      }
    }

    /* ================= KEAMANAN & PASSWORD ================= */
    async function loadSecurityInfo() {
      try {
        const res = await fetch('/api/security');
        const d = await res.json();
        if (!d.detected) {
          document.getElementById('sec-supports').innerText = 'Tidak terdeteksi';
          document.getElementById('sec-pwd-status').innerText = '-';
          document.getElementById('sec-prot-mode').innerText = '-';
          document.getElementById('sec-auth0').innerText = '-';
          document.getElementById('sec-locked').innerText = '-';
          document.getElementById('sec-session').innerText = '-';
          showAlert('security-overview-alert', 'warning', '[Perhatian] Tag tidak terdeteksi pada sensor RC522.');
          return;
        }

        document.getElementById('sec-supports').innerHTML = d.supports_pwd 
          ? '<span class="pill pill-green">Didukung (NTAG21x)</span>' 
          : '<span class="pill pill-gray">Tidak Didukung (' + d.type + ')</span>';

        document.getElementById('sec-locked').innerHTML = d.is_locked 
          ? '<span class="pill pill-red">Terkunci Permanen (Read-Only)</span>' 
          : '<span class="pill pill-green">Terbuka (Bisa Ditulis)</span>';

        if (d.supports_pwd) {
          document.getElementById('sec-pwd-status').innerHTML = d.pwd_active 
            ? '<span class="pill pill-amber">Password Aktif</span>' 
            : '<span class="pill pill-green">Tidak Ada Password</span>';

          document.getElementById('sec-prot-mode').innerText = d.pwd_active 
            ? (d.prot_mode === 'rw' ? 'Baca & Tulis (Read & Write)' : 'Hanya Tulis (Write-Only)')
            : '-';

          document.getElementById('sec-auth0').innerText = d.pwd_active ? ('Page ' + d.auth0) : '-';

          document.getElementById('sec-session').innerHTML = d.authenticated 
            ? '<span class="pill pill-green">Terautentikasi (Terbuka)</span>' 
            : (d.pwd_active ? '<span class="pill pill-amber">Belum Diautentikasi (Terkunci)</span>' : '<span class="pill pill-gray">Bebas</span>');
        } else {
          document.getElementById('sec-pwd-status').innerText = 'Tidak Tersedia';
          document.getElementById('sec-prot-mode').innerText = '-';
          document.getElementById('sec-auth0').innerText = '-';
          document.getElementById('sec-session').innerText = '-';
        }

        document.getElementById('security-overview-alert').style.display = 'none';
      } catch (e) {
        showAlert('security-overview-alert', 'error', '[Error] Gagal membaca status keamanan.');
      }
    }

    async function authTag() {
      const pwd = document.getElementById('auth-pwd').value.trim();
      if (!pwd) {
        showAlert('auth-alert', 'error', '[Peringatan] Masukkan password terlebih dahulu.');
        return;
      }
      showAlert('auth-alert', 'info', '[Info] Memverifikasi password ke chip tag...');
      try {
        const res = await fetch('/api/auth', {
          method: 'POST',
          headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
          body: 'pwd=' + encodeURIComponent(pwd)
        });
        const d = await res.json();
        showAlert('auth-alert', d.success ? 'success' : 'error', (d.success ? '[Sukses] ' : '[Gagal] ') + d.message);
        loadSecurityInfo();
      } catch (e) {
        showAlert('auth-alert', 'error', '[Error] Terjadi kesalahan komunikasi.');
      }
    }

    async function unauthTag() {
      try {
        const res = await fetch('/api/unauth', { method: 'POST' });
        const d = await res.json();
        showAlert('auth-alert', 'info', '[Info] Sesi autentikasi telah dinonaktifkan.');
        loadSecurityInfo();
      } catch (e) {
        showAlert('auth-alert', 'error', '[Error] Terjadi kesalahan komunikasi.');
      }
    }

    async function setPwdTag() {
      const pwd = document.getElementById('new-pwd').value.trim();
      if (!pwd) {
        showAlert('setpwd-alert', 'error', '[Peringatan] Harap tentukan password baru.');
        return;
      }
      const mode = document.getElementById('new-pwd-mode').value;
      const auth0 = document.getElementById('new-pwd-auth0').value.trim() || '4';

      showAlert('setpwd-alert', 'info', '[Info] Menulis konfigurasi password ke chip...');
      try {
        const res = await fetch('/api/setpwd', {
          method: 'POST',
          headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
          body: 'pwd=' + encodeURIComponent(pwd) + '&mode=' + encodeURIComponent(mode) + '&auth0=' + encodeURIComponent(auth0)
        });
        const d = await res.json();
        showAlert('setpwd-alert', d.success ? 'success' : 'error', (d.success ? '[Sukses] ' : '[Gagal] ') + d.message);
        loadSecurityInfo();
      } catch (e) {
        showAlert('setpwd-alert', 'error', '[Error] Terjadi kesalahan komunikasi.');
      }
    }

    async function removePwdTag() {
      if (!confirm('Hapus seluruh proteksi password dan kembalikan kartu ke kondisi bebas?')) return;
      showAlert('removepwd-alert', 'info', '[Info] Mengirim perintah penghapusan password...');
      try {
        const res = await fetch('/api/removepwd', { method: 'POST' });
        const d = await res.json();
        showAlert('removepwd-alert', d.success ? 'success' : 'error', (d.success ? '[Sukses] ' : '[Gagal] ') + d.message);
        loadSecurityInfo();
      } catch (e) {
        showAlert('removepwd-alert', 'error', '[Error] Terjadi kesalahan komunikasi.');
      }
    }

    async function lockTag() {
      const confirmVal = document.getElementById('lock-confirm').value.trim();
      if (confirmVal !== 'YES') {
        showAlert('lock-alert', 'error', '[Peringatan] Anda harus mengetik kata YES untuk mengonfirmasi.');
        return;
      }
      if (!confirm('PERINGATAN TERAKHIR: Penguncian ini PERMANEN dan tidak dapat dibatalkan selamanya. Lanjutkan?')) return;
      showAlert('lock-alert', 'info', '[Info] Mengunci bit OTP kartu...');
      try {
        const res = await fetch('/api/lock', {
          method: 'POST',
          headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
          body: 'confirm=YES'
        });
        const d = await res.json();
        showAlert('lock-alert', d.success ? 'success' : 'error', (d.success ? '[Sukses] ' : '[Gagal] ') + d.message);
        loadSecurityInfo();
      } catch (e) {
        showAlert('lock-alert', 'error', '[Error] Terjadi kesalahan komunikasi.');
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
