#!/usr/bin/env python3
"""Produit build/vectors.h : cas de test du moteur, tirés directement du xkb et du XCompose.

- Pour chaque position et chaque niveau 1-4 : le point de code attendu (0 pour une touche morte).
- Pour chaque séquence du XCompose : les (position, niveau) à taper et la sortie attendue.
"""

import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "tools"))
import gen_optimot as g  # noqa: E402

OUT = g.ROOT / "build/vectors.h"


def c_str32(s):
    return "{" + ", ".join(f"0x{ord(c):X}" for c in s) + ", 0}"


def main():
    ks = g.Keysyms(g.load_keysymdef())
    rows = g.parse_xkb(ks)
    seqs, _ = g.parse_compose(set(ks.names[1:]))

    # keysym -> première (position, niveau) qui le produit
    where = {}
    for pos, (_, ids, _, syms) in enumerate(rows):
        for lvl, name in enumerate(syms[:4]):
            where.setdefault(name, (pos, lvl))

    w = ["// Généré par tests/gen_vectors.py", "#pragma once", ""]
    w.append("typedef struct { uint8_t pos, level; uint32_t cp; } level_case_t;")
    w.append("static const level_case_t level_cases[] = {")
    for pos, (_, ids, _, syms) in enumerate(rows):
        for lvl, name in enumerate(syms[:4]):
            w.append(f"    {{{pos}, {lvl}, 0x{ks.unicode(name) or 0:X}}}, // {name}")
    w.append("};\n")

    maxlen = max(len(k) for k in seqs)
    w.append(f"#define SEQ_MAX {maxlen}")
    w.append("typedef struct { uint8_t n; uint8_t pos[SEQ_MAX]; uint8_t level[SEQ_MAX]; uint32_t out[8]; } seq_case_t;")
    w.append("static const seq_case_t seq_cases[] = {")
    for inputs, out in seqs.items():
        if len(out) > 7:
            continue
        pl = [where[i] for i in inputs]
        pos = ", ".join(str(p) for p, _ in pl)
        lvl = ", ".join(str(l) for _, l in pl)
        w.append(f"    {{{len(pl)}, {{{pos}}}, {{{lvl}}}, {c_str32(out)}}},")
    w.append("};")
    OUT.parent.mkdir(exist_ok=True)
    OUT.write_text("\n".join(w) + "\n")


if __name__ == "__main__":
    main()
