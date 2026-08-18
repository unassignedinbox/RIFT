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
| `remix-remix-global-ui/components/DirectoryPane.tsx` | `Engine/SlateUI/Interface/OutlinerPanel` — the scene directory, the general-purpose outliner | `OutlinerHost` (standalone) and reused inside `DraftingPanel` |
| `remix-remix-global-ui/components/Inspector.tsx` | `Engine/SlateUI/Interface/PropertiesPanel` — record cards + the **Properties / History carousel** | `PanelValidationHost` |
| `remix-remix-global-ui/components/MetadataPane.tsx` + `page.tsx` drafting seat | `Engine/SlateUI/Interface/DraftingPanel` — directory + Properties & Actions bar + metadata pane | `PanelValidationHost` |
| `remix-remix-global-ui/components/TexturePaint.tsx` | `Engine/SlateUI/Interface/TexturePaintPanel` — `LayersPane`, `ChannelPropertyPanel`, `MaskPropertyPanel` | `PanelValidationHost` |
| `remix-remix-global-ui/app/globals.css` + `components/Controls.tsx` | `Engine/SlateUI/Interface/ThemeSpecification` (the control-panel theme tokens) + `ControlPanel` (the shared widget kit — every field row in every panel is a `ControlPanel` widget) | both |

The reference folders themselves are vendored unmodified under `References/` as the comparison of record.

## The two executables

```bash
make                # builds Build/OutlinerHost and Build/PanelValidationHost
make proof          # runs both headlessly and encodes VisualProof/*.png
```

- **`OutlinerHost`** — the **standalone outliner**: the scene directory alone on the desk (no editor chrome,
  no viewport, no lattice), seeded with the reference's `initialStore` (Bracket_Rev4). Three states:
  `directory` (SOL_Plate taken), `multiselect` (control-gesture additive selection, "3 sel" pill),
  `filter` (retention run `sk`).
- **`OutlinerWindowHost`** (`make outliner-window`) — the interactive variant behind a GLFW window.
  Requires local GLFW + OpenGL dev packages; the headless build never compiles it.
- **`PanelValidationHost`** — the validation: the texture-paint panels (`texturepaint-layers`,
  `texturepaint-mask`, `texturepaint-reorder` — a scripted live drag-to-reorder) and the **CAD drafting
  panel** (`cad-properties`, `cad-history`) — the scene directory beside the metadata pane, with the
  record inspector's Properties / History carousel. The outliner inside the CAD panel *is* the standalone
  `OutlinerPanel`, reused.
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
