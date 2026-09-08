#!/usr/bin/env python3
"""H8 - RFID/NFC Spoofer host helper: SIMULATION-ONLY card-dump formatting.
Cloning/emulation is never triggered from this script.
Educational/authorized own-lab use only (see README "IMPORTANT").
"""
import argparse
import os
import sys

MOD = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, MOD)
from hw_common import DEMO_TAG, read_target


def parse_uid(line):
    line = line.strip()
    if ":" not in line:
        return None
    label, _, uid = line.partition(":")
    try:
        b = bytes.fromhex(uid.strip().replace(":", ""))
    except ValueError:
        return None
    return {"label": label.strip(), "uid": b, "hex": uid.strip().upper()}


def analyze(text):
    return [u for l in text.splitlines() if (u := parse_uid(l))]


def emulate_sim(uid):
    return "SIM-EMULATION of UID %s (no RF field generated)" % ":".join(
        "%02X" % b for b in uid)


def run_demo():
    print("=== H8 RFID dump analysis (SIMULATION ONLY) ===")
    for u in analyze(read_target(
            "fixtures/cards.txt",
            "card0: 04:5A:67:1B\ncard1: 13:7D:00:2F\n")):
        print("  %-6s UID=%s size=%d bytes" % (u["label"], u["hex"], len(u["uid"])))
        print("    " + emulate_sim(u["uid"]))
    print(DEMO_TAG)
    return 0


def main(argv=None):
    p = argparse.ArgumentParser(
        description="H8 RFID spoofer - simulation-only card analysis")
    p.add_argument("--demo", action="store_true", help="offline demo (exit 0)")
    p.add_argument("--file", help="card dump path")
    args = p.parse_args(argv)
    if args.demo or not args.file:
        return run_demo()
    for u in analyze(open(args.file).read()):
        print("%s UID=%s" % (u["label"], u["hex"]))
    return 0


if __name__ == "__main__":
    sys.exit(main())
