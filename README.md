# RIFT — reference panels transcribed to C++ (Dear ImGui)

This repository holds the HTML → C++ transcription of the Slate reference prototypes, built standalone on the
vendored Dear ImGui (docking) that `Slate` pins — `ocornut/imgui@83f668625d4564de71d385aeb6a5dd04bee02e`,
byte-identical to `ExternalPackages/imgui` in the Slate repository.

No new HTML was written. Every panel below is C++ presented through a `RecordingSurface` seam, in the Slate
folder grammar (`Engine/SlateUI/Interface/<Unit>/{Api,Source}`, `Deliver<>`/`Refusal` contract, SKILL-Naming /
SKILL-Formatting throughout).

## What was transcribed

| Reference (`Slate@f19e4ff`) | C++ unit | Host |
|---|---|---|
| `References/remix-remix-global-ui/components/GameOutliner.tsx` | `Engine/SlateUI/Interface/OutlinerPanel` — general-purpose, declaration-driven outliner + entry inspector | `OutlinerHost` (standalone) |
| `References/remix-remix-global-ui/components/TexturePaint.tsx` | `Engine/SlateUI/Interface/TexturePaintPanel` — `LayersPane`, `ChannelPropertyPanel`, `MaskPropertyPanel` | `PanelValidationHost` |
| `References/remix-remix-global-ui/app/globals.css` + `components/Controls.tsx` | `Engine/SlateUI/Interface/ThemeSpecification` (the control-panel theme tokens) + `ControlPanel` (the widget kit) | both |
| `References/Cad` (Frontier CAD workspace) | `Engine/SlateUI/Interface/CadPanel` — tab strip, header, browser dock, model stage, inspector dock | `PanelValidationHost` |

The reference folders themselves are vendored unmodified under `References/` as the comparison of record.

## The two executables

```bash
make                # builds Build/OutlinerHost and Build/PanelValidationHost
make proof          # runs both headlessly and encodes VisualProof/*.png
```

- **`OutlinerHost`** — the standalone general-purpose outliner, seated in the reference's world-editor
  composition (top bar, options menu, weave-lattice viewport, docked inspector slide). Runs three scripted
  states: `world-editor`, `outliner-run` (retention filter `light`), `inspector` (entry components).
- **`OutlinerWindowHost`** (`make outliner-window`) — the interactive variant: the same seat behind a GLFW
  window (Tab summons the inspector slide, double-press inspects). Requires local GLFW + OpenGL dev packages;
  the headless build never compiles it. One composition (`WorldEditorSeat`), two hosts.
- **`PanelValidationHost`** — the validation: runs the texture-paint panels (`texturepaint-layers`,
  `texturepaint-mask`, `texturepaint-reorder` — a scripted live drag under a real pointer press) and the
  CAD workspace (`cad-workspace`) exactly as transcribed, for side-by-side comparison. Layer-stack
  drag-to-reorder is fully interactive: press the top row of a card, travel, drop — the dragged card
  ghosts at opacity-40, the drop target carries the marker rail, the stack splices on release.

All three hosts share the composition and the seam; the two proof hosts render headlessly: `RasterCodec` translates the recorded ImGui draw data into pixels in software
(no window, no GPU, no display server), dumps a marked raw frame, and `Tools/EncodeProof.py` encodes the PNG
(stdlib only). `Tools/ProofProbe.py` reads the proofs back and asserts the seated inks.

## Constraints honoured

- **Default ImGui typeface** — the embedded default (vector variant) at 13/11/10 px; no font files, no custom
  atlas. The reference's `·` and `°` runs are drawn stand-ins on the seam, so nothing depends on atlas ranges.
- **One dummy SVG icon** — `IconDepot` rasterises the single `DummyGlyph.svg` (rounded square + dot) and every
  classification seat presents it tinted with the reference's classification colour. No icon artwork was
  authored, generated or imported; affordances (chevrons, eyes, plus, search, trash, checks) are drawn strokes.
- **Slate conventions** — SKILL-Naming (closed role suffixes, banned words avoided), SKILL-Formatting
  (142/122 rulers, `/// 🧩` annotation blocks, `[px]` columns), `Deliver<Content>`/`Refusal`, and per-unit
  `Module.toml` declarative maps beside the Makefile build.

## Layout

```
Engine/Contract/Api/DeliveryContract.h       the fallible-delivery contract (standalone slice)
Engine/SlateUI/Interface/…                   one unit per panel concern (Api + Source + Module.toml)
Engine/Application/…                         the two host executables
ExternalPackages/imgui                       vendored, pinned (see PINNED.md)
References/remix-remix-global-ui, References/Cad   the unmodified references (from SultanAladin/Slate@f19e4ff)
Tools/EncodeProof.py, Tools/ProofProbe.py    proof encode + ink-assert readback
VisualProof/…                                the rendered proof shots
```
