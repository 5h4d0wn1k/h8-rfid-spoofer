# RFID/NFC Spoofer Firmware

## Purpose

Read and analyze MIFARE Classic cards you own; card clone/emulation is SIMULATION ONLY and requires explicit lab authorization.

## Board

- **Board**: ESP32 NodeMCU + MFRC522 (RC522)
- **FQBN**: `esp32:esp32:esp32`
- **Sketch**: `h8_rfid_spoofer/h8_rfid_spoofer.ino`

## Wiring

```
MFRC522: SDA->GPIO5(SS), RST->GPIO27, MOSI->GPIO23, MISO->GPIO19, SCK->GPIO18, VCC->3V3, GND->GND
```

## Build

```bash
arduino-cli compile --fqbn esp32:esp32:esp32 firmware/h8_rfid_spoofer
# upload (example, ESP32-C6):
# arduino-cli upload --fqbn esp32:esp32:esp32 --port /dev/ttyACM0 firmware/h8_rfid_spoofer
```

## Runtime

See the root README "IMPORTANT" section before powering on. This firmware is
for authorized own-lab study. Serial console exposes the interactive command
set described in the root README. All identifiers in the sketch are
placeholders (`lab-*` SSIDs, `00:11:22:33:44:55`, RFC 5737 / example.com).
