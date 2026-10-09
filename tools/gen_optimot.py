#!/usr/bin/env python3
"""Génère keyboards/zsa/moonlander/keymaps/optimot/optimot_tables.h à partir du pilote Linux d'Optimot.

Entrées :
  vendor/optimot/*.xkb       -> 6 niveaux par position (base, Maj, AltGr, AltGr+Maj, Ctrl, Ctrl+Maj)
  vendor/optimot/*.XCompose  -> séquences de touches mortes
  $KEYSYMDEF (keysymdef.h)   -> correspondance nom de keysym -> Unicode

Sortie : un en-tête C contenant uniquement des données (tables en flash).

Optimot (c) Patrick Jamet, https://optimot.fr/ — CC BY-NC-SA 4.0.
"""

import os
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
XKB = ROOT / "vendor/optimot/Optimot_Ergo_1.8.0.xkb"
XCOMPOSE = ROOT / "vendor/optimot/Optimot_Ergo_1.8.0.XCompose"
OUT = ROOT / "keyboards/zsa/moonlander/keymaps/optimot/optimot_tables.h"

# Positions xkb, dans l'ordre des index du moteur. Le keycode QMK est celui qu'Oryx
# envoie aujourd'hui pour cette position (QWERTY US + touche ISO <LSGT>).
POSITIONS = [
    ("TLDE", "KC_GRV"),
    ("AE01", "KC_1"), ("AE02", "KC_2"), ("AE03", "KC_3"), ("AE04", "KC_4"), ("AE05", "KC_5"),
    ("AE06", "KC_6"), ("AE07", "KC_7"), ("AE08", "KC_8"), ("AE09", "KC_9"), ("AE10", "KC_0"),
    ("AE11", "KC_MINS"), ("AE12", "KC_EQL"),
    ("AD01", "KC_Q"), ("AD02", "KC_W"), ("AD03", "KC_E"), ("AD04", "KC_R"), ("AD05", "KC_T"),
    ("AD06", "KC_Y"), ("AD07", "KC_U"), ("AD08", "KC_I"), ("AD09", "KC_O"), ("AD10", "KC_P"),
    ("AD11", "KC_LBRC"), ("AD12", "KC_RBRC"),
    ("AC01", "KC_A"), ("AC02", "KC_S"), ("AC03", "KC_D"), ("AC04", "KC_F"), ("AC05", "KC_G"),
    ("AC06", "KC_H"), ("AC07", "KC_J"), ("AC08", "KC_K"), ("AC09", "KC_L"), ("AC10", "KC_SCLN"),
    ("AC11", "KC_QUOT"), ("BKSL", "KC_BSLS"), ("LSGT", "KC_NUBS"),
    ("AB01", "KC_Z"), ("AB02", "KC_X"), ("AB03", "KC_C"), ("AB04", "KC_V"), ("AB05", "KC_B"),
    ("AB06", "KC_N"), ("AB07", "KC_M"), ("AB08", "KC_COMM"), ("AB09", "KC_DOT"), ("AB10", "KC_SLSH"),
    ("SPCE", "KC_SPC"),
]

TYPES = {
    "FOUR_LEVEL_CONTROL": "OPT_T_CONTROL",
    "FOUR_LEVEL_ALPHABETIC_CONTROL": "OPT_T_ALPHA",
    "FOUR_LEVEL_SEMIALPHABETIC_CONTROL": "OPT_T_SEMIALPHA",
}

# --- Hôte AZERTY (commun à xkb « fr » et à l'AZERTY Windows) -----------------------
# caractère -> (keycode HID QMK, Maj, AltGr, touche morte sous Windows)
AZ_MAIN = {
    "²": ("KC_GRV", 0, 0),
    "&": ("KC_1", 0, 0), "1": ("KC_1", 1, 0),
    "é": ("KC_2", 0, 0), "2": ("KC_2", 1, 0), "~": ("KC_2", 0, 1),
    '"': ("KC_3", 0, 0), "3": ("KC_3", 1, 0), "#": ("KC_3", 0, 1),
    "'": ("KC_4", 0, 0), "4": ("KC_4", 1, 0), "{": ("KC_4", 0, 1),
    "(": ("KC_5", 0, 0), "5": ("KC_5", 1, 0), "[": ("KC_5", 0, 1),
    "-": ("KC_6", 0, 0), "6": ("KC_6", 1, 0), "|": ("KC_6", 0, 1),
    "è": ("KC_7", 0, 0), "7": ("KC_7", 1, 0), "`": ("KC_7", 0, 1),
    "_": ("KC_8", 0, 0), "8": ("KC_8", 1, 0), "\\": ("KC_8", 0, 1),
    "ç": ("KC_9", 0, 0), "9": ("KC_9", 1, 0), "^": ("KC_9", 0, 1),
    "à": ("KC_0", 0, 0), "0": ("KC_0", 1, 0), "@": ("KC_0", 0, 1),
    ")": ("KC_MINS", 0, 0), "°": ("KC_MINS", 1, 0), "]": ("KC_MINS", 0, 1),
    "=": ("KC_EQL", 0, 0), "+": ("KC_EQL", 1, 0), "}": ("KC_EQL", 0, 1),
    "€": ("KC_E", 0, 1),
    "$": ("KC_RBRC", 0, 0), "£": ("KC_RBRC", 1, 0), "¤": ("KC_RBRC", 0, 1),
    "ù": ("KC_QUOT", 0, 0), "%": ("KC_QUOT", 1, 0),
    "*": ("KC_NUHS", 0, 0), "µ": ("KC_NUHS", 1, 0),
    "<": ("KC_NUBS", 0, 0), ">": ("KC_NUBS", 1, 0),
    ",": ("KC_M", 0, 0), "?": ("KC_M", 1, 0),
    ";": ("KC_COMM", 0, 0), ".": ("KC_COMM", 1, 0),
    ":": ("KC_DOT", 0, 0), "/": ("KC_DOT", 1, 0),
    "!": ("KC_SLSH", 0, 0), "§": ("KC_SLSH", 1, 0),
    " ": ("KC_SPC", 0, 0),
}
# Lettres : position physique AZERTY.
AZ_LETTERS = {
    "a": "KC_Q", "z": "KC_W", "e": "KC_E", "r": "KC_R", "t": "KC_T", "y": "KC_Y", "u": "KC_U",
    "i": "KC_I", "o": "KC_O", "p": "KC_P", "q": "KC_A", "s": "KC_S", "d": "KC_D", "f": "KC_F",
    "g": "KC_G", "h": "KC_H", "j": "KC_J", "k": "KC_K", "l": "KC_L", "m": "KC_SCLN",
    "w": "KC_Z", "x": "KC_X", "c": "KC_C", "v": "KC_V", "b": "KC_B", "n": "KC_N",
}
# Touches mortes AZERTY présentes sur les deux OS : ^ (KC_LBRC) et ¨ (Maj+KC_LBRC).
AZ_DEAD_CIRC = {"a": "â", "e": "ê", "i": "î", "o": "ô", "u": "û"}
AZ_DEAD_DIAE = {"a": "ä", "e": "ë", "i": "ï", "o": "ö", "u": "ü", "y": "ÿ"}
WIN_DEAD = {"~", "`"}  # AltGr+2 et AltGr+7 sont des touches mortes sous Windows



def die(msg):
    sys.exit(f"gen_optimot: {msg}")


def load_keysymdef():
    path = os.environ.get("KEYSYMDEF")
    if not path or not Path(path).exists():
        die("KEYSYMDEF doit pointer vers keysymdef.h (fourni par devenv)")
    table = {}
    rx = re.compile(r"^#define XK_(\w+)\s+0x([0-9a-fA-F]+)\s*(?:/\*\s*(?:\(?U\+([0-9A-Fa-f]+))?)?")
    for line in Path(path).read_text().splitlines():
        m = rx.match(line)
        if not m:
            continue
        name, val, uni = m.group(1), int(m.group(2), 16), m.group(3)
        if uni:
            table[name] = int(uni, 16)
        elif 0x20 <= val <= 0x7E or 0xA0 <= val <= 0xFF:
            table.setdefault(name, val)  # alias Latin-1 sans commentaire U+ (ex. guillemotleft)
    return table


class Keysyms:
    def __init__(self, ksdef):
        self.ksdef = ksdef
        self.names = ["NoSymbol"]
        self.index = {"NoSymbol": 0}

    def unicode(self, name):
        if re.fullmatch(r"U[0-9A-Fa-f]{4,6}", name):
            return int(name[1:], 16)
        if name.startswith("dead_"):
            return None
        if name in self.ksdef:
            return self.ksdef[name]
        if len(name) == 1:
            return ord(name)
        die(f"keysym inconnu : {name}")

    def id(self, name):
        if name not in self.index:
            self.index[name] = len(self.names)
            self.names.append(name)
        return self.index[name]


def parse_xkb(ks):
    text = XKB.read_text()
    rx = re.compile(r'key <(\w+)>\s*\{\s*type\[group1\]\s*=\s*"(\w+)",\s*\[([^\]]*)\]')
    keys = {}
    for m in rx.finditer(text):
        syms = [s.strip() for s in m.group(3).split(",")]
        if len(syms) != 6:
            die(f"{m.group(1)} : 6 niveaux attendus")
        keys[m.group(1)] = (TYPES[m.group(2)], syms)
    rows = []
    for pos, _ in POSITIONS:
        if pos not in keys:
            die(f"position absente du xkb : {pos}")
        typ, syms = keys[pos]
        rows.append((typ, [ks.id(s) for s in syms[:4]], syms[4], syms))
    return rows


def parse_compose(producible):
    rx = re.compile(r'^((?:<\w+>\s*)+):\s*"((?:[^"\\]|\\.)*)"')
    seqs = {}
    skipped = 0
    for line in XCOMPOSE.read_text().splitlines():
        m = rx.match(line)
        if not m:
            continue
        inputs = re.findall(r"<(\w+)>", m.group(1))
        out = re.sub(r"\\(.)", r"\1", m.group(2))
        if not all(i in producible for i in inputs):
            skipped += 1
            continue
        seqs[tuple(inputs)] = out
    return seqs, skipped


def build_trie(seqs, ks):
    root = {}
    for inputs, out in seqs.items():
        node = root
        for i, name in enumerate(inputs):
            last = i == len(inputs) - 1
            entry = node.setdefault(ks.index[name], [None, {}])
            if last:
                if entry[1]:
                    print(f"avertissement : préfixe et séquence complète {inputs}", file=sys.stderr)
                entry[0] = out
            else:
                if entry[0] is not None:
                    print(f"avertissement : séquence masquée {inputs}", file=sys.stderr)
                    entry[0] = None
                node = entry[1]
    # BFS : les enfants d'un nœud sont contigus et triés par keysym.
    flat = []
    queue = [0]
    flat.append([0, 0, 0, None, root])
    while queue:
        idx = queue.pop(0)
        children = flat[idx][4]
        if not children:
            continue
        flat[idx][1] = len(children)
        flat[idx][2] = len(flat)
        for k in sorted(children):
            out, sub = children[k]
            queue.append(len(flat))
            flat.append([k, 0, 0, out if not sub else None, sub])
    return flat


def utf16(s):
    units = []
    for ch in s:
        cp = ord(ch)
        if cp > 0xFFFF:
            cp -= 0x10000
            units += [0xD800 | (cp >> 10), 0xDC00 | (cp & 0x3FF)]
        else:
            units.append(cp)
    return units


def main():
    ksdef = load_keysymdef()
    ks = Keysyms(ksdef)
    rows = parse_xkb(ks)
    producible = set(ks.names[1:])
    seqs, skipped = parse_compose(producible)
    trie = build_trie(seqs, ks)
    dead_ids = sorted({k for k in trie[0][4]})

    # Unicode de chaque keysym (0 pour un dead_* : sa forme isolée vient du trie, <dead> <space>).
    ks_uni = [0] + [ks.unicode(n) or 0 for n in ks.names[1:]]

    # Pool de sorties (UTF-16, préfixé par la longueur), dédupliqué.
    pool, offsets = [], {}

    def intern(s):
        if s not in offsets:
            u = utf16(s)
            if len(u) > 255:
                die("sortie trop longue")
            offsets[s] = len(pool)
            pool.extend([len(u)] + u)
        return offsets[s]

    for n in trie:
        if n[3] is not None:
            n[2] = intern(n[3])
    if len(trie) > 0xFFFF or len(pool) > 0xFFFF:
        die("trie trop grand pour des index 16 bits")

    # Table hôte AZERTY pour tous les points de code BMP utiles.
    needed = {c for c in ks_uni if c} | {ord(ch) for s in seqs.values() for ch in s}
    az = {}
    for ch, (kc, sh, ag) in AZ_MAIN.items():
        az[ord(ch)] = (kc, sh, ag, 1 if ch in WIN_DEAD else 0, 0)
    for ch, kc in AZ_LETTERS.items():
        az[ord(ch)] = (kc, 0, 0, 0, 0)
        az[ord(ch.upper())] = (kc, 1, 0, 0, 0)
    for base, ch in AZ_DEAD_CIRC.items():
        az[ord(ch)] = (AZ_LETTERS[base], 0, 0, 0, 1)
        az[ord(ch.upper())] = (AZ_LETTERS[base], 1, 0, 0, 1)
    for base, ch in AZ_DEAD_DIAE.items():
        az[ord(ch)] = (AZ_LETTERS[base], 0, 0, 0, 2)
        if base != "y":
            az[ord(ch.upper())] = (AZ_LETTERS[base], 1, 0, 0, 2)
    az = {cp: v for cp, v in az.items() if cp in needed or cp < 0x80}

    # Raccourcis Ctrl par position.
    ctrl = []
    for typ, ids, l5, syms in rows:
        if len(l5) == 1 and l5.isalpha():
            ctrl.append(AZ_LETTERS[l5.lower()])
        elif len(l5) == 1 and l5.isdigit():
            ctrl.append("KC_0" if l5 == "0" else f"KC_{l5}")
        elif chr(ks.unicode(l5)) in AZ_MAIN:
            # Symbole : la touche AZERTY qui le porte, sans ses modificateurs.
            ctrl.append(AZ_MAIN[chr(ks.unicode(l5))][0])
        else:
            die(f"raccourci Ctrl non géré : {l5}")
        if syms[5] != l5:
            die("niveau 6 différent du niveau 5 : non géré")

    # Majuscules automatiques (Caps Word) : positions dont le niveau 1 est une lettre.
    letter = [1 if ks_uni[ids[0]] and chr(ks_uni[ids[0]]).isalpha() else 0 for _, ids, _, _ in rows]

    w = []
    w.append("// Généré par tools/gen_optimot.py — ne pas modifier à la main.")
    w.append("// Optimot Ergo 1.8.0 (c) Patrick Jamet, https://optimot.fr/ — CC BY-NC-SA 4.0.")
    w.append("#pragma once\n")
    w.append(f"#define OPT_NPOS {len(POSITIONS)}")
    w.append(f"#define OPT_NKEYSYM {len(ks.names)}")
    w.append(f"#define OPT_KS_SPACE {ks.index['space']}")
    w.append(f"#define OPT_TRIE_NODES {len(trie)}")
    w.append(f"#define OPT_MAX_SEQ {max(len(k) for k in seqs)}")
    w.append(f"#define OPT_AZ_COUNT {len(az)}\n")
    w.append("#ifdef OPT_TABLES_IMPLEMENTATION\n")
    w.append("static const uint16_t opt_pos_keycode[OPT_NPOS] = {")
    w.append("    " + ", ".join(kc for _, kc in POSITIONS) + ",\n};\n")
    w.append("static const uint8_t opt_pos_type[OPT_NPOS] = {")
    w.append("    " + ", ".join(r[0] for r in rows) + ",\n};\n")
    w.append("// Niveaux 1 à 4 (base, Maj, AltGr, AltGr+Maj) -> id de keysym.")
    w.append("static const uint8_t opt_pos_levels[OPT_NPOS][4] = {")
    for (pos, _), (_, ids, _, syms) in zip(POSITIONS, rows):
        w.append(f"    {{{', '.join(f'{i:3d}' for i in ids)}}}, // {pos}: {' '.join(syms[:4])}")
    w.append("};\n")
    w.append("// Niveaux 5/6 (Ctrl) : keycode AZERTY du raccourci.")
    w.append("static const uint8_t opt_pos_ctrl[OPT_NPOS] = {")
    w.append("    " + ", ".join(ctrl) + ",\n};\n")
    w.append("static const uint8_t opt_pos_letter[OPT_NPOS] = {")
    w.append("    " + ", ".join(map(str, letter)) + ",\n};\n")
    w.append("// Unicode de chaque keysym (0 : touche morte sans forme propre).")
    w.append("static const uint32_t opt_ks_unicode[OPT_NKEYSYM] = {")
    for i, n in enumerate(ks.names):
        w.append(f"    0x{ks_uni[i]:05X}, // {i}: {n}")
    w.append("};\n")
    w.append("// Trie des séquences de composition (BFS, enfants contigus triés par keysym).")
    w.append("// Nœud interne : val = premier enfant. Feuille (nchild == 0) : val = offset dans opt_out_pool.")
    for name, col in (("opt_trie_ks", 0), ("opt_trie_nchild", 1)):
        w.append(f"static const uint8_t {name}[OPT_TRIE_NODES] = {{")
        vals = [str(n[col]) for n in trie]
        for i in range(0, len(vals), 32):
            w.append("    " + ",".join(vals[i:i + 32]) + ",")
        w.append("};\n")
    w.append("static const uint16_t opt_trie_val[OPT_TRIE_NODES] = {")
    vals = [str(n[2]) for n in trie]
    for i in range(0, len(vals), 24):
        w.append("    " + ",".join(vals[i:i + 24]) + ",")
    w.append("};\n")
    w.append("// Sorties : [longueur, unités UTF-16...]")
    w.append(f"static const uint16_t opt_out_pool[{len(pool)}] = {{")
    for i in range(0, len(pool), 24):
        w.append("    " + ",".join(str(u) for u in pool[i:i + 24]) + ",")
    w.append("};\n")
    w.append("#endif // OPT_TABLES_IMPLEMENTATION\n")
    w.append("typedef struct { uint16_t cp; uint8_t kc; uint8_t flags; } opt_az_t;\n")
    w.append("#ifdef OPT_AZ_IMPLEMENTATION\n")
    w.append("// Hôte AZERTY : point de code -> frappe. flags : bit0 Maj, bit1 AltGr,")
    w.append("// bit2 touche morte sous Windows (suivre d'Espace), bits3-4 préfixe mort (1 = ^, 2 = ¨).")
    w.append("static const opt_az_t opt_az[OPT_AZ_COUNT] = {")
    for cp in sorted(az):
        kc, sh, ag, wd, dead = az[cp]
        flags = sh | ag << 1 | wd << 2 | dead << 3
        w.append(f"    {{0x{cp:04X}, {kc}, {flags}}}, // {chr(cp)!r}")
    w.append("};\n")
    w.append("#endif // OPT_AZ_IMPLEMENTATION")
    OUT.write_text("\n".join(w) + "\n")

    trie_bytes = len(trie) * 4
    print(f"keysyms : {len(ks.names)}  séquences : {len(seqs)} (ignorées : {skipped})")
    print(f"préfixes morts : {', '.join(ks.names[i] for i in dead_ids)}")
    print(f"trie : {len(trie)} nœuds = {trie_bytes} o ; pool : {len(pool) * 2} o ; AZERTY : {len(az) * 4} o")
    print(f"total ≈ {trie_bytes + len(pool) * 2 + len(az) * 4 + len(ks.names) * 4} o")


if __name__ == "__main__":
    main()
