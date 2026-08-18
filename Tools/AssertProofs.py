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
    # ① OutlinerHost — world editor.
    ("VisualProof/OutlinerHost/outliner-world-editor.png", [
        (750, 18, 0x101012, 6, "top bar ground"),
        (110, 200, 0x101012, 6, "options menu ground"),
        (900, 300, 0x17171A, 8, "outliner panel ground"),
        (848, 210, 0x3B82F6, 12, "taken row rail"),
        (866, 192, 0x181F38, 8, "taken row tint"),
        (900, 846, 0x101012, 6, "outliner foot"),
        (700, 400, 0x0A0A0B, 16, "viewport desk"),
    ]),
    # ② OutlinerHost — retention run `light`.
    ("VisualProof/OutlinerHost/outliner-outliner-run.png", [
        (900, 260, 0x17171A, 8, "unretained row vacant"),
        (848, 210, 0x3B82F6, 12, "retained row rail"),
    ]),
    # ③ OutlinerHost — inspector slide.
    ("VisualProof/OutlinerHost/outliner-inspector.png", [
        (1000, 100, 0x101012, 8, "inspector head"),
        (900, 300, 0x0A0A0B, 10, "transform card"),
        (1316, 322, 0xF4F4F5, 25, "intensity knob"),
        (1020, 322, 0x8A8A8E, 20, "intensity fill"),
        (1400, 322, 0x2F2F33, 12, "intensity track"),
    ]),
    # ④ PanelValidationHost — texture paint, layers.
    ("VisualProof/PanelValidationHost/validation-texturepaint-layers.png", [
        (200, 160, 0x121214, 8, "stack head"),
        (700, 205, 0x141414, 8, "add layer ground"),
        (1000, 160, 0x0E0E0E, 8, "channel head"),
        (1500, 350, 0x0E0E0E, 8, "chips region"),
        (790, 300, 0x000000, 8, "channel desk"),
    ]),
    # ⑤ PanelValidationHost — texture paint, mask.
    ("VisualProof/PanelValidationHost/validation-texturepaint-mask.png", [
        (1000, 160, 0x0E0E0E, 8, "mask head"),
        (798, 400, 0x1C1C1C, 12, "section hair edge"),
        (1200, 840, 0x0E0E0E, 8, "mask foot"),
    ]),
    # ⑥ PanelValidationHost — CAD workspace.
    ("VisualProof/PanelValidationHost/validation-cad-workspace.png", [
        (800, 19, 0x050505, 6, "tab bar"),
        (600, 60, 0x070707, 6, "part header"),
        (160, 200, 0x0A0A0A, 6, "left dock"),
        (700, 500, 0x050608, 8, "stage ground"),
        (800, 885, 0x0E0E0E, 6, "condition strip"),
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
