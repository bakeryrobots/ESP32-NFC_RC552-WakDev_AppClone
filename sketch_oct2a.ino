/*
  =============================================================================
  NFC Tools ESP32 + MFRC522 (Versi Lengkap: Web Dashboard + Serial Monitor)
  Fungsi sama persis seperti "NFC Tools" (Wakdev) untuk membaca & menulis kartu:
    - NTAG213 / NTAG215 / NTAG216 / Ultralight (NFC Forum Type 2)
    - MIFARE Classic 1K / 4K (NFC Forum Type MIFARE Classic dengan MAD1)
  =============================================================================
  
  FITUR UTAMA:
  1. Menulis Link Website (URL), Teks, Telepon, Email, dll.
  2. Buka Link Otomatis di HP:
     * NTAG213/215/216 : Otomatis buka web di SEMUA HP (iPhone & Android)
     * MIFARE Classic  : Buka di NFC Tools / Android yang mendukung NXP
  3. Format NDEF Otomatis:
     * MIFARE Classic  : Menulis MAD Sektor 0 + Kunci NFC Forum (D3F7D3F7D3F7)
     * NTAG            : Menginisialisasi Capability Container (CC di Page 3)
  4. Web Dashboard GUI (Mirip NFC Tools PC/Mac):
     * Hubungkan laptop/HP ke WiFi Access Point: "ESP32-NFC-Tools"
     * Buka browser ke: http://192.168.4.1
  5. Serial Monitor CLI (115200 baud):
     * Ketik 'help' untuk daftar perintah (wurl, scan, dump, format, erase, dll.)

  Wiring (MFRC522 -> ESP32):
    3.3V -> 3V3 (PERINGATAN: JANGAN KE 5V!)
    RST  -> GPIO 22
    GND  -> GND
    MISO -> GPIO 19
    MOSI -> GPIO 23
    SCK  -> GPIO 18
    SDA  -> GPIO 5
*/

#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <WebServer.h>
#include "index_html.h"

#define RST_PIN  22
#define SS_PIN   5
#define SCK_PIN  18
#define MISO_PIN 19
#define MOSI_PIN 23

// Konfigurasi WiFi AP untuk Dashboard Web
#define AP_SSID "ESP32-NFC-Tools"
#define AP_PASS "12345678" // Ganti atau buat "" jika tanpa password

MFRC522 mfrc522(SS_PIN, RST_PIN);
WebServer server(80);

// ====================== TIPE DATA & STRUKTUR ======================
enum Kind : byte { K_NONE, K_CLASSIC, K_UL, K_OTHER };

struct TagInfo {
  Kind kind;
  const char* name;
  uint16_t pages, userEnd, cfg, dynLock;  // NTAG / Ultralight
  byte cc;                                // byte ke-3 Capability Container
  uint16_t blocks;                        // Classic
  byte sectors;
};

struct NtagDef { 
  byte type, size; 
  const char* name; 
  uint16_t pages, userEnd, cfg, dynLock; 
  byte cc; 
};

const NtagDef NTAGS[] = {
  {0x04, 0x0F, "NTAG213", 45, 39, 41, 40, 0x12},
  {0x04, 0x11, "NTAG215", 135, 129, 131, 130, 0x3E},
  {0x04, 0x13, "NTAG216", 231, 225, 227, 226, 0x6D},
  {0x03, 0x0B, "MIFARE Ultralight EV1 (MF0UL11)", 20, 15, 16, 0, 0x06},
  {0x03, 0x0E, "MIFARE Ultralight EV1 (MF0UL21)", 41, 35, 37, 36, 0x10},
};

const char* URI_PREFIX[] = {
  "", "http://www.", "https://www.", "http://", "https://", "tel:", "mailto:",
  "ftp://anonymous:anonymous@", "ftp://ftp.", "ftps://", "sftp://", "smb://", "nfs://",
  "ftp://", "dav://", "news:", "telnet://", "imap:", "rtsp://", "urn:", "pop:", "sip:",
  "sips:", "tftp:", "btspp://", "btl2cap://", "btgoep://", "tcpobex://", "irdaobex://",
  "file://", "urn:epc:id:", "urn:epc:tag:", "urn:epc:pat:", "urn:epc:raw:", "urn:epc:", "urn:nfc:"
};
const int NPREFIX = sizeof(URI_PREFIX) / sizeof(URI_PREFIX[0]);

// Kunci umum MIFARE Classic (termasuk standar NDEF NFC Forum)
const byte KEYS[][6] = {
  {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}, // Default Pabrik
  {0xD3,0xF7,0xD3,0xF7,0xD3,0xF7}, // Standar NFC Forum NDEF
  {0xA0,0xA1,0xA2,0xA3,0xA4,0xA5}, // Standar MAD Sektor 0
  {0x00,0x00,0x00,0x00,0x00,0x00}, 
  {0xB0,0xB1,0xB2,0xB3,0xB4,0xB5}, 
  {0x4D,0x3A,0x99,0xC3,0x51,0xDD},
  {0x1A,0x98,0x2C,0x7E,0x45,0x9A}, 
  {0xAA,0xBB,0xCC,0xDD,0xEE,0xFF}, 
  {0x71,0x4C,0x5C,0x88,0x6E,0x97},
  {0x58,0x7E,0xE5,0xF9,0x35,0x0F}, 
  {0xA0,0x47,0x8C,0xC3,0x90,0x91}, 
  {0x53,0x3C,0xB6,0xC7,0x23,0xF6},
  {0x8F,0xD0,0xA4,0xF2,0x56,0xE9}
};
const int NKEYS = sizeof(KEYS) / 6;

// Variabel Global
TagInfo tag;
byte sessPwd[4], lastPack[2];
bool sessPwdSet = false, authed = false;
byte sessKey[6];
bool sessKeySet = false;
String ndefLang = "en";
byte mem[900], msg[960], pbuf[900], clip[900];
uint16_t clipLen = 0;
String inLine;
byte curGain = 0x40; // 33 dB

struct NdefRecordInfo {
  bool found;
  String type;      // "URI", "Text", "MIME", "Raw"
  String payload;   // "https://google.com", dll.
};
NdefRecordInfo lastNdef;

// Variabel Mode Auto Scan & Debounce
bool autoScanEnabled = true;               // Default: aktif
unsigned long autoScanDebounceMs = 3000;   // Debounce: 3 detik (3000 ms)
unsigned long lastScanTime = 0;
String lastScannedUid = "";
bool cardCurrentlyPresent = false;

const char* TAGCMDS = " scan dump read write wtext wurl wuri wtel wmail wmime erase format lock setpwd removepwd copy paste setkey resetkey autoscan ";

// Forward declaration fungsi Classic & NDEF
bool selectTag(unsigned long timeoutMs = 80);
void endTag();
void showNdef();
bool classicAuth(int block);
int sectorFirst(int s);
int sectorBlocks(int s);
bool isTrailer(int b);
bool formatClassicNdef();
bool isClassicNdefFormatted();
bool writeClassicBytes(const byte* d, size_t len);
bool readClassicBytes(byte* out, size_t maxLen, size_t& bytesRead);
bool factoryResetClassic();

// ====================== UTILITAS ======================
void printHex(const byte* d, size_t n) {
  for (size_t i = 0; i < n; i++) {
    if (d[i] < 0x10) Serial.print('0');
    Serial.print(d[i], HEX);
    if (i + 1 < n) Serial.print(' ');
  }
}

String getUidString() {
  String s = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) s += "0";
    s += String(mfrc522.uid.uidByte[i], HEX);
    if (i + 1 < mfrc522.uid.size) s += ":";
  }
  s.toUpperCase();
  return s;
}

void printAscii(const byte* d, size_t n) {
  for (size_t i = 0; i < n; i++) Serial.print((d[i] >= 32 && d[i] < 127) ? (char)d[i] : '.');
}

int hv(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  c |= 0x20;
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  return -1;
}

bool parseHex(const String& s, byte* out, size_t n) {
  if (s.length() != n * 2) return false;
  for (size_t i = 0; i < n; i++) {
    int a = hv(s[2 * i]), b = hv(s[2 * i + 1]);
    if (a < 0 || b < 0) return false;
    out[i] = (a << 4) | b;
  }
  return true;
}

bool parsePwd(const String& s, byte* out) {
  if (s.length() == 4) { for (int i = 0; i < 4; i++) out[i] = s[i]; return true; }
  return parseHex(s, out, 4);
}

String nextTok(String& s) {
  s.trim();
  int i = s.indexOf(' ');
  String t;
  if (i < 0) { t = s; s = ""; }
  else { t = s.substring(0, i); s = s.substring(i + 1); s.trim(); }
  return t;
}

// ====================== LAYER TAG & HARDWARE ======================
bool selectTag(unsigned long timeoutMs) {
  unsigned long t0 = millis();
  do {
    byte atqa[2]; byte len = sizeof(atqa);
    if (mfrc522.PICC_WakeupA(atqa, &len) == MFRC522::STATUS_OK &&
        mfrc522.PICC_Select(&mfrc522.uid, 0) == MFRC522::STATUS_OK) return true;
    delay(15);
  } while (millis() - t0 < timeoutMs);
  
  // Power-cycle RF antena jika belum respons
  mfrc522.PCD_AntennaOff(); delay(10); mfrc522.PCD_AntennaOn(); delay(10);
  byte atqa[2]; byte len = sizeof(atqa);
  return mfrc522.PICC_WakeupA(atqa, &len) == MFRC522::STATUS_OK &&
         mfrc522.PICC_Select(&mfrc522.uid, 0) == MFRC522::STATUS_OK;
}

void endTag() { 
  mfrc522.PICC_HaltA(); 
  mfrc522.PCD_StopCrypto1(); 
}

bool getVersion(byte* v) {
  byte cmd[3] = {0x60, 0, 0};
  mfrc522.PCD_CalculateCRC(cmd, 1, &cmd[1]);
  byte resp[18]; byte rl = sizeof(resp);
  if (mfrc522.PCD_TransceiveData(cmd, 3, resp, &rl) != MFRC522::STATUS_OK || rl < 8) return false;
  memcpy(v, resp, 8);
  return true;
}

bool ntagAuth(const byte* pwd, byte* pack) {
  byte cmd[7] = {0x1B, pwd[0], pwd[1], pwd[2], pwd[3], 0, 0};
  mfrc522.PCD_CalculateCRC(cmd, 5, &cmd[5]);
  byte resp[18]; byte rl = sizeof(resp);
  if (mfrc522.PCD_TransceiveData(cmd, 7, resp, &rl) != MFRC522::STATUS_OK || rl < 2) return false;
  pack[0] = resp[0]; pack[1] = resp[1];
  return true;
}

bool rdPage(uint16_t p, byte* o4) {
  byte b[18]; byte n = sizeof(b);
  if (mfrc522.MIFARE_Read(p, b, &n) != MFRC522::STATUS_OK) return false;
  memcpy(o4, b, 4);
  return true;
}

bool wrPage(uint16_t p, const byte* d4) {
  byte t[4]; memcpy(t, d4, 4);
  return mfrc522.MIFARE_Ultralight_Write(p, t, 4) == MFRC522::STATUS_OK;
}

void authSession() {
  if (!sessPwdSet || tag.kind != K_UL || !tag.cfg) return;
  if (ntagAuth(sessPwd, lastPack)) { authed = true; return; }
  Serial.println(F("[!] Auth gagal (password sesi salah?)"));
  selectTag(80);
}

void reopen() { if (selectTag(80)) authSession(); }

void detectUL() {
  tag.kind = K_UL;
  byte v[8];
  bool ok = getVersion(v);
  if (!ok) selectTag(80);
  if (ok) {
    for (const NtagDef& n : NTAGS) {
      if (v[2] == n.type && v[6] == n.size) {
        tag.name = n.name; tag.pages = n.pages; tag.userEnd = n.userEnd;
        tag.cfg = n.cfg; tag.dynLock = n.dynLock; tag.cc = n.cc;
        return;
      }
    }
  }
  byte b[18]; byte sz = sizeof(b);
  if (mfrc522.MIFARE_Read(3, b, &sz) == MFRC522::STATUS_OK && b[2] == 0x12) {
    tag.name = "NTAG203"; tag.pages = 42; tag.userEnd = 39; tag.cc = 0x12;
  } else {
    tag.name = "MIFARE Ultralight"; tag.pages = 16; tag.userEnd = 15; tag.cc = 0x06;
  }
}

void detect() {
  memset(&tag, 0, sizeof(tag));
  switch (mfrc522.PICC_GetType(mfrc522.uid.sak)) {
    case MFRC522::PICC_TYPE_MIFARE_MINI: 
      tag.kind = K_CLASSIC; tag.name = "MIFARE Classic Mini"; tag.sectors = 5;  tag.blocks = 20;  break;
    case MFRC522::PICC_TYPE_MIFARE_1K:   
      tag.kind = K_CLASSIC; tag.name = "MIFARE Classic 1K";   tag.sectors = 16; tag.blocks = 64;  break;
    case MFRC522::PICC_TYPE_MIFARE_4K:   
      tag.kind = K_CLASSIC; tag.name = "MIFARE Classic 4K";   tag.sectors = 40; tag.blocks = 256; break;
    case MFRC522::PICC_TYPE_MIFARE_UL:   
      detectUL(); break;
    default: 
      tag.kind = K_OTHER; tag.name = "Tag Lain / Tidak Dikenal";
  }
}

bool prepare() {
  authed = false;
  if (!selectTag(1500)) { return false; }
  detect();
  authSession();
  return true;
}

uint16_t userBytes() { 
  if (tag.kind == K_UL) return (tag.userEnd - 3) * 4;
  if (tag.kind == K_CLASSIC) return (tag.sectors - 1) * 48; // Sektor 1..N (3 block data * 16 byte = 48 byte/sektor)
  return 0;
}

// ----- Memori User NTAG / Ultralight -----
bool readUser(byte* out) {
  uint16_t n = tag.userEnd - 3;
  for (uint16_t i = 0; i < n; i += 4) {
    uint16_t p = 4 + i;
    if (p + 3 > tag.userEnd) p = tag.userEnd - 3;
    byte b[18]; byte sz = sizeof(b);
    if (mfrc522.MIFARE_Read(p, b, &sz) != MFRC522::STATUS_OK) return false;
    memcpy(out + (p - 4) * 4, b, 16);
  }
  return true;
}

bool writeUser(const byte* d, size_t len) {
  for (size_t off = 0; off < len; off += 4) {
    byte c[4] = {0, 0, 0, 0};
    size_t n = (len - off) < 4 ? (len - off) : 4;
    memcpy(c, d + off, n);
    uint16_t p = 4 + off / 4;
    if (!wrPage(p, c)) { 
      Serial.printf("[!] Gagal menulis page %u\n", p); 
      return false; 
    }
  }
  return true;
}

// ====================== NDEF ENGINE ======================
bool findNdef(const byte* b, size_t total, size_t& st, size_t& len, size_t& end) {
  size_t i = 0;
  while (i < total) {
    byte t = b[i];
    if (t == 0x00) { i++; continue; }
    if (t == 0xFE || i + 1 >= total) break;
    size_t l, s;
    if (b[i + 1] == 0xFF) { 
      if (i + 3 >= total) break; 
      l = ((size_t)b[i + 2] << 8) | b[i + 3]; 
      s = i + 4; 
    } else { 
      l = b[i + 1]; 
      s = i + 2; 
    }
    if (s + l > total) break;
    if (t == 0x03) { 
      st = s; len = l; 
      end = (s + l + 1 < total) ? s + l + 1 : total; 
      return true; 
    }
    i = s + l;
  }
  return false;
}

#define BAD { Serial.println(F("(rekaman NDEF rusak)")); lastNdef.found = false; return; }

void decodeNdef(const byte* m, size_t len) {
  lastNdef.found = false;
  lastNdef.type = "";
  lastNdef.payload = "";
  if (len == 0) { 
    Serial.println(F("NDEF   : kosong")); 
    return; 
  }
  size_t i = 0; int n = 0;
  while (i < len) {
    byte h = m[i++];
    bool sr = h & 0x10, il = h & 0x08;
    byte tnf = h & 0x07;
    if (i >= len) BAD
    size_t tl = m[i++], pl = 0;
    if (sr) { if (i >= len) BAD pl = m[i++]; }
    else {
      if (i + 4 > len) BAD
      pl = ((size_t)m[i] << 24) | ((size_t)m[i + 1] << 16) | ((size_t)m[i + 2] << 8) | m[i + 3]; i += 4;
    }
    size_t idl = 0;
    if (il) { if (i >= len) BAD idl = m[i++]; }
    if (i + tl + idl + pl > len) BAD
    const byte* ty = m + i;
    const byte* pay = m + i + tl + idl;
    
    Serial.printf("Rekam #%d: ", ++n);
    lastNdef.found = true;

    if (tnf == 1 && tl == 1 && ty[0] == 'T' && pl >= 1) {
      size_t ll = pay[0] & 0x3F;
      if (ll > pl - 1) ll = pl - 1;
      Serial.print(F("Teks ["));
      Serial.write(pay + 1, ll);
      Serial.print(F("] "));
      Serial.write(pay + 1 + ll, pl - 1 - ll);
      lastNdef.type = "Text";
      String t = "";
      for (size_t k = 0; k < pl - 1 - ll; k++) t += (char)pay[1 + ll + k];
      lastNdef.payload = t;
    } else if (tnf == 1 && tl == 1 && ty[0] == 'U' && pl >= 1) {
      Serial.print(F("URI  "));
      const char* pfx = (pay[0] < NPREFIX ? URI_PREFIX[pay[0]] : "");
      Serial.print(pfx);
      Serial.write(pay + 1, pl - 1);
      lastNdef.type = "URI";
      String u = String(pfx);
      for (size_t k = 0; k < pl - 1; k++) u += (char)pay[1 + k];
      lastNdef.payload = u;
    } else if (tnf == 2) {
      Serial.print(F("MIME "));
      Serial.write(ty, tl);
      Serial.print(F(": "));
      Serial.write(pay, pl);
      lastNdef.type = "MIME";
      String mStr = "";
      for (size_t k = 0; k < pl; k++) mStr += (char)pay[k];
      lastNdef.payload = mStr;
    } else {
      Serial.printf("TNF=%u tipe=", tnf);
      Serial.write(ty, tl);
      Serial.print(F(" data="));
      printHex(pay, pl < 32 ? pl : 32);
      lastNdef.type = "Raw";
      lastNdef.payload = "Data Biner";
    }
    Serial.println();
    i += tl + idl + pl;
    if (h & 0x40) break;
  }
}

void showNdef() {
  size_t bytesTotal = 0;
  if (tag.kind == K_UL) {
    if (!readUser(mem)) {
      Serial.println(F("NDEF   : tidak bisa dibaca (terproteksi? coba 'auth <pwd>')"));
      reopen();
      return;
    }
    bytesTotal = userBytes();
  } else if (tag.kind == K_CLASSIC) {
    size_t bytesRead = 0;
    if (!readClassicBytes(mem, sizeof(mem), bytesRead)) {
      Serial.println(F("NDEF   : tidak ada / belum diformat NDEF"));
      lastNdef.found = false;
      return;
    }
    bytesTotal = bytesRead;
  } else {
    Serial.println(F("NDEF   : jenis kartu tidak didukung"));
    lastNdef.found = false;
    return;
  }

  size_t st, len, end;
  if (findNdef(mem, bytesTotal, st, len, end)) {
    decodeNdef(mem + st, len);
  } else {
    Serial.println(F("NDEF   : kosong (belum ada rekaman)"));
    lastNdef.found = false;
  }
}

size_t buildNdef(byte tnf, const byte* type, byte tl, const byte* pay, size_t pl, byte* out) {
  bool sr = pl < 256;
  size_t rec = 2 + (sr ? 1 : 4) + tl + pl, o = 0;
  out[o++] = 0x03; // NDEF TLV Tag
  if (rec < 0xFF) out[o++] = rec;
  else { out[o++] = 0xFF; out[o++] = rec >> 8; out[o++] = rec & 0xFF; }
  out[o++] = 0xC0 | (sr ? 0x10 : 0) | tnf; // Record Header: MB=1, ME=1
  out[o++] = tl;
  if (sr) out[o++] = pl;
  else { out[o++] = 0; out[o++] = 0; out[o++] = pl >> 8; out[o++] = pl & 0xFF; }
  memcpy(out + o, type, tl); o += tl;
  memcpy(out + o, pay, pl);  o += pl;
  out[o++] = 0xFE; // Terminator TLV
  return o;
}

bool doWriteNdef(byte tnf, const byte* type, byte tl, const byte* pay, size_t pl) {
  if (pl + tl + 16 > sizeof(msg)) { 
    Serial.println(F("[!] Data terlalu panjang untuk buffer")); 
    return false; 
  }
  size_t n = buildNdef(tnf, type, tl, pay, pl, msg);

  // 1. JIKA KARTU NTAG / ULTRALIGHT
  if (tag.kind == K_UL) {
    // Pastikan Page 3 (Capability Container - CC) terinisialisasi
    byte cc[4];
    if (!rdPage(3, cc) || cc[0] != 0xE1) {
      byte ncc[4] = {0xE1, 0x10, tag.cc ? tag.cc : 0x12, 0x00};
      wrPage(3, ncc);
    }
    if (n > userBytes()) { 
      Serial.printf("[!] Data %u byte > kapasitas NTAG %u byte\n", (unsigned)n, userBytes()); 
      return false; 
    }
    if (writeUser(msg, n)) { 
      Serial.printf("OK: %u byte berhasil ditulis ke NTAG.\n", (unsigned)n); 
      showNdef(); 
      return true;
    }
  } 
  // 2. JIKA KARTU MIFARE CLASSIC 1K / 4K
  else if (tag.kind == K_CLASSIC) {
    // Cek apakah kartu sudah diformat NDEF standar NFC Forum. Jika belum, format otomatis!
    if (!isClassicNdefFormatted()) {
      Serial.println(F("[*] Kartu MIFARE Classic belum berformat NDEF. Memformat otomatis..."));
      if (!formatClassicNdef()) {
        Serial.println(F("[!] Gagal format NDEF otomatis pada MIFARE Classic."));
        return false;
      }
    }
    if (n > userBytes()) {
      Serial.printf("[!] Data %u byte > kapasitas %u byte\n", (unsigned)n, userBytes());
      return false;
    }
    if (writeClassicBytes(msg, n)) {
      Serial.printf("OK: %u byte berhasil ditulis ke MIFARE Classic.\n", (unsigned)n);
      showNdef();
      return true;
    }
  } else {
    Serial.println(F("[!] Tipe kartu ini tidak didukung untuk penulisan NDEF."));
    return false;
  }
  return false;
}

void cmdWText(const String& t) {
  if (t.length() > 700) { Serial.println(F("[!] Teks terlalu panjang")); return; }
  size_t ll = ndefLang.length();
  pbuf[0] = ll;
  memcpy(pbuf + 1, ndefLang.c_str(), ll);
  memcpy(pbuf + 1 + ll, t.c_str(), t.length());
  doWriteNdef(0x01, (const byte*)"T", 1, pbuf, 1 + ll + t.length());
}

void cmdWUri(const String& uri) {
  if (uri.length() > 700) { Serial.println(F("[!] URI terlalu panjang")); return; }
  int code = 0; size_t best = 0;
  for (int i = 1; i < NPREFIX; i++) {
    size_t l = strlen(URI_PREFIX[i]);
    if (l > best && uri.startsWith(URI_PREFIX[i])) { best = l; code = i; }
  }
  size_t rest = uri.length() - best;
  pbuf[0] = code;
  memcpy(pbuf + 1, uri.c_str() + best, rest);
  doWriteNdef(0x01, (const byte*)"U", 1, pbuf, rest + 1);
}

void cmdWMime(String a) {
  String ty = nextTok(a);
  if (ty.indexOf('/') < 0 || ty.length() > 100 || !a.length() || a.length() > 700) {
    Serial.println(F("Pakai: wmime <tipe/mime> <data>   contoh: wmime text/plain halo")); return;
  }
  doWriteNdef(0x02, (const byte*)ty.c_str(), ty.length(), (const byte*)a.c_str(), a.length());
}

// ====================== MIFARE CLASSIC NDEF & MAD ======================
int sectorFirst(int s)  { return s < 32 ? s * 4 : 128 + (s - 32) * 16; }
int sectorBlocks(int s) { return s < 32 ? 4 : 16; }
bool isTrailer(int b)   { return b < 128 ? (b % 4 == 3) : (b % 16 == 15); }

bool classicAuth(int block) {
  MFRC522::MIFARE_Key k;
  for (int i = -1; i < NKEYS; i++) {
    if (i < 0 && !sessKeySet) continue;
    memcpy(k.keyByte, i < 0 ? sessKey : KEYS[i], 6);
    for (int ab = 0; ab < 2; ab++) {
      byte cmd = ab ? MFRC522::PICC_CMD_MF_AUTH_KEY_B : MFRC522::PICC_CMD_MF_AUTH_KEY_A;
      if (mfrc522.PCD_Authenticate(cmd, block, &k, &mfrc522.uid) == MFRC522::STATUS_OK) return true;
      selectTag(40);
    }
  }
  return false;
}

bool isClassicNdefFormatted() {
  if (tag.kind != K_CLASSIC) return false;
  MFRC522::MIFARE_Key k;
  byte ndefKey[6] = {0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7};
  memcpy(k.keyByte, ndefKey, 6);
  // Otentikasi block 4 (Sektor 1) dengan Key A standar NDEF
  if (mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 4, &k, &mfrc522.uid) == MFRC522::STATUS_OK) {
    return true;
  }
  selectTag(40);
  return false;
}

bool formatClassicNdef() {
  if (tag.kind != K_CLASSIC) return false;
  Serial.println(F("[*] Memformat MIFARE Classic ke standar NFC Forum NDEF..."));

  // 1. Format Sektor 0 (MAD1)
  if (!classicAuth(0)) {
    Serial.println(F("[!] Gagal otentikasi Sektor 0. Pastikan key benar."));
    return false;
  }

  // Block 1: MAD1 Sektor 1..7 (AID = 0x03, 0xE1)
  byte mad1[16] = {0x14, 0x01, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1};
  // Block 2: MAD1 Sektor 8..15 (AID = 0x03, 0xE1)
  byte mad2[16] = {0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1};

  if (mfrc522.MIFARE_Write(1, mad1, 16) != MFRC522::STATUS_OK ||
      mfrc522.MIFARE_Write(2, mad2, 16) != MFRC522::STATUS_OK) {
    Serial.println(F("[!] Gagal menulis MAD di Sektor 0."));
    return false;
  }

  // Block 3: Trailer Sektor 0 (Key A: A0A1A2A3A4A5, Access: 78 77 88 C1, Key B: D3F7D3F7D3F7)
  byte s0Trailer[16] = {0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0x78, 0x77, 0x88, 0xC1, 0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7};
  if (mfrc522.MIFARE_Write(3, s0Trailer, 16) != MFRC522::STATUS_OK) {
    Serial.println(F("[!] Gagal menulis trailer Sektor 0."));
    return false;
  }

  // 2. Format Sektor 1 s.d. selesai (Key A: D3F7D3F7D3F7, Access: 7F 07 88 40, Key B: D3F7D3F7D3F7)
  byte ndefTrailer[16] = {0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7, 0x7F, 0x07, 0x88, 0x40, 0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7};
  byte emptyNdef[16] = {0x03, 0x00, 0xFE, 0x00, 0,0,0,0,0,0,0,0,0,0,0,0};
  byte zeros[16] = {0};

  for (int s = 1; s < tag.sectors; s++) {
    int tb = sectorFirst(s) + sectorBlocks(s) - 1;
    selectTag(40);
    if (!classicAuth(tb)) {
      Serial.printf("[!] Gagal auth Sektor %d saat format\n", s);
      return false;
    }
    int firstBlock = sectorFirst(s);
    for (int b = firstBlock; b < tb; b++) {
      if (b == 4) {
        mfrc522.MIFARE_Write(b, emptyNdef, 16);
      } else {
        mfrc522.MIFARE_Write(b, zeros, 16);
      }
    }
    if (mfrc522.MIFARE_Write(tb, ndefTrailer, 16) != MFRC522::STATUS_OK) {
      Serial.printf("[!] Gagal menulis trailer Sektor %d\n", s);
      return false;
    }
  }

  Serial.println(F("OK: MIFARE Classic diformat NDEF (kompatibel NFC Tools & Android)."));
  return true;
}

bool writeClassicBytes(const byte* d, size_t len) {
  if (len > 720) return false;
  size_t off = 0;
  for (int s = 1; s < tag.sectors && off < len; s++) {
    int tb = sectorFirst(s) + sectorBlocks(s) - 1;
    selectTag(40);
    if (!classicAuth(tb)) {
      Serial.printf("[!] Gagal auth Sektor %d saat menulis\n", s);
      return false;
    }
    int firstBlock = sectorFirst(s);
    for (int b = firstBlock; b < tb && off < len; b++) {
      byte chunk[16] = {0};
      size_t n = (len - off < 16) ? (len - off) : 16;
      memcpy(chunk, d + off, n);
      off += n;
      if (mfrc522.MIFARE_Write(b, chunk, 16) != MFRC522::STATUS_OK) {
        Serial.printf("[!] Gagal tulis block %d\n", b);
        return false;
      }
    }
  }
  return true;
}

bool readClassicBytes(byte* out, size_t maxLen, size_t& bytesRead) {
  bytesRead = 0;
  for (int s = 1; s < tag.sectors && bytesRead < maxLen; s++) {
    int tb = sectorFirst(s) + sectorBlocks(s) - 1;
    selectTag(40);
    if (!classicAuth(tb)) break;
    int firstBlock = sectorFirst(s);
    for (int b = firstBlock; b < tb && bytesRead < maxLen; b++) {
      byte buf[18];
      byte sz = sizeof(buf);
      if (mfrc522.MIFARE_Read(b, buf, &sz) != MFRC522::STATUS_OK) {
        return bytesRead > 0;
      }
      size_t n = (maxLen - bytesRead < 16) ? (maxLen - bytesRead) : 16;
      memcpy(out + bytesRead, buf, n);
      bytesRead += n;
    }
  }
  return bytesRead > 0;
}

bool factoryResetClassic() {
  if (tag.kind != K_CLASSIC) return false;
  Serial.println(F("[*] Mengembalikan MIFARE Classic ke setelan pabrik (Key: FFFFFFFFFFFF)..."));
  byte fTrailer[16] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x07, 0x80, 0x69, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  byte zeros[16] = {0};

  for (int s = 0; s < tag.sectors; s++) {
    int tb = sectorFirst(s) + sectorBlocks(s) - 1;
    selectTag(40);
    if (!classicAuth(tb)) {
      Serial.printf("[!] Lewati Sektor %d (key tidak cocok)\n", s);
      continue;
    }
    int firstBlock = sectorFirst(s);
    for (int b = firstBlock; b < tb; b++) {
      if (b > 0) mfrc522.MIFARE_Write(b, zeros, 16); // Block 0 tidak boleh ditulis
    }
    mfrc522.MIFARE_Write(tb, fTrailer, 16);
  }
  sessKeySet = false;
  Serial.println(F("OK: MIFARE Classic direset ke default pabrik (FFFFFFFFFFFF)."));
  return true;
}

void printBlock(int b, const byte* d) {
  Serial.printf("B%03d: ", b); printHex(d, 16); Serial.print("  |"); printAscii(d, 16); Serial.println('|');
}

bool writeTrailer(int s, const byte* ka, const byte* kb) {
  if (s < 0 || s >= tag.sectors) { Serial.println(F("[!] Nomor sektor tidak valid")); return false; }
  int tb = sectorFirst(s) + sectorBlocks(s) - 1;
  if (!classicAuth(tb)) { Serial.println(F("[!] Auth gagal: key lama tidak diketahui. Set dulu: key <12hex>")); return false; }
  byte t[16];
  memcpy(t, ka, 6);
  t[6] = 0xFF; t[7] = 0x07; t[8] = 0x80; t[9] = 0x69; // access bits default
  memcpy(t + 10, kb, 6);
  MFRC522::StatusCode st = mfrc522.MIFARE_Write(tb, t, 16);
  if (st != MFRC522::STATUS_OK) { 
    Serial.print(F("[!] Gagal menulis trailer: ")); Serial.println(mfrc522.GetStatusCodeName(st)); return false; 
  }
  Serial.printf("OK: sektor %d  KeyA=", s); printHex(ka, 6); Serial.print("  KeyB="); printHex(kb, 6); Serial.println();
  memcpy(sessKey, ka, 6); sessKeySet = true;
  return true;
}

void cmdSetKey(String a) {
  if (tag.kind != K_CLASSIC) { Serial.println(F("[!] Hanya untuk MIFARE Classic")); return; }
  String ss = nextTok(a), sa = nextTok(a), sb = nextTok(a);
  byte ka[6], kb[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  if (!ss.length() || !parseHex(sa, ka, 6) || (sb.length() && !parseHex(sb, kb, 6))) {
    Serial.println(F("Pakai: setkey <sektor> <keyA 12hex> [keyB 12hex]")); return;
  }
  writeTrailer(ss.toInt(), ka, kb);
}

void cmdResetKey(String a) {
  if (tag.kind != K_CLASSIC) { Serial.println(F("[!] Hanya untuk MIFARE Classic")); return; }
  byte f[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  writeTrailer(nextTok(a).toInt(), f, f);
}

// ====================== FITUR SPESIFIK NTAG ======================
const char* pageLabel(uint16_t p) {
  if (p < 2) return "UID/BCC";
  if (p == 2) return "Lock bytes";
  if (p == 3) return "CC (Capability Container)";
  if (tag.cfg) {
    if (tag.dynLock && p == tag.dynLock) return "Dynamic lock";
    if (p == tag.cfg)     return "CFG0 (AUTH0)";
    if (p == tag.cfg + 1) return "CFG1 (ACCESS)";
    if (p == tag.cfg + 2) return "PWD";
    if (p == tag.cfg + 3) return "PACK";
  }
  return nullptr;
}

// ====================== STATUS KEAMANAN & PROTEKSI ======================
struct TagSecurity {
  bool supportsPwd = false;
  bool isLocked = false;
  bool pwdActive = false;
  bool configReadable = true;
  byte auth0 = 255;
  String protMode = "none"; // "w" (write-only), "rw" (read & write), "none"
  byte authLim = 0;
  bool authenticated = false;
};

TagSecurity getSecurityInfo() {
  TagSecurity sec;
  sec.supportsPwd = (tag.kind == K_UL && tag.cfg != 0);
  sec.authenticated = authed;

  if (tag.kind == K_UL) {
    byte b[18]; byte sz = sizeof(b);
    if (mfrc522.MIFARE_Read(2, b, &sz) == MFRC522::STATUS_OK) {
      sec.isLocked = (b[2] == 0xFF && b[3] == 0xFF);
    }
    if (sec.supportsPwd) {
      sz = sizeof(b);
      if (mfrc522.MIFARE_Read(tag.cfg, b, &sz) != MFRC522::STATUS_OK) {
        // Tag menolak pembacaan config -> password aktif dengan read-protection!
        sec.pwdActive = true;
        sec.configReadable = false;
        sec.protMode = "rw";
        sec.auth0 = 4;
        reopen();
      } else {
        sec.configReadable = true;
        sec.auth0 = b[3];
        byte acc = b[4];
        if (sec.auth0 < tag.pages) {
          sec.pwdActive = true;
          sec.protMode = (acc & 0x80) ? "rw" : "w";
          sec.authLim = acc & 7;
        } else {
          sec.pwdActive = false;
          sec.protMode = "none";
        }
      }
    }
  }
  return sec;
}

void showProtection() {
  TagSecurity sec = getSecurityInfo();
  if (sec.isLocked) Serial.println(F("Lock    : TERKUNCI PERMANEN (read-only)"));
  else Serial.println(F("Lock    : Terbuka (bisa ditulis)"));

  if (!sec.supportsPwd) return;

  if (!sec.configReadable) {
    Serial.println(F("Password: AKTIF (konfigurasi diproteksi baca) - jalankan 'auth <pwd>'"));
    return;
  }
  if (sec.pwdActive) {
    Serial.printf("Password: AKTIF mulai page %u, proteksi %s, batas salah %u (0=tak terbatas)\n",
                  sec.auth0, (sec.protMode == "rw") ? "BACA+TULIS" : "TULIS saja", sec.authLim);
    if (sec.authenticated) Serial.println(F("Sesi    : TERAUTENTIKASI (kartu terbuka)"));
    else Serial.println(F("Sesi    : BELUM DIAUTENTIKASI (kartu terkunci)"));
  } else {
    Serial.println(F("Password: Tidak aktif (bebas tanpa password)"));
  }
}

void cmdSetPwd(String a) {
  if (tag.kind != K_UL || !tag.cfg) { Serial.println(F("[!] Tag ini tidak mendukung password (butuh NTAG21x).")); return; }
  String sp = nextTok(a), mode = nextTok(a), s0 = nextTok(a), sl = nextTok(a);
  byte pwd[4];
  if (!parsePwd(sp, pwd)) { Serial.println(F("Pakai: setpwd <pwd> [rw|w] [auth0] [limit]  (pwd: 4 karakter / 8 hex)")); return; }
  bool rw = mode.equalsIgnoreCase("rw");
  int auth0 = s0.length() ? s0.toInt() : 4;
  int lim = sl.length() ? sl.toInt() : 0;
  if (auth0 < 4 || auth0 >= tag.pages || lim < 0 || lim > 7) { Serial.println(F("[!] auth0 harus 4..page terakhir, limit 0..7")); return; }
  byte b[18]; byte sz = sizeof(b);
  if (mfrc522.MIFARE_Read(tag.cfg, b, &sz) != MFRC522::STATUS_OK) {
    Serial.println(F("[!] Konfigurasi tidak terbaca. Jika tag sudah ber-password: auth <pwd> dulu.")); return;
  }
  byte c0[4], c1[4], pk[4] = {0x80, 0x80, 0, 0};
  memcpy(c0, b, 4); memcpy(c1, b + 4, 4);
  c1[0] = (c1[0] & 0x78) | (rw ? 0x80 : 0x00) | lim;
  c0[3] = auth0;
  bool ok = wrPage(tag.cfg + 2, pwd) && wrPage(tag.cfg + 3, pk) && wrPage(tag.cfg + 1, c1) && wrPage(tag.cfg, c0);
  if (!ok) { Serial.println(F("[!] Gagal menulis konfigurasi.")); return; }
  memcpy(sessPwd, pwd, 4); sessPwdSet = true;
  Serial.print(F("OK: password aktif. PWD=")); printHex(pwd, 4);
  Serial.printf("  PACK=8080  proteksi=%s  mulai page %d  limit=%d\n", rw ? "BACA+TULIS" : "TULIS saja", auth0, lim);
}

void cmdRemovePwd() {
  if (tag.kind != K_UL || !tag.cfg) return;
  byte b[18]; byte sz = sizeof(b);
  if (mfrc522.MIFARE_Read(tag.cfg, b, &sz) != MFRC522::STATUS_OK) { 
    Serial.println(F("[!] Gagal baca konfigurasi. Jalankan: auth <pwd>")); return; 
  }
  byte c0[4], c1[4], ff[4] = {0xFF, 0xFF, 0xFF, 0xFF}, z[4] = {0, 0, 0, 0};
  memcpy(c0, b, 4); memcpy(c1, b + 4, 4);
  c0[3] = 0xFF; c1[0] &= 0x78;
  bool ok = wrPage(tag.cfg, c0) && wrPage(tag.cfg + 1, c1) && wrPage(tag.cfg + 2, ff) && wrPage(tag.cfg + 3, z);
  if (ok) { sessPwdSet = false; Serial.println(F("OK: password dihapus.")); }
  else Serial.println(F("[!] Gagal menghapus password."));
}

void cmdLock(const String& a) {
  if (tag.kind != K_UL) return;
  if (a != "YES") { Serial.println(F("[!] Mengunci itu PERMANEN & tidak bisa dibatalkan. Ketik: lock YES")); return; }
  byte p2[4];
  if (!rdPage(2, p2)) { Serial.println(F("[!] Gagal baca lock bytes")); return; }
  p2[2] = 0xFF; p2[3] = 0xFF;
  bool ok = wrPage(2, p2);
  if (ok && tag.dynLock) { byte dl[4] = {0xFF, 0xFF, 0xFF, 0x00}; ok = wrPage(tag.dynLock, dl); }
  Serial.println(ok ? F("OK: tag dikunci permanen (read-only).") : F("[!] Gagal mengunci."));
}

void cmdErase() {
  if (tag.kind == K_UL) {
    memset(mem, 0, sizeof(mem));
    mem[0] = 0x03; mem[2] = 0xFE;
    if (writeUser(mem, userBytes())) Serial.println(F("OK: memori user dihapus + NDEF kosong."));
  } else if (tag.kind == K_CLASSIC) {
    factoryResetClassic();
  }
}

void cmdFormat() {
  if (tag.kind == K_UL) {
    byte cc[4];
    if (rdPage(3, cc) && !(cc[0] | cc[1] | cc[2] | cc[3])) {
      byte n[4] = {0xE1, 0x10, tag.cc ? tag.cc : 0x12, 0x00};
      if (!wrPage(3, n)) { Serial.println(F("[!] Gagal menulis CC")); return; }
    }
    static const byte empty[4] = {0x03, 0x00, 0xFE, 0x00};
    if (writeUser(empty, 4)) Serial.println(F("OK: tag NTAG diformat (NDEF kosong)."));
  } else if (tag.kind == K_CLASSIC) {
    formatClassicNdef();
  }
}

void cmdCopy() {
  if (tag.kind == K_UL) {
    if (!readUser(mem)) { Serial.println(F("[!] Gagal membaca tag")); return; }
    size_t st, len, end;
    if (findNdef(mem, userBytes(), st, len, end)) { memcpy(clip, mem, end); clipLen = end; }
    else { clip[0] = 0x03; clip[1] = 0x00; clip[2] = 0xFE; clipLen = 3; }
    Serial.printf("OK: %u byte disalin dari NTAG. Dekatkan tag tujuan lalu ketik: paste\n", clipLen);
  } else if (tag.kind == K_CLASSIC) {
    size_t br = 0;
    if (!readClassicBytes(mem, sizeof(mem), br)) { Serial.println(F("[!] Gagal membaca tag Classic")); return; }
    size_t st, len, end;
    if (findNdef(mem, br, st, len, end)) { memcpy(clip, mem, end); clipLen = end; }
    else { clip[0] = 0x03; clip[1] = 0x00; clip[2] = 0xFE; clipLen = 3; }
    Serial.printf("OK: %u byte disalin dari Classic. Dekatkan tag tujuan lalu ketik: paste\n", clipLen);
  }
}

void cmdPaste() {
  if (!clipLen) { Serial.println(F("[!] Clipboard kosong. Jalankan 'copy' dulu.")); return; }
  if (clipLen > userBytes()) { Serial.println(F("[!] Data tidak muat di tag ini")); return; }
  if (tag.kind == K_UL) {
    if (writeUser(clip, clipLen)) { Serial.println(F("OK: ditempel ke NTAG.")); showNdef(); }
  } else if (tag.kind == K_CLASSIC) {
    if (writeClassicBytes(clip, clipLen)) { Serial.println(F("OK: ditempel ke Classic.")); showNdef(); }
  }
}

// ====================== PERINTAH UMUM ======================
void cmdScan() {
  Serial.print(F("UID    : ")); printHex(mfrc522.uid.uidByte, mfrc522.uid.size); Serial.println();
  Serial.printf("Tipe   : %s (SAK 0x%02X)\n", tag.name, mfrc522.uid.sak);
  if (tag.kind == K_UL) {
    Serial.printf("Memori : %u page, %u byte user\n", tag.pages, userBytes());
    if (authed) Serial.printf("Auth   : OK (PACK %02X%02X)\n", lastPack[0], lastPack[1]);
    showProtection();
    showNdef();
  } else if (tag.kind == K_CLASSIC) {
    Serial.printf("Memori : %u sektor, %u block (%u byte user NDEF)\n", tag.sectors, tag.blocks, userBytes());
    showNdef();
  } else Serial.println(F("Tipe tag ini belum didukung."));
}

void cmdDump() {
  if (tag.kind == K_UL) {
    for (uint16_t p = 0; p < tag.pages; p++) {
      byte d[4];
      Serial.printf("P%03u: ", p);
      if (rdPage(p, d)) { printHex(d, 4); Serial.print("  |"); printAscii(d, 4); Serial.print('|'); }
      else { Serial.print(F("?? ?? ?? ??  (tidak terbaca)")); reopen(); }
      const char* l = pageLabel(p);
      if (l) { Serial.print("  "); Serial.print(l); }
      Serial.println();
    }
  } else if (tag.kind == K_CLASSIC) {
    for (int s = 0; s < tag.sectors; s++) {
      int f = sectorFirst(s), n = sectorBlocks(s);
      selectTag(40);
      if (!classicAuth(f)) { Serial.printf("S%02d: auth gagal (key tidak diketahui)\n", s); continue; }
      for (int b = f; b < f + n; b++) {
        byte buf[18]; byte sz = sizeof(buf);
        if (mfrc522.MIFARE_Read(b, buf, &sz) == MFRC522::STATUS_OK) {
          printBlock(b, buf);
          if (b == f + n - 1) Serial.println(F("      ^ sector trailer (KeyA | access bits | KeyB)"));
        } else Serial.printf("B%03d: gagal baca\n", b);
      }
    }
  } else Serial.println(F("Tipe tag ini belum didukung."));
}

void cmdRead(String a) {
  String sn = nextTok(a);
  if (!sn.length()) { Serial.println(F("Pakai: read <page|block>")); return; }
  int n = sn.toInt();
  byte buf[18]; byte sz = sizeof(buf);
  if (tag.kind == K_UL) {
    if (n < 0 || n >= tag.pages) { Serial.println(F("[!] Page di luar jangkauan")); return; }
    if (mfrc522.MIFARE_Read(n, buf, &sz) != MFRC522::STATUS_OK) { Serial.println(F("[!] Gagal baca (terproteksi?)")); return; }
    for (int i = 0; i < 4 && n + i < tag.pages; i++) {
      Serial.printf("P%03d: ", n + i); printHex(buf + i * 4, 4); Serial.print("  |"); printAscii(buf + i * 4, 4); Serial.println('|');
    }
  } else if (tag.kind == K_CLASSIC) {
    if (n < 0 || n >= tag.blocks) { Serial.println(F("[!] Block di luar jangkauan")); return; }
    if (!classicAuth(n)) { Serial.println(F("[!] Auth gagal (key tidak diketahui)")); return; }
    if (mfrc522.MIFARE_Read(n, buf, &sz) == MFRC522::STATUS_OK) printBlock(n, buf);
    else Serial.println(F("[!] Gagal baca block"));
  }
}

void cmdWrite(String a) {
  String sn = nextTok(a), sh = nextTok(a);
  int n = sn.toInt();
  if (tag.kind == K_UL) {
    byte d[4];
    if (!sn.length() || !parseHex(sh, d, 4)) { Serial.println(F("Pakai: write <page> <8 hex>")); return; }
    if (n < 4 || n > tag.userEnd) {
      Serial.printf("[!] Hanya page 4..%u\n", tag.userEnd); return;
    }
    if (wrPage(n, d)) Serial.println(F("OK")); else Serial.println(F("[!] Gagal menulis"));
  } else if (tag.kind == K_CLASSIC) {
    byte d[16];
    if (!sn.length() || !parseHex(sh, d, 16)) { Serial.println(F("Pakai: write <block> <32 hex>")); return; }
    if (n <= 0 || n >= tag.blocks || isTrailer(n)) { Serial.println(F("[!] Block 0 & sector trailer tidak boleh")); return; }
    if (!classicAuth(n)) { Serial.println(F("[!] Auth gagal")); return; }
    MFRC522::StatusCode st = mfrc522.MIFARE_Write(n, d, 16);
    if (st == MFRC522::STATUS_OK) Serial.println(F("OK"));
    else { Serial.print(F("[!] Gagal: ")); Serial.println(mfrc522.GetStatusCodeName(st)); }
  }
}

void help() {
  Serial.print(R"HELP(
=== NFC Tools ESP32 + MFRC522 (Versi Lengkap) ===
Dashboard Web: http://192.168.4.1 (WiFi AP: ESP32-NFC-Tools)

BACA & INFORMASI:
  scan                       Info tag + isi NDEF (link URL / teks)
  dump                       Dump seluruh memori (Hex + ASCII)
  read <n>                   Baca page (NTAG, 4 page) / block (Classic)

TULIS NDEF (NTAG21x & MIFARE Classic):
  wurl <url>                 Tulis Link URL (otomatis https:// bila tanpa prefix)
  wtext <teks>               Tulis Teks (bahasa default 'en', ubah: lang id)
  wtel <nomor>               Tulis Nomor Telepon (contoh: wtel +628123456789)
  wmail <alamat>             Tulis Email (contoh: wmail info@example.com)
  wuri <uri lengkap>         Tulis URI Kustom (misal: geo:-7.1,110.2 atau sms:0812)
  wmime <tipe> <data>        Tulis MIME (misal: wmime text/plain Halo)
  write <n> <hex>            Tulis Raw: page NTAG=8 hex, block Classic=32 hex

MANAJEMEN KARTU:
  format                     Format NDEF:
                             - MIFARE Classic: MAD1 Sektor 0 + Kunci D3F7D3F7D3F7
                             - NTAG: Inisialisasi Capability Container (Page 3)
  erase                      Reset Kartu:
                             - MIFARE Classic: Reset ke default pabrik (FFFFFFFFFFFF)
                             - NTAG: Kosongkan memori user
  copy / paste               Salin isi NDEF dari satu kartu ke kartu lain

KEAMANAN NTAG (NTAG21x):
  setpwd <pwd> [rw|w] [auth0] [limit]   Kunci kartu NTAG dengan password
  removepwd                             Hapus password NTAG
  lock YES                              KUNCI PERMANEN (Read-Only)

MIFARE CLASSIC:
  key <12hex>                Key sesi kustom
  setkey <sektor> <keyA>     Ganti key sektor
  resetkey <sektor>          Kembalikan key sektor ke FFFFFFFFFFFF

SISTEM & AUTO SCAN:
  autoscan                   Cek status mode auto scan & durasi debounce
  autoscan on / off          Aktifkan / nonaktifkan auto scan
  autoscan <detik>           Atur jeda debounce (contoh: autoscan 3 untuk 3 detik)
  diag                       Uji hardware modul RC522 & kekuatan antena
  gain <18|23|33|38|43|48>   Atur sensitivitas penerima antena (dB)
  lang <id|en>               Atur kode bahasa NDEF Text
)HELP");
}

// ----- Diagnosa & Tuning Gain Antena -----
const byte GAIN_VAL[] = {0x00, 0x10, 0x40, 0x50, 0x60, 0x70};
const int  GAIN_DB[]  = {18, 23, 33, 38, 43, 48};
const int  NGAIN = 6;

bool probeOnce() {
  byte atqa[2]; byte len = sizeof(atqa);
  if (mfrc522.PICC_WakeupA(atqa, &len) != MFRC522::STATUS_OK) return false;
  bool ok = mfrc522.PICC_Select(&mfrc522.uid, 0) == MFRC522::STATUS_OK;
  if (ok) mfrc522.PICC_HaltA();
  return ok;
}

void cmdGain(const String& a) {
  int db = a.toInt();
  for (int i = 0; i < NGAIN; i++) {
    if (GAIN_DB[i] == db) {
      curGain = GAIN_VAL[i];
      mfrc522.PCD_SetAntennaGain(curGain);
      Serial.printf("OK: gain = %d dB\n", db);
      return;
    }
  }
  Serial.println(F("Pakai: gain <18|23|33|38|43|48>"));
}

void cmdDiag() {
  byte v = mfrc522.PCD_ReadRegister(MFRC522::VersionReg);
  Serial.printf("VersionReg : 0x%02X  ", v);
  if (v == 0x91 || v == 0x92) Serial.println(F("(Chip MFRC522 Asli, OK)"));
  else if (v == 0x00 || v == 0xFF) Serial.println(F("(TIDAK ADA RESPON -> periksa wiring 3.3V, GND, SPI!)"));
  else Serial.println(F("(Versi kompatibel clone, OK)"));
  byte tx = mfrc522.PCD_ReadRegister(MFRC522::TxControlReg);
  Serial.printf("Antena RF  : %s (TxControlReg=0x%02X)\n", (tx & 0x03) == 0x03 ? "ON" : "OFF", tx);
  Serial.println(F("Menguji kekuatan gain antena dengan kartu terdekat..."));
  int best = -1, bestOk = 0;
  for (int i = 0; i < NGAIN; i++) {
    mfrc522.PCD_SetAntennaGain(GAIN_VAL[i]);
    int ok = 0;
    for (int k = 0; k < 25; k++) { if (probeOnce()) ok++; delay(15); }
    Serial.printf("  gain %2d dB : %2d/25 respon\n", GAIN_DB[i], ok);
    if (ok > bestOk) { bestOk = ok; best = i; }
  }
  if (best >= 0) {
    curGain = GAIN_VAL[best];
    mfrc522.PCD_SetAntennaGain(curGain);
    Serial.printf("Dipilih gain terbaik: %d dB (%d/25 respon)\n", GAIN_DB[best], bestOk);
  }
}

// ====================== DISPATCH PERINTAH SERIAL ======================
void handle(String line) {
  line.trim();
  String cmd = nextTok(line);
  cmd.toLowerCase();
  String a = line;

  if (cmd == "help" || cmd == "?") { help(); return; }
  if (cmd == "diag") { cmdDiag(); return; }
  if (cmd == "gain") { cmdGain(a); return; }
  if (cmd == "lang") { 
    if (a.length() >= 2 && a.length() <= 8) { ndefLang = a; Serial.println("OK: bahasa = " + a); } 
    else Serial.println(F("Pakai: lang <kode>, misal: lang id")); 
    return; 
  }
  if (cmd == "unauth") { sessPwdSet = false; Serial.println(F("Password sesi dilepas.")); return; }
  if (cmd == "key") {
    byte k[6];
    if (parseHex(nextTok(a), k, 6)) { memcpy(sessKey, k, 6); sessKeySet = true; Serial.println(F("OK: key sesi disimpan.")); }
    else Serial.println(F("Pakai: key <12 hex>"));
    return;
  }
  if (cmd == "auth") {
    if (!parsePwd(nextTok(a), sessPwd)) { Serial.println(F("Pakai: auth <pwd>")); return; }
    sessPwdSet = true;
    if (prepare()) {
      if (tag.kind == K_UL && tag.cfg && authed) Serial.printf("Auth OK (PACK %02X%02X)\n", lastPack[0], lastPack[1]);
      endTag();
    }
    return;
  }
  if (cmd == "autoscan") {
    if (a == "on" || a == "1") {
      autoScanEnabled = true;
      Serial.println(F("OK: Mode Auto Scan DIAKTIFKAN."));
    } else if (a == "off" || a == "0") {
      autoScanEnabled = false;
      Serial.println(F("OK: Mode Auto Scan DINONAKTIFKAN."));
    } else if (a.length() && a.toInt() > 0) {
      autoScanDebounceMs = a.toInt() * 1000;
      autoScanEnabled = true;
      Serial.printf("OK: Auto Scan AKTIF dengan debounce %d detik.\n", a.toInt());
    } else {
      Serial.printf("Status Auto Scan : %s\nDurasi Debounce  : %u detik\n", 
                    autoScanEnabled ? "AKTIF" : "NONAKTIF", 
                    (unsigned)(autoScanDebounceMs / 1000));
      Serial.println(F("Pakai: autoscan <on|off|<detik>>  contoh: autoscan 3"));
    }
    return;
  }
  if (String(TAGCMDS).indexOf(" " + cmd + " ") < 0) { 
    Serial.println(F("Perintah tidak dikenal. Ketik 'help' untuk daftar perintah.")); 
    return; 
  }
  if (!a.length() && (cmd == "wtext" || cmd == "wurl" || cmd == "wuri" || cmd == "wtel" || cmd == "wmail")) {
    Serial.println(F("Argumen kosong! Contoh: wurl https://google.com")); 
    return;
  }

  if (!prepare()) {
    Serial.println(F("[!] Tag tidak terdeteksi. Tempelkan kartu/tag NFC ke sensor RC522."));
    return;
  }

  if (cmd == "scan") cmdScan();
  else if (cmd == "dump") cmdDump();
  else if (cmd == "read") cmdRead(a);
  else if (cmd == "write") cmdWrite(a);
  else if (cmd == "wtext") cmdWText(a);
  else if (cmd == "wurl") { if (a.indexOf("://") < 0) a = "https://" + a; cmdWUri(a); }
  else if (cmd == "wuri") cmdWUri(a);
  else if (cmd == "wtel") cmdWUri("tel:" + a);
  else if (cmd == "wmail") cmdWUri("mailto:" + a);
  else if (cmd == "wmime") cmdWMime(a);
  else if (cmd == "erase") cmdErase();
  else if (cmd == "format") cmdFormat();
  else if (cmd == "lock") cmdLock(a);
  else if (cmd == "setpwd") cmdSetPwd(a);
  else if (cmd == "removepwd") cmdRemovePwd();
  else if (cmd == "copy") cmdCopy();
  else if (cmd == "paste") cmdPaste();
  else if (cmd == "setkey") cmdSetKey(a);
  else if (cmd == "resetkey") cmdResetKey(a);
  endTag();
}

// ====================== WEB SERVER & DASHBOARD GUI ======================
void handleRoot() {
  server.send_P(200, "text/html", INDEX_HTML);
}

void handleApiScan() {
  if (!prepare()) {
    server.send(200, "application/json", "{\"detected\":false}");
    return;
  }
  showNdef();
  TagSecurity sec = getSecurityInfo();
  String json = "{";
  json += "\"detected\":true,";
  json += "\"uid\":\"" + getUidString() + "\",";
  json += "\"type\":\"" + String(tag.name) + "\",";
  json += "\"size\":" + String(userBytes()) + ",";
  json += "\"ndef_found\":" + String(lastNdef.found ? "true" : "false") + ",";
  json += "\"ndef_type\":\"" + lastNdef.type + "\",";
  String safePayload = lastNdef.payload;
  safePayload.replace("\\", "\\\\");
  safePayload.replace("\"", "\\\"");
  json += "\"ndef_payload\":\"" + safePayload + "\",";
  json += "\"supports_pwd\":" + String(sec.supportsPwd ? "true" : "false") + ",";
  json += "\"is_locked\":" + String(sec.isLocked ? "true" : "false") + ",";
  json += "\"pwd_active\":" + String(sec.pwdActive ? "true" : "false") + ",";
  json += "\"config_readable\":" + String(sec.configReadable ? "true" : "false") + ",";
  json += "\"prot_mode\":\"" + sec.protMode + "\",";
  json += "\"auth0\":" + String(sec.auth0) + ",";
  json += "\"authlim\":" + String(sec.authLim) + ",";
  json += "\"authenticated\":" + String(sec.authenticated ? "true" : "false");
  json += "}";
  endTag();
  server.send(200, "application/json", json);
}

void handleApiWrite() {
  if (!server.hasArg("val")) {
    server.send(400, "application/json", "{\"success\":false,\"message\":\"Data kosong\"}");
    return;
  }
  String ty = server.arg("type");
  String val = server.arg("val");

  if (!prepare()) {
    server.send(200, "application/json", "{\"success\":false,\"message\":\"Tag tidak terdeteksi. Tempelkan kartu ke reader!\"}");
    return;
  }

  TagSecurity sec = getSecurityInfo();
  if (sec.isLocked) {
    endTag();
    server.send(200, "application/json", "{\"success\":false,\"message\":\"Gagal: Kartu ini telah dikunci permanen (Read-Only) dan tidak dapat ditulisi lagi.\"}");
    return;
  }
  if (sec.pwdActive && !sec.authenticated) {
    endTag();
    server.send(200, "application/json", "{\"success\":false,\"message\":\"Gagal: Kartu terlindungi password. Buka tab Keamanan dan lakukan Autentikasi terlebih dahulu.\"}");
    return;
  }

  bool ok = false;
  if (ty == "url" || ty == "uri") {
    cmdWUri(val);
    ok = lastNdef.found;
  } else if (ty == "text") {
    cmdWText(val);
    ok = lastNdef.found;
  } else if (ty == "tel") {
    cmdWUri("tel:" + val);
    ok = lastNdef.found;
  } else if (ty == "mail") {
    cmdWUri("mailto:" + val);
    ok = lastNdef.found;
  }
  endTag();

  if (ok) {
    server.send(200, "application/json", "{\"success\":true,\"message\":\"Data NDEF berhasil ditulis ke kartu!\"}");
  } else {
    server.send(200, "application/json", "{\"success\":false,\"message\":\"Gagal menulis NDEF. Jika kartu terlindungi password, buka kunci di tab Keamanan.\"}");
  }
}

void handleApiFormat() {
  if (!prepare()) {
    server.send(200, "application/json", "{\"success\":false,\"message\":\"Tag tidak terdeteksi.\"}");
    return;
  }
  TagSecurity sec = getSecurityInfo();
  if (sec.isLocked) {
    endTag();
    server.send(200, "application/json", "{\"success\":false,\"message\":\"Gagal: Kartu telah dikunci permanen (Read-Only).\"}");
    return;
  }
  if (sec.pwdActive && !sec.authenticated) {
    endTag();
    server.send(200, "application/json", "{\"success\":false,\"message\":\"Gagal: Kartu terlindungi password! Buka tab Keamanan dan masukkan password terlebih dahulu.\"}");
    return;
  }

  bool ok = false;
  if (tag.kind == K_CLASSIC) {
    ok = formatClassicNdef();
  } else if (tag.kind == K_UL) {
    cmdFormat();
    ok = true;
  }
  endTag();
  if (ok) {
    server.send(200, "application/json", "{\"success\":true,\"message\":\"Kartu berhasil diformat ke standar NFC Forum NDEF!\"}");
  } else {
    server.send(200, "application/json", "{\"success\":false,\"message\":\"Gagal memformat kartu.\"}");
  }
}

void handleApiErase() {
  if (!prepare()) {
    server.send(200, "application/json", "{\"success\":false,\"message\":\"Tag tidak terdeteksi.\"}");
    return;
  }
  TagSecurity sec = getSecurityInfo();
  if (sec.isLocked) {
    endTag();
    server.send(200, "application/json", "{\"success\":false,\"message\":\"Gagal: Kartu telah dikunci permanen (Read-Only).\"}");
    return;
  }
  if (sec.pwdActive && !sec.authenticated) {
    endTag();
    server.send(200, "application/json", "{\"success\":false,\"message\":\"Gagal: Kartu terlindungi password! Buka tab Keamanan dan masukkan password terlebih dahulu.\"}");
    return;
  }

  if (tag.kind == K_CLASSIC) {
    factoryResetClassic();
  } else if (tag.kind == K_UL) {
    cmdErase();
  }
  endTag();
  server.send(200, "application/json", "{\"success\":true,\"message\":\"Kartu berhasil direset ke kondisi awal.\"}");
}

void handleApiSecurity() {
  if (!prepare()) {
    server.send(200, "application/json", "{\"detected\":false}");
    return;
  }
  TagSecurity sec = getSecurityInfo();
  String json = "{";
  json += "\"detected\":true,";
  json += "\"uid\":\"" + getUidString() + "\",";
  json += "\"type\":\"" + String(tag.name) + "\",";
  json += "\"supports_pwd\":" + String(sec.supportsPwd ? "true" : "false") + ",";
  json += "\"is_locked\":" + String(sec.isLocked ? "true" : "false") + ",";
  json += "\"pwd_active\":" + String(sec.pwdActive ? "true" : "false") + ",";
  json += "\"config_readable\":" + String(sec.configReadable ? "true" : "false") + ",";
  json += "\"auth0\":" + String(sec.auth0) + ",";
  json += "\"prot_mode\":\"" + sec.protMode + "\",";
  json += "\"authlim\":" + String(sec.authLim) + ",";
  json += "\"authenticated\":" + String(sec.authenticated ? "true" : "false");
  json += "}";
  endTag();
  server.send(200, "application/json", json);
}

void handleApiAuth() {
  if (!server.hasArg("pwd")) {
    server.send(400, "application/json", "{\"success\":false,\"message\":\"Parameter password tidak ada.\"}");
    return;
  }
  String pwdStr = server.arg("pwd");
  pwdStr.trim();
  byte bPwd[4];
  if (!parsePwd(pwdStr, bPwd)) {
    server.send(200, "application/json", "{\"success\":false,\"message\":\"Format password salah (harus 4 karakter atau 8 hex, misal: 1234 atau A1B2C3D4).\"}");
    return;
  }

  if (!prepare()) {
    server.send(200, "application/json", "{\"success\":false,\"message\":\"Tag tidak terdeteksi. Tempelkan kartu ke reader!\"}");
    return;
  }

  if (tag.kind != K_UL || !tag.cfg) {
    endTag();
    server.send(200, "application/json", "{\"success\":false,\"message\":\"Kartu ini bukan NTAG21x (tidak mendukung autentikasi password).\"}");
    return;
  }

  byte pack[2];
  if (ntagAuth(bPwd, pack)) {
    memcpy(sessPwd, bPwd, 4);
    sessPwdSet = true;
    authed = true;
    char packHex[8];
    snprintf(packHex, sizeof(packHex), "%02X%02X", pack[0], pack[1]);
    endTag();
    server.send(200, "application/json", "{\"success\":true,\"message\":\"Autentikasi berhasil! Kartu terbuka untuk operasi baca/tulis.\",\"pack\":\"" + String(packHex) + "\"}");
  } else {
    endTag();
    server.send(200, "application/json", "{\"success\":false,\"message\":\"Autentikasi gagal! Password salah atau kartu menolak akses.\"}");
  }
}

void handleApiUnauth() {
  sessPwdSet = false;
  authed = false;
  server.send(200, "application/json", "{\"success\":true,\"message\":\"Autentikasi sesi dinonaktifkan (kartu terkunci kembali).\"}");
}

void handleApiSetPwd() {
  if (!server.hasArg("pwd")) {
    server.send(400, "application/json", "{\"success\":false,\"message\":\"Password tidak boleh kosong.\"}");
    return;
  }
  String sp = server.arg("pwd");
  sp.trim();
  byte pwd[4];
  if (!parsePwd(sp, pwd)) {
    server.send(200, "application/json", "{\"success\":false,\"message\":\"Password harus 4 karakter teks atau 8 digit HEX.\"}");
    return;
  }

  String mode = server.hasArg("mode") ? server.arg("mode") : "w";
  bool rw = mode.equalsIgnoreCase("rw");
  int auth0 = server.hasArg("auth0") ? server.arg("auth0").toInt() : 4;
  int lim = server.hasArg("limit") ? server.arg("limit").toInt() : 0;

  if (!prepare()) {
    server.send(200, "application/json", "{\"success\":false,\"message\":\"Tag tidak terdeteksi. Tempelkan kartu ke reader!\"}");
    return;
  }

  if (tag.kind != K_UL || !tag.cfg) {
    endTag();
    server.send(200, "application/json", "{\"success\":false,\"message\":\"Tag ini bukan NTAG21x (tidak mendukung password).\"}");
    return;
  }

  if (auth0 < 4 || auth0 >= tag.pages || lim < 0 || lim > 7) {
    endTag();
    server.send(200, "application/json", "{\"success\":false,\"message\":\"Nilai auth0 atau limit tidak valid (auth0: 4..page akhir, limit: 0..7).\"}");
    return;
  }

  byte b[18]; byte sz = sizeof(b);
  if (mfrc522.MIFARE_Read(tag.cfg, b, &sz) != MFRC522::STATUS_OK) {
    endTag();
    server.send(200, "application/json", "{\"success\":false,\"message\":\"Gagal membaca konfigurasi tag. Jika tag sudah ber-password, lakukan Autentikasi dengan password lama terlebih dahulu!\"}");
    return;
  }

  byte c0[4], c1[4], pk[4] = {0x80, 0x80, 0, 0};
  memcpy(c0, b, 4); memcpy(c1, b + 4, 4);
  c1[0] = (c1[0] & 0x78) | (rw ? 0x80 : 0x00) | lim;
  c0[3] = (byte)auth0;

  bool ok = wrPage(tag.cfg + 2, pwd) && 
            wrPage(tag.cfg + 3, pk) && 
            wrPage(tag.cfg + 1, c1) && 
            wrPage(tag.cfg, c0);
  endTag();

  if (ok) {
    memcpy(sessPwd, pwd, 4);
    sessPwdSet = true;
    authed = true;
    server.send(200, "application/json", "{\"success\":true,\"message\":\"Password berhasil dipasang! Proteksi: " + String(rw ? "BACA & TULIS" : "HANYA TULIS") + " mulai page " + String(auth0) + ".\"}");
  } else {
    server.send(200, "application/json", "{\"success\":false,\"message\":\"Gagal menulis konfigurasi password ke chip tag.\"}");
  }
}

void handleApiRemovePwd() {
  if (!prepare()) {
    server.send(200, "application/json", "{\"success\":false,\"message\":\"Tag tidak terdeteksi. Tempelkan kartu ke reader!\"}");
    return;
  }

  if (tag.kind != K_UL || !tag.cfg) {
    endTag();
    server.send(200, "application/json", "{\"success\":false,\"message\":\"Tag ini bukan NTAG21x.\"}");
    return;
  }

  byte b[18]; byte sz = sizeof(b);
  if (mfrc522.MIFARE_Read(tag.cfg, b, &sz) != MFRC522::STATUS_OK) {
    endTag();
    server.send(200, "application/json", "{\"success\":false,\"message\":\"Konfigurasi ditolak oleh chip! Tag masih terkunci password. Buka kunci dengan password yang benar terlebih dahulu.\"}");
    return;
  }

  byte c0[4], c1[4], ff[4] = {0xFF, 0xFF, 0xFF, 0xFF}, z[4] = {0, 0, 0, 0};
  memcpy(c0, b, 4); memcpy(c1, b + 4, 4);
  c0[3] = 0xFF; // Nonaktifkan AUTH0
  c1[0] &= 0x78; // Matikan PROT dan AUTHLIM

  bool ok = wrPage(tag.cfg, c0) && 
            wrPage(tag.cfg + 1, c1) && 
            wrPage(tag.cfg + 2, ff) && 
            wrPage(tag.cfg + 3, z);
  endTag();

  if (ok) {
    sessPwdSet = false;
    authed = false;
    server.send(200, "application/json", "{\"success\":true,\"message\":\"Password berhasil dihapus! Tag sekarang bebas tanpa proteksi.\"}");
  } else {
    server.send(200, "application/json", "{\"success\":false,\"message\":\"Gagal menghapus password dari tag (pastikan sudah diautentikasi).\"}");
  }
}

void handleApiLock() {
  if (server.arg("confirm") != "YES") {
    server.send(200, "application/json", "{\"success\":false,\"message\":\"Ketik 'YES' untuk konfirmasi penguncian permanen.\"}");
    return;
  }
  if (!prepare()) {
    server.send(200, "application/json", "{\"success\":false,\"message\":\"Tag tidak terdeteksi.\"}");
    return;
  }
  if (tag.kind != K_UL) {
    endTag();
    server.send(200, "application/json", "{\"success\":false,\"message\":\"Fitur lock ini untuk tag NTAG / Ultralight.\"}");
    return;
  }
  byte p2[4];
  if (!rdPage(2, p2)) {
    endTag();
    server.send(200, "application/json", "{\"success\":false,\"message\":\"Gagal membaca lock bytes (mungkin terproteksi password).\"}");
    return;
  }
  p2[2] = 0xFF; p2[3] = 0xFF;
  bool ok = wrPage(2, p2);
  if (ok && tag.dynLock) {
    byte dl[4] = {0xFF, 0xFF, 0xFF, 0x00};
    ok = wrPage(tag.dynLock, dl);
  }
  endTag();
  if (ok) {
    server.send(200, "application/json", "{\"success\":true,\"message\":\"Tag berhasil dikunci permanen (Read-Only). Tidak dapat ditulisi lagi selamanya.\"}");
  } else {
    server.send(200, "application/json", "{\"success\":false,\"message\":\"Gagal mengunci tag.\"}");
  }
}

void handleApiDump() {
  if (!prepare()) {
    server.send(200, "application/json", "{\"success\":false,\"message\":\"Tag tidak terdeteksi.\"}");
    return;
  }
  String json = "{\"success\":true,\"rows\":[";
  bool first = true;

  if (tag.kind == K_UL) {
    for (uint16_t p = 0; p < tag.pages; p++) {
      byte d[4];
      if (rdPage(p, d)) {
        if (!first) json += ",";
        first = false;
        char hexBuf[16];
        snprintf(hexBuf, sizeof(hexBuf), "%02X %02X %02X %02X", d[0], d[1], d[2], d[3]);
        String asc = "";
        for (int k = 0; k < 4; k++) asc += (d[k] >= 32 && d[k] < 127) ? (char)d[k] : '.';
        const char* lbl = pageLabel(p);
        json += "{\"idx\":\"P" + String(p) + "\",\"hex\":\"" + String(hexBuf) + "\",\"ascii\":\"" + asc + "\",\"label\":\"" + String(lbl ? lbl : "") + "\"}";
      }
    }
  } else if (tag.kind == K_CLASSIC) {
    for (int s = 0; s < tag.sectors; s++) {
      int f = sectorFirst(s), n = sectorBlocks(s);
      selectTag(40);
      if (!classicAuth(f)) continue;
      for (int b = f; b < f + n; b++) {
        byte buf[18]; byte sz = sizeof(buf);
        if (mfrc522.MIFARE_Read(b, buf, &sz) == MFRC522::STATUS_OK) {
          if (!first) json += ",";
          first = false;
          String hStr = "";
          for (int k = 0; k < 16; k++) {
            if (buf[k] < 0x10) hStr += "0";
            hStr += String(buf[k], HEX);
            if (k < 15) hStr += " ";
          }
          hStr.toUpperCase();
          String asc = "";
          for (int k = 0; k < 16; k++) asc += (buf[k] >= 32 && buf[k] < 127) ? (char)buf[k] : '.';
          String lbl = (b == f + n - 1) ? "Trailer S" + String(s) : (b == 0 ? "UID" : "Data");
          json += "{\"idx\":\"B" + String(b) + "\",\"hex\":\"" + hStr + "\",\"ascii\":\"" + asc + "\",\"label\":\"" + lbl + "\"}";
        }
      }
    }
  }
  json += "]}";
  endTag();
  server.send(200, "application/json", json);
}

void handleApiAutoScan() {
  if (server.hasArg("enabled")) {
    autoScanEnabled = (server.arg("enabled") == "true" || server.arg("enabled") == "1");
  }
  if (server.hasArg("debounce")) {
    int sec = server.arg("debounce").toInt();
    if (sec > 0) autoScanDebounceMs = sec * 1000;
  }
  String json = "{";
  json += "\"autoscan\":" + String(autoScanEnabled ? "true" : "false") + ",";
  json += "\"debounce\":" + String(autoScanDebounceMs / 1000);
  json += "}";
  server.send(200, "application/json", json);
}

void handleAutoScan() {
  if (!autoScanEnabled) return;

  // Cek setiap 120ms agar responsif tapi tidak membebani loop/SPI
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck < 120) return;
  lastCheck = millis();

  byte atqa[2]; byte len = sizeof(atqa);
  bool cardDetected = (mfrc522.PICC_WakeupA(atqa, &len) == MFRC522::STATUS_OK) &&
                      (mfrc522.PICC_Select(&mfrc522.uid, 0) == MFRC522::STATUS_OK);

  if (cardDetected) {
    String currentUid = getUidString();
    unsigned long now = millis();

    bool trigger = false;
    if (!cardCurrentlyPresent) {
      trigger = true; // Kartu baru saja ditempelkan
    } else if (currentUid != lastScannedUid) {
      trigger = true; // Kartu diganti dengan kartu lain
    } else if (now - lastScanTime >= autoScanDebounceMs) {
      trigger = true; // Kartu sama tapi durasi debounce sudah lewat
    }

    if (trigger) {
      lastScanTime = now;
      lastScannedUid = currentUid;
      cardCurrentlyPresent = true;

      detect();
      authSession();

      Serial.println(F("\n----------------------------------------"));
      Serial.println(F("  >>> AUTO SCAN: KARTU TERDETEKSI <<<   "));
      Serial.println(F("----------------------------------------"));
      cmdScan();
      Serial.print(F("> "));
    }

    endTag();
  } else {
    // Toleransi 2x polling berturut-turut tanpa sinyal menandakan kartu diangkat
    static byte noCardStreak = 0;
    if (++noCardStreak >= 2) {
      cardCurrentlyPresent = false;
      noCardStreak = 0;
    }
  }
}

// ====================== SETUP & LOOP ======================
void setup() {
  Serial.begin(115200);
  delay(300);
  memset(sessKey, 0xFF, 6);

  // Inisialisasi SPI & MFRC522
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
  mfrc522.PCD_Init();
  mfrc522.PCD_SetAntennaGain(curGain);

  // Inisialisasi WiFi AP untuk Dashboard Web
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS);
  IPAddress myIP = WiFi.softAPIP();

  // Setup rute Web Server
  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/scan", HTTP_GET, handleApiScan);
  server.on("/api/write", HTTP_POST, handleApiWrite);
  server.on("/api/format", HTTP_POST, handleApiFormat);
  server.on("/api/erase", HTTP_POST, handleApiErase);
  server.on("/api/dump", HTTP_GET, handleApiDump);
  server.on("/api/autoscan", HTTP_POST, handleApiAutoScan);
  server.on("/api/autoscan", HTTP_GET, handleApiAutoScan);
  server.on("/api/security", HTTP_GET, handleApiSecurity);
  server.on("/api/auth", HTTP_POST, handleApiAuth);
  server.on("/api/unauth", HTTP_POST, handleApiUnauth);
  server.on("/api/setpwd", HTTP_POST, handleApiSetPwd);
  server.on("/api/removepwd", HTTP_POST, handleApiRemovePwd);
  server.on("/api/lock", HTTP_POST, handleApiLock);
  server.begin();

  byte v = mfrc522.PCD_ReadRegister(MFRC522::VersionReg);
  Serial.println(F("\n========================================================"));
  Serial.println(F("       NFC TOOLS ESP32 + MFRC522 (Reader & Writer)      "));
  Serial.println(F("========================================================"));
  if (v == 0x00 || v == 0xFF) {
    Serial.println(F("[!] PERINGATAN: MFRC522 tidak terdeteksi!"));
    Serial.println(F("    Periksa kabel jumper SPI (3.3V, GND, RST, SDA, SCK, MOSI, MISO)."));
  } else {
    Serial.printf("RC522 OK (Versi chip 0x%02X)\n", v);
  }
  Serial.printf("Mode Auto Scan: %s (Debounce: %u detik)\n", 
                autoScanEnabled ? "AKTIF" : "NONAKTIF", 
                (unsigned)(autoScanDebounceMs / 1000));
  Serial.println(F("\n>>> FITUR WEB DASHBOARD AKTIF:"));
  Serial.printf("    1. Hubungkan WiFi Laptop/HP ke SSID : %s\n", AP_SSID);
  Serial.printf("    2. Buka Browser ke Alamat           : http://%s\n", myIP.toString().c_str());
  Serial.println(F("\n>>> ATAU GUNAKAN SERIAL MONITOR:"));
  Serial.println(F("    Ketik 'help' untuk melihat daftar perintah."));
  Serial.println(F("    Contoh: wurl https://google.com"));
  Serial.println(F("========================================================\n> "));
}

void loop() {
  // Tangani request HTTP Web Dashboard
  server.handleClient();

  // Mode Auto Scan otomatis dengan debounce
  handleAutoScan();

  // Tangani input perintah Serial Monitor
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\r' || c == '\n') {
      if (inLine.length()) { 
        handle(inLine); 
        inLine = ""; 
        Serial.print("> "); 
      }
    } else {
      inLine += c;
    }
  }
}
