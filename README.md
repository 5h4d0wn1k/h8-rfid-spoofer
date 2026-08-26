# H8 — RFID/NFC Spoofer

MIFARE Classic RFID reader, cloner, and emulator using ESP32 + RC522.

## Overview

This project implements a standalone RFID/NFC tool that:
- Reads MIFARE Classic 1K/4K card UIDs and block data
- Dumps full sector contents with key management
- Clones card data to blank MIFARE tags
- Provides serial replay emulation mode
- Supports all 16 sectors with sector trailer awareness

## Hardware

| Component | Connection | Role |
|-----------|------------|------|
| ESP32 NodeMCU | Main board | SPI master, serial interface |
| RC522 (MFRC522) | HSPI (D5/D18/D19/D23) | 13.56 MHz RFID reader |

## Wiring

```
RC522 Module (HSPI):
  SDA  → D5  (GPIO5)
  SCK  → D18 (GPIO18) [default HSPI]
  MOSI → D23 (GPIO23) [default HSPI]
  MISO → D19 (GPIO19) [default HSPI]
  RST  → D27 (GPIO27)
  3.3V → 3.3V
  GND  → GND
```

## Serial Commands

| Command | Description |
|---------|-------------|
| `read` | Scan and dump card data |
| `clone` | Write saved data to blank card |
| `dump` | Display saved card data |
| `emulate` | Serial replay mode |
| `keys` | Show default key table |

## Serial Output

```
=== H8 — RFID Spoofer ===
RC522 detected (v0x92)

[+] Scanning...
╔══════════════════════════════════════════╗
║       H8 — RFID Spoofer — Scan          ║
╚══════════════════════════════════════════╝
  UID:       A1:B2:C3:D4
  Type:      MIFARE 1K
```

## Build & Flash

```bash
# Using Arduino CLI
arduino-cli compile --fqbn esp32:esp32:esp32 firmware/h8_rfid_spoofer.ino
arduino-cli upload --fqbn esp32:esp32:esp32 --port /dev/ttyUSB0 firmware/h8_rfid_spoofer.ino
```

## Legal Disclaimer

**IMPORTANT: Read before use.**

This project is provided for **educational and authorized security testing purposes only**.

### Authorization Requirements
- You MUST have explicit written permission from the card/system owner before using this tool
- Unauthorized access to RFID-protected systems is illegal under federal and state laws
- This tool should ONLY be used on systems you own or have written authorization to test

### Legal Framework
- **Computer Fraud and Abuse Act (CFAA)**: Unauthorized access to computer systems is a federal crime
- **Wiretap Act (18 U.S.C. § 2511)**: Interception of electronic communications without consent is illegal
- **State Laws**: Many states have additional computer crime and wiretapping statutes
- **GDPR/CCPA**: Data collection may be subject to privacy regulations

### Acceptable Use
- Testing security of your own RFID systems
- Authorized penetration testing with written scope
- Academic research in controlled lab environments
- Security education and training

### Prohibited Use
- Cloning access cards without authorization
- Bypassing physical security controls illegally
- Any activity that violates applicable laws or regulations
- Commercial use without proper licensing

### No Warranty
This software is provided "AS IS" without warranty of any kind. The author is not responsible for any misuse or damage caused by this software.

### Responsible Disclosure
If you discover vulnerabilities using this tool, follow responsible disclosure practices:
1. Report to the vendor/owner privately
2. Allow reasonable time for remediation
3. Do not exploit beyond proof of concept

## License

MIT
