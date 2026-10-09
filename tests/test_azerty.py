#!/usr/bin/env python3
"""Vérifie la table hôte AZERTY générée contre la disposition xkb « fr » (xkbcli how-to-type).

Chaque frappe directe (sans touche morte) doit apparaître comme accès direct au
caractère, sur la même touche et au même niveau.
"""

import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
TABLES = ROOT / "keyboards/zsa/moonlander/keymaps/optimot/optimot_tables.h"

HID_TO_XKB = {
    "KC_GRV": "TLDE", "KC_1": "AE01", "KC_2": "AE02", "KC_3": "AE03", "KC_4": "AE04", "KC_5": "AE05",
    "KC_6": "AE06", "KC_7": "AE07", "KC_8": "AE08", "KC_9": "AE09", "KC_0": "AE10", "KC_MINS": "AE11",
    "KC_EQL": "AE12", "KC_Q": "AD01", "KC_W": "AD02", "KC_E": "AD03", "KC_R": "AD04", "KC_T": "AD05",
    "KC_Y": "AD06", "KC_U": "AD07", "KC_I": "AD08", "KC_O": "AD09", "KC_P": "AD10", "KC_LBRC": "AD11",
    "KC_RBRC": "AD12", "KC_A": "AC01", "KC_S": "AC02", "KC_D": "AC03", "KC_F": "AC04", "KC_G": "AC05",
    "KC_H": "AC06", "KC_J": "AC07", "KC_K": "AC08", "KC_L": "AC09", "KC_SCLN": "AC10", "KC_QUOT": "AC11",
    "KC_NUHS": "BKSL", "KC_NUBS": "LSGT", "KC_Z": "AB01", "KC_X": "AB02", "KC_C": "AB03", "KC_V": "AB04",
    "KC_B": "AB05", "KC_N": "AB06", "KC_M": "AB07", "KC_COMM": "AB08", "KC_DOT": "AB09", "KC_SLSH": "AB10",
    "KC_SPC": "SPCE",
}


def direct_access(ch):
    out = subprocess.run(["xkbcli", "how-to-type", "--layout", "fr", ch], capture_output=True, text=True, check=True).stdout
    section = out.split("=== Direct access ===")[1].split("===")[0]
    found = set()
    for line in section.splitlines():
        m = re.match(r"\s*\d+\s+(\w+)\s+\d+\s+.*?\s(\d+)\s+\[", line)
        if m:
            found.add((m.group(1), int(m.group(2))))
    return found


def main():
    rx = re.compile(r"\{0x([0-9A-F]+), (KC_\w+), (\d+)\}")
    entries = [(int(cp, 16), kc, int(f)) for cp, kc, f in rx.findall(TABLES.read_text())]
    failures = 0
    checked = 0
    for cp, kc, flags in entries:
        if flags >> 3:  # passe par une touche morte AZERTY
            continue
        level = 1 + (flags & 1) + (2 if flags & 2 else 0)
        key = HID_TO_XKB[kc]
        found = direct_access(chr(cp))
        checked += 1
        if (key, level) not in found:
            failures += 1
            print(f"ÉCHEC : {chr(cp)!r} attendu sur {key} niveau {level}, xkb : {sorted(found)}")
    print(f"AZERTY : {checked} frappes directes vérifiées : {'ÉCHEC' if failures else 'OK'} ({failures} échec(s))")
    sys.exit(1 if failures else 0)


if __name__ == "__main__":
    main()
