#!/usr/bin/env python3
"""AssertProofs — the seated-ink gate over every VisualProof shot.

Usage: python3 Tools/AssertProofs.py
Every assertion below was verified by hand against the reference sheets; the gate
fails loudly if a panel drifts from its seated ink.
"""

import sys

sys.path.insert(0, "Tools")
from ProofProbe import Decode, Sample

GATES = [
    # ① OutlinerHost — the standalone scene directory.
    ("VisualProof/OutlinerHost/directory.png", [
        (200, 250, 0x17171A, 8, "directory panel ground"),
        (37, 48, 0x000000, 6, "head black tile"),
        (250, 316, 0x232327, 8, "taken row ground (--row-sel)"),
        (87, 316, 0x4A90E2, 45, "taken rail (--accent)"),
        (200, 727, 0x101012, 6, "directory foot"),
    ]),
    # ② OutlinerHost — additive selection under control.
    ("VisualProof/OutlinerHost/multiselect.png", [
        (87, 316, 0x4A90E2, 45, "first taken rail"),
        (72, 412, 0x4A90E2, 45, "second taken rail"),
        (72, 444, 0x4A90E2, 45, "third taken rail"),
    ]),
    # ③ OutlinerHost — retention run `sk`.
    ("VisualProof/OutlinerHost/filter.png", [
        (150, 188, 0x232327, 8, "retained SK_BasePlate taken row"),
        (150, 250, 0x17171A, 8, "unretained Bodies row vacant"),
    ]),
    # ④ PanelValidationHost — texture paint, layers.
    ("VisualProof/PanelValidationHost/texturepaint-layers.png", [
        (200, 160, 0x121214, 8, "stack head"),
        (700, 205, 0x141414, 8, "add layer ground"),
        (1000, 160, 0x0E0E0E, 8, "channel head"),
        (1500, 350, 0x0E0E0E, 8, "chips region"),
    ]),
    # ⑤ PanelValidationHost — texture paint, mask.
    ("VisualProof/PanelValidationHost/texturepaint-mask.png", [
        (1000, 160, 0x0E0E0E, 8, "mask head"),
        (798, 400, 0x1C1C1C, 12, "section hair edge"),
        (1200, 840, 0x0E0E0E, 8, "mask foot"),
    ]),
    # ⑥ PanelValidationHost — texture paint, reorder drag.
    ("VisualProof/PanelValidationHost/texturepaint-reorder.png", [
        (400, 347, 0x4A90E2, 14, "insertion rail"),
    ]),
    # ⑦ PanelValidationHost — CAD drafting, properties page.
    ("VisualProof/PanelValidationHost/cad-properties.png", [
        (200, 160, 0x17171A, 8, "directory column ground"),
        (500, 160, 0x101012, 8, "properties bar column"),
        (1046, 217, 0x4A90E2, 16, "carousel properties underline"),
        (900, 263, 0x0A0A0B, 10, "record card ground"),
    ]),
    # ⑧ PanelValidationHost — CAD drafting, history page.
    ("VisualProof/PanelValidationHost/cad-history.png", [
        (1394, 217, 0x4A90E2, 16, "carousel history underline"),
        (889, 270, 0x4FD18B, 16, "cylinder revision bubble"),
    ]),
]


def Main():
    Failures = 0
    Total = 0
    for Path, Probes in GATES:
        Width, Height, Pixels = Decode(Path)
        for Along, Across, Expected, Tolerance, Label in Probes:
            Total += 1
            Red, Green, Blue, _ = Sample(Pixels, Width, Along, Across)
            Passed = (abs(Red - ((Expected >> 16) & 0xFF)) <= Tolerance and
                      abs(Green - ((Expected >> 8) & 0xFF)) <= Tolerance and
                      abs(Blue - (Expected & 0xFF)) <= Tolerance)
            if not Passed:
                Failures += 1
                Got = (Red << 16) | (Green << 8) | Blue
                print(f"  [FAIL] {Path} ({Along},{Across}) {Label}: want #{Expected:06x} got #{Got:06x}")
    print(f"AssertProofs: {Total - Failures}/{Total} seated inks stand")
    raise SystemExit(1 if Failures else 0)


if __name__ == "__main__":
    Main()
