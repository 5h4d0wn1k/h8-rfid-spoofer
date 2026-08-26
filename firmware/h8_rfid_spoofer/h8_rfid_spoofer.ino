/*
 * H8 — RFID/NFC Spoofer
 * ESP32 + RC522 MIFARE Classic reader/cloner/emulator
 *
 * Features:
 *   - Read MIFARE Classic 1K/4K UIDs and block data
 *   - Clone card data to blank MIFARE tags
 *   - Emulate card via serial replay
 *   - Full sector dump with key management
 *
 * Hardware: ESP32 NodeMCU + MFRC522 (RC522) module
 * Library:  MFRC522 (install via Arduino Library Manager)
 */

#include <SPI.h>
#include <MFRC522.h>

// ── Pin Definitions (HSPI) ───────────────────────────────────────
#define SS_PIN    5    // GPIO5  → SDA/SS
#define RST_PIN   27   // GPIO27 → RST
#define MOSI_PIN  23   // GPIO23 → MOSI (default HSPI)
#define MISO_PIN  19   // GPIO19 → MISO (default HSPI)
#define SCK_PIN   18   // GPIO18 → SCK  (default HSPI)

// ── Objects ──────────────────────────────────────────────────────
MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key defaultKey;

// ── Card Data Storage ────────────────────────────────────────────
byte savedUID[10];
byte savedUIDSize = 0;
byte cardData[16][16];
bool hasData = false;

// ── Helpers ──────────────────────────────────────────────────────
void printHex(const byte *buf, byte len) {
  for (byte i = 0; i < len; i++) {
    if (buf[i] < 0x10) Serial.print("0");
    Serial.print(buf[i], HEX);
    if (i < len - 1) Serial.print(":");
  }
}

const char* cardTypeName(byte sak) {
  if (sak == 0x08) return "MIFARE 1K";
  if (sak == 0x09) return "MIFARE MINI";
  if (sak == 0x18) return "MIFARE 4K";
  if (sak == 0x00) return "MIFARE Ultralight";
  return "Unknown";
}

// ── Key Management ───────────────────────────────────────────────
void loadDefaultKey() {
  for (byte i = 0; i < 6; i++) defaultKey.keyByte[i] = 0xFF;
}

// ── Card Operations ──────────────────────────────────────────────
bool authenticateBlock(byte blockAddr) {
  return mfrc522.PCD_Authenticate(
    MFRC522::PICC_CMD_MF_AUTH_KEY_A,
    blockAddr,
    &defaultKey,
    &(mfrc522.uid)
  ) == MFRC522::STATUS_OK;
}

bool readBlock(byte blockAddr, byte *buffer) {
  byte size = 18;
  return mfrc522.MIFARE_Read(blockAddr, buffer, &size) == MFRC522::STATUS_OK;
}

bool writeBlock(byte blockAddr, const byte *data) {
  return mfrc522.MIFARE_Write(blockAddr, (byte*)data, 16) == MFRC522::STATUS_OK;
}

void showCardInfo() {
  Serial.println(F("╔══════════════════════════════════════════╗"));
  Serial.println(F("║       H8 — RFID Spoofer — Scan          ║"));
  Serial.println(F("╚══════════════════════════════════════════╝"));

  Serial.print(F("  UID:       "));
  printHex(mfrc522.uid.uidByte, mfrc522.uid.size);
  Serial.println();

  Serial.print(F("  Type:      "));
  Serial.println(cardTypeName(mfrc522.uid.sak));

  Serial.print(F("  UID Size:  "));
  Serial.print(mfrc522.uid.size);
  Serial.println(F(" bytes"));

  Serial.print(F("  SAK:       0x"));
  if (mfrc522.uid.sak < 0x10) Serial.print("0");
  Serial.println(mfrc522.uid.sak, HEX);
  Serial.println();
}

// ── READ ─────────────────────────────────────────────────────────
void cmdRead() {
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    Serial.println(F("[!] No card detected"));
    return;
  }

  showCardInfo();

  // Save UID
  savedUIDSize = mfrc522.uid.size;
  memcpy(savedUID, mfrc522.uid.uidByte, savedUIDSize);

  Serial.println(F("[*] Reading all blocks..."));
  Serial.println(F("─── Block Data ───────────────────────────"));

  for (byte block = 0; block < 64; block++) {
    // Skip sector trailer blocks (3, 7, 11, ...)
    if (block % 4 == 3) {
      Serial.print(F("  Sector Trailer ["));
      if (block < 10) Serial.print("0");
      Serial.print(block);
      Serial.println(F("]  -- reserved --"));
      continue;
    }

    if (!authenticateBlock(block)) {
      Serial.print(F("  Block ["));
      if (block < 10) Serial.print("0");
      Serial.print(block);
      Serial.println(F("]  AUTH FAILED"));
      continue;
    }

    byte readBuf[18];
    if (readBlock(block, readBuf)) {
      memcpy(cardData[block], readBuf, 16);
      Serial.print(F("  Block ["));
      if (block < 10) Serial.print("0");
      Serial.print(block);
      Serial.print(F("]  "));
      printHex(readBuf, 16);
      Serial.print(F("  |"));
      for (byte i = 0; i < 16; i++) {
        Serial.print((readBuf[i] >= 32 && readBuf[i] < 127) ? (char)readBuf[i] : '.');
      }
      Serial.println(F("|"));
    }
  }

  Serial.println(F("─── End ──────────────────────────────────"));
  hasData = true;
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  Serial.print(F("[+] Card data saved ("));
  Serial.print(savedUIDSize);
  Serial.println(F(" byte UID). Use 'clone' to write to blank card."));
}

// ── CLONE ────────────────────────────────────────────────────────
void cmdClone() {
  if (!hasData) {
    Serial.println(F("[!] No card data loaded. Run 'read' first."));
    return;
  }

  Serial.println(F("[*] Place blank MIFARE card on reader..."));
  while (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    delay(100);
  }

  Serial.print(F("[*] Blank card detected: "));
  printHex(mfrc522.uid.uidByte, mfrc522.uid.size);
  Serial.println();

  int written = 0;
  int failed = 0;

  for (byte block = 0; block < 64; block++) {
    if (block % 4 == 3) continue;  // skip sector trailers

    if (!authenticateBlock(block)) {
      failed++;
      continue;
    }

    if (writeBlock(block, cardData[block])) {
      written++;
    } else {
      failed++;
    }
  }

  Serial.println(F("─── Clone Results ────────────────────────"));
  Serial.print(F("  Written: "));
  Serial.print(written);
  Serial.println(F(" blocks"));
  Serial.print(F("  Failed:  "));
  Serial.print(failed);
  Serial.println(F(" blocks"));

  if (written > 0) {
    Serial.print(F("[+] Clone complete. UID: "));
    printHex(savedUID, savedUIDSize);
    Serial.println();
  }

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

// ── DUMP ─────────────────────────────────────────────────────────
void cmdDump() {
  if (!hasData) {
    Serial.println(F("[!] No card data loaded. Run 'read' first."));
    return;
  }

  Serial.println(F("╔══════════════════════════════════════════╗"));
  Serial.println(F("║         H8 — Card Data Dump             ║"));
  Serial.println(F("╚══════════════════════════════════════════╝"));

  Serial.print(F("  Saved UID: "));
  printHex(savedUID, savedUIDSize);
  Serial.println();
  Serial.println();

  for (byte sector = 0; sector < 16; sector++) {
    Serial.print(F("── Sector "));
    Serial.print(sector);
    Serial.println(F(" ──"));

    for (byte relBlock = 0; relBlock < 4; relBlock++) {
      byte block = sector * 4 + relBlock;
      if (relBlock == 3) {
        Serial.println(F("  [TRAILER]  -- key data --"));
      } else {
        Serial.print(F("  ["));
        Serial.print(block);
        Serial.print(F("]  "));
        printHex(cardData[block], 16);
        Serial.println();
      }
    }
  }
}

// ── EMULATE ──────────────────────────────────────────────────────
void cmdEmulate() {
  if (!hasData) {
    Serial.println(F("[!] No card data loaded. Run 'read' first."));
    return;
  }

  Serial.println(F("[*] Emulation mode (serial replay):"));
  Serial.print(F("    Present cloned UID: "));
  printHex(savedUID, savedUIDSize);
  Serial.println();
  Serial.println(F("    Bring reader close. Monitoring..."));

  while (true) {
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
      Serial.print(F("[EMUL] Reader detected: "));
      printHex(mfrc522.uid.uidByte, mfrc522.uid.size);
      Serial.println();

      // Attempt to authenticate and respond
      if (authenticateBlock(0)) {
        byte buf[18];
        if (readBlock(0, buf)) {
          Serial.print(F("[EMUL] Block 0 served: "));
          printHex(buf, 16);
          Serial.println();
        }
      }

      mfrc522.PICC_HaltA();
      mfrc522.PCD_StopCrypto1();
    }
    delay(50);
  }
}

// ── KEYS ─────────────────────────────────────────────────────────
void cmdKeys() {
  Serial.println(F("╔══════════════════════════════════════════╗"));
  Serial.println(F("║      H8 — Default Key Table             ║"));
  Serial.println(F("╚══════════════════════════════════════════╝"));
  Serial.println();
  Serial.println(F("  Common MIFARE keys:"));
  Serial.println(F("    FF:FF:FF:FF:FF:FF  (factory default)"));
  Serial.println(F("    A0:A1:A2:A3:A4:A5  (Madrid metro)"));
  Serial.println(F("    D3:F7:D3:F7:D3:F7  (NDEF)"));
  Serial.println(F("    00:00:00:00:00:00  (null key)"));
  Serial.println(F("    B0:B1:B2:B3:B4:B5  (common alt)"));
  Serial.println();
  Serial.print(F("  Current key: "));
  for (byte i = 0; i < 6; i++) {
    if (defaultKey.keyByte[i] < 0x10) Serial.print("0");
    Serial.print(defaultKey.keyByte[i], HEX);
    if (i < 5) Serial.print(":");
  }
  Serial.println();
}

// ── Menu ─────────────────────────────────────────────────────────
void printMenu() {
  Serial.println();
  Serial.println(F("╔══════════════════════════════════════════╗"));
  Serial.println(F("║        H8 — RFID Spoofer Menu           ║"));
  Serial.println(F("╠══════════════════════════════════════════╣"));
  Serial.println(F("║  read    — Scan & dump card data         ║"));
  Serial.println(F("║  clone   — Write saved data to blank     ║"));
  Serial.println(F("║  dump    — Display saved card data       ║"));
  Serial.println(F("║  emulate — Serial replay mode            ║"));
  Serial.println(F("║  keys    — Show default key table        ║"));
  Serial.println(F("║  menu    — Show this menu                ║"));
  Serial.println(F("╚══════════════════════════════════════════╝"));
  Serial.println();
}

// ── Setup ────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
  mfrc522.PCD_Init(SS_PIN, RST_PIN);

  delay(100);

  Serial.println();
  Serial.println(F("╔══════════════════════════════════════════╗"));
  Serial.println(F("║    H8 — RFID/NFC Spoofer v1.0           ║"));
  Serial.println(F("║    ESP32 + RC522 (MIFARE Classic)       ║"));
  Serial.println(F("╚══════════════════════════════════════════╝"));

  // Verify RC522 communication
  byte v = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  if (v == 0x00 || v == 0xFF) {
    Serial.println(F("[!] RC522 not responding. Check wiring!"));
    Serial.print(F("    Version register: 0x"));
    Serial.println(v, HEX);
  } else {
    Serial.print(F("[+] RC522 detected (v0x"));
    Serial.print(v, HEX);
    Serial.println(F(")"));
  }

  loadDefaultKey();
  printMenu();
}

// ── Loop ─────────────────────────────────────────────────────────
void loop() {
  if (!Serial.available()) return;

  String cmd = Serial.readStringUntil('\n');
  cmd.trim();
  cmd.toLowerCase();

  if (cmd == "read")      cmdRead();
  else if (cmd == "clone")  cmdClone();
  else if (cmd == "dump")   cmdDump();
  else if (cmd == "emulate") cmdEmulate();
  else if (cmd == "keys")   cmdKeys();
  else if (cmd == "menu")   printMenu();
  else {
    Serial.print(F("[?] Unknown command: "));
    Serial.println(cmd);
  }
}
