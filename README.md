> **⚠️ EDUCATIONAL USE ONLY — AUTHORIZED TESTING ONLY.**
> This project exists for education, research, and **defense of systems you own
> or hold explicit written authorization to assess**. Unauthorized use is
> prohibited and may be illegal. Read [ETHICS.md](ETHICS.md) and
> [SCOPE.md](SCOPE.md) before use. Use at your own risk; **AS IS**, no warranty.

# H8 — RFID/NFC Spoofer

**RFID emulation and cloning toolkit** by **5h4d0wn1k** for **physical security
research** on cards you own: a MIFARE Classic 1K/4K reader, cloner and
emulator on an ESP32 + RC522 (MFRC522), plus a simulation-only host helper and
card fixtures for offline study. Bench-scoped — never aimed at access systems
or cards you don't own.

## Why study RFID emulation

MIFARE Classic cards are everywhere — office doors, lockers, campus passes —
and their generations-proven cryptographic weaknesses are the canonical
"real-world crypto failure" lesson. This project teaches the physical layer:
how the RC522 authenticates to a card with key A/B, how sectors and trailers
are organized, and how a dump maps to a clone. The ESP32 firmware reads,
dumps and clones MIFARE data over SPI; the host helper formats card dumps and
simulates emulation offline with no RF field generated. All experiments stay
on your own bench hardware. See [ETHICS.md](ETHICS.md) and [SCOPE.md](SCOPE.md).

## Features

- **MIFARE Classic reading** — authenticate with key A/B, read UIDs and block
  data for 1K/4K/MINI/Ultralight tag types (`firmware/h8_rfid_spoofer/`).
- **Full sector dump** — all 16 sectors with sector-trailer awareness and
  key management.
- **Cloning** — write saved data to blank MIFARE tags.
- **Serial replay emulation** — `emulate` command streams a saved dump in
  replay mode.
- **Serial command interface** — `read`, `clone`, `dump`, `emulate`, `keys`
  over the ESP32 serial console.
- **Simulation-only host helper** — `python3 host/h8_cli.py --demo` formats
  dump files and simulates UID emulation offline (no RF generated), exit `0`.
- **Card fixtures** — offline card-dump corpus in `fixtures/cards.txt`.

## Quickstart

### Firmware (ESP32 + RC522)

Prerequisites: ESP32 NodeMCU, MFRC522 (RC522) module, and the MFRC522 Arduino
library.

```bash
arduino-cli compile --fqbn esp32:esp32:esp32 firmware/h8_rfid_spoofer
arduino-cli upload --fqbn esp32:esp32:esp32 --port /dev/ttyUSB0 firmware/h8_rfid_spoofer
```

**Wiring (HSPI):** RC522 `SDA→D5`, `SCK→D18`, `MOSI→D23`, `MISO→D19`,
`RST→D27`, `3.3V→3.3V`, `GND→GND`.

### Host helper (offline)

```bash
# Format bundled card dumps and simulate emulation, exit 0
python3 host/h8_cli.py --demo

# Analyze your own dump file
python3 host/h8_cli.py --file fixtures/cards.txt

# Run the offline test suite
python3 -m unittest discover -s tests
```

## Project structure

```
firmware/h8_rfid_spoofer/h8_rfid_spoofer.ino   # ESP32 + RC522 firmware
host/h8_cli.py                                 # simulation-only card-dump helper
host/hw_common.py                              # shared hardware helpers
fixtures/cards.txt                             # offline card-dump corpus
tests/                                         # unittest coverage
```

## Documentation

- [firmware README](firmware/README.md) — build and bench details.
- [ETHICS.md](ETHICS.md) — acceptable and prohibited use.
- [SCOPE.md](SCOPE.md) — authorized target scope and shielded-lab rules.
- [SECURITY.md](SECURITY.md) — responsible disclosure.

## Contributing

New card-family support, key search strategies and dump fixtures are welcome.
Open an issue or PR against the default branch; keep contributions scoped to
bench and simulation tooling.

## License

MIT — full legal shield in [LICENSE](LICENSE). Educational, authorization-
required software for studying RFID on cards and systems you own in an
isolated lab.