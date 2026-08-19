//============================================================================================================================================
//                                                       THEMESPECIFICATION.H
//============================================================================================================================================
// 🧩 Every ink the transcribed reference panels draw with — the control-panel theme of globals.css, the channel sheet, and the CAD sheet.

#pragma once

#include <cstdint>

namespace Slate
{
namespace Reference
{

//------------------------------------------------------------------------------------------------------------------------
//                                                          INK
//------------------------------------------------------------------------------------------------------------------------

/// 🧩 One display-referred ink, packed at eight bits per component.
/// tag   contract, nonallocating, nonthrowing
struct InkOrdinate
{
    std::uint8_t  Red     = 0u;     // [-] - sRGB-encoded, never linear
    std::uint8_t  Green   = 0u;     // [-]
    std::uint8_t  Blue    = 0u;     // [-]
    std::uint8_t  Opacity = 255u;   // [-] - 255 is fully covering
};

/// 🧩 Constructs a fully covering ink from a packed 0xRRGGBB literal.
/// cost  ✔️
constexpr InkOrdinate Covering(std::uint32_t Packed)
{
    return InkOrdinate{ static_cast<std::uint8_t>((Packed >> 16) & 0xFFu),
                        static_cast<std::uint8_t>((Packed >>  8) & 0xFFu),
                        static_cast<std::uint8_t>( Packed        & 0xFFu),
                        255u };
}

/// 🧩 Constructs an ink at a declared coverage, matching CSS `color-mix(… n%, transparent)`.
/// cost  ✔️
constexpr InkOrdinate Partial(std::uint32_t Packed, double Coverage)
{
    InkOrdinate Constructed = Covering(Packed);
    Constructed.Opacity     = static_cast<std::uint8_t>(Coverage * 255.0 + 0.5);
    return Constructed;
}

/// 🧩 Linearly mixes two inks, zero returns Leading and one returns Trailing.
/// cost  ✔️
constexpr InkOrdinate Mix(const InkOrdinate& Leading, const InkOrdinate& Trailing, double Fraction)
{
    const double f = Fraction < 0.0 ? 0.0 : (Fraction > 1.0 ? 1.0 : Fraction);
    return InkOrdinate
    {
        static_cast<std::uint8_t>(Leading.Red     + (Trailing.Red     - Leading.Red)     * f + 0.5),
        static_cast<std::uint8_t>(Leading.Green   + (Trailing.Green   - Leading.Green)   * f + 0.5),
        static_cast<std::uint8_t>(Leading.Blue    + (Trailing.Blue    - Leading.Blue)    * f + 0.5),
        static_cast<std::uint8_t>(Leading.Opacity + (Trailing.Opacity - Leading.Opacity) * f + 0.5)
    };
}

//------------------------------------------------------------------------------------------------------------------------
//                                            THE WORKSPACE SHEET (globals.css)
//------------------------------------------------------------------------------------------------------------------------

// 📝 The workspace sheet is the control-panel theme the whole remix-remix-global-ui reference draws with:
//    `--desk #0a0a0b`, `--menu #17171a`, `--menu-2 #101012`, `--tile #1d1d21`, `--accent #4a90e2` and the
//    field tokens of the Controls preview (`--value-bg`, `--track-bg`, `--knob`, …), transcribed verbatim.
/// 🧩 Every ink the workspace sheet declares, named by the responsibility it carries.
/// tag   contract, nonallocating, nonthrowing
struct WorkspaceInk
{
    // Surfaces
    InkOrdinate  DeskGround       = Covering(0x0A0A0Bu);   // [-] - --desk
    InkOrdinate  StandingGround   = Covering(0x17171Au);   // [-] - --menu
    InkOrdinate  SunkenGround     = Covering(0x101012u);   // [-] - --menu-2
    InkOrdinate  RailTaken        = Covering(0x232327u);   // [-] - --rail-sel
    InkOrdinate  TileGround       = Covering(0x1D1D21u);   // [-] - --tile
    InkOrdinate  TileRoused       = Covering(0x26262Bu);   // [-] - --tile-hi
    InkOrdinate  HairEdge         = Partial(0xFFFFFFu, 0.06);   // [-] - --hair
    InkOrdinate  HairEdgeStrong   = Partial(0xFFFFFFu, 0.10);   // [-] - --hair-strong

    // Ink ladder
    InkOrdinate  Accent           = Covering(0x4A90E2u);   // [-] - --accent (Controls preview)
    InkOrdinate  AccentSoft       = Partial(0xFFFFFFu, 0.12);   // [-] - --accent-soft
    InkOrdinate  InkPrimary       = Covering(0xECECF0u);   // [-] - --ink
    InkOrdinate  InkMuted         = Covering(0x7B7B82u);   // [-] - --muted
    InkOrdinate  InkFaint         = Covering(0x55555Du);   // [-] - --faint

    // Field widgets (the control panel's preview sheet)
    InkOrdinate  FieldGround      = Covering(0x232326u);   // [-] - --value-bg
    InkOrdinate  FieldSunken      = Covering(0x0A0A0Bu);   // [-] - --value-black
    InkOrdinate  FieldNumeral     = Covering(0x131315u);   // [-] - --value-num
    InkOrdinate  FieldUnit        = Covering(0x33333Au);   // [-] - --value-unit
    InkOrdinate  FieldOutline     = Partial(0xFFFFFFu, 0.22);   // [-] - --outline
    InkOrdinate  FieldFocus       = Covering(0x303035u);   // [-] - --value-focus
    InkOrdinate  TrackGround      = Covering(0x2F2F33u);   // [-] - --track-bg
    InkOrdinate  TrackFill        = Covering(0x8A8A8Eu);   // [-] - --track-fill
    InkOrdinate  KnobInk          = Covering(0xF4F4F5u);   // [-] - --knob

    // Row states
    InkOrdinate  RowRoused        = Partial(0xFFFFFFu, 0.045);   // [-] - --row-hover
    InkOrdinate  RowTakenGround   = Partial(0x1E40AFu, 0.20);    // [-] - bg-[#1e40af33]
    InkOrdinate  RowTakenRail     = Covering(0x3B82F6u);         // [-] - the 3 px selection rail
    InkOrdinate  AccentTile       = Covering(0x3B82F6u);         // [-] - the outliner head medallion

    // Measured extents (not inks, seated beside the sheet they belong to)
    static constexpr float SheetRadius   = 18.0f;   // [px] - --r-menu
    static constexpr float TileRadius    = 12.0f;   // [px] - --r-tile
    static constexpr float RowExtent     = 36.0f;   // [px] - --row-h
};

//------------------------------------------------------------------------------------------------------------------------
//                                    THE CHANNEL SHEET (TexturePaint embedded theme)
//------------------------------------------------------------------------------------------------------------------------

// 📝 The texture-paint panels carry their own embedded sheet (`--desk #000`, `--menu #0e0e0e`, `--tile #141414`,
//    `--accent #e8e8e8`, `--marker #4a90e2`, `--danger #e05a5a` …), distinct from the workspace sheet.
/// 🧩 Every ink the texture-paint channel and mask sheets declare.
/// tag   contract, nonallocating, nonthrowing
struct ChannelInk
{
    InkOrdinate  DeskGround       = Covering(0x000000u);   // [-] - --desk
    InkOrdinate  StandingGround   = Covering(0x0E0E0Eu);   // [-] - --menu
    InkOrdinate  SunkenGround     = Covering(0x0E0E0Eu);   // [-] - --menu-2
    InkOrdinate  TileGround       = Covering(0x141414u);   // [-] - --tile
    InkOrdinate  TileRoused       = Covering(0x1A1A1Au);   // [-] - --tile-hi
    InkOrdinate  TileTaken        = Covering(0x1F1F1Fu);   // [-] - --tile-active
    InkOrdinate  HairEdge         = Covering(0x1C1C1Cu);   // [-] - --hair
    InkOrdinate  HairEdgeStrong   = Covering(0x2A2A30u);   // [-] - --hair-strong

    InkOrdinate  Accent           = Covering(0xE8E8E8u);   // [-] - --accent (near-white)
    InkOrdinate  OnAccent         = Covering(0x111111u);   // [-] - --on-accent
    InkOrdinate  Marker           = Covering(0x4A90E2u);   // [-] - --marker (the blue seat)
    InkOrdinate  InkPrimary       = Covering(0xEDEDEDu);   // [-] - --ink
    InkOrdinate  InkMuted         = Covering(0x8A8A8Au);   // [-] - --muted
    InkOrdinate  InkFaint         = Covering(0x6A6A6Au);   // [-] - --faint
    InkOrdinate  Danger           = Covering(0xE05A5Au);   // [-] - --danger

    InkOrdinate  FieldSunken      = Covering(0x000000u);   // [-] - --value-black
    InkOrdinate  FieldUnit        = Covering(0x141414u);   // [-] - --value-unit
    InkOrdinate  TrackGround      = Covering(0x141414u);   // [-] - --track-bg
    InkOrdinate  TrackFill        = Covering(0x5A5A5Au);   // [-] - --track-fill
    InkOrdinate  KnobInk          = Covering(0xFFFFFFu);   // [-] - --knob

    InkOrdinate  RowRoused        = Partial(0xFFFFFFu, 0.045);   // [-] - --row-hover

    // The layer-stack sheet (LayersPane) is a touch warmer than the channel sheet.
    InkOrdinate  StackGround      = Covering(0x0B0B0Bu);   // [-] - bg-[#0b0b0b]
    InkOrdinate  StackHead        = Covering(0x121214u);   // [-] - bg-[#121214]
    InkOrdinate  StackSpineVacant = Covering(0x1B1B1Bu);   // [-] - the hidden spine
    InkOrdinate  StackMedallion   = Covering(0x2A2A2Au);   // [-] - the hidden ordinal medallion
    InkOrdinate  StackEdge        = Covering(0x1C1C1Cu);   // [-] - border-[#1c1c1c]
    InkOrdinate  StackTile        = Covering(0x141414u);   // [-] - border-[#242424] family ground
    InkOrdinate  StackTileRoused  = Covering(0x1A1A1Au);   // [-] - hover border family

    // Classification tints the layer stack declares.
    static constexpr std::uint32_t PaintTint      = 0xF97316u;   // [-] - KINDS[0].Tint
    static constexpr std::uint32_t MaterialTint   = 0x8B5CF6u;   // [-] - KINDS[1].Tint
    static constexpr std::uint32_t GeneratorTint  = 0x10B981u;   // [-] - KINDS[2].Tint
};

//------------------------------------------------------------------------------------------------------------------------
//                                          THE CAD SHEET (Cad/css/theme.css)
//------------------------------------------------------------------------------------------------------------------------

/// 🧩 Every ink the CAD workspace sheet declares.
/// tag   contract, nonallocating, nonthrowing
struct CadInk
{
    InkOrdinate  DeskGround       = Covering(0x000000u);   // [-] - --bg
    InkOrdinate  PanelGround      = Covering(0x0A0A0Au);   // [-] - --panel
    InkOrdinate  PanelRaised      = Covering(0x0E0E0Eu);   // [-] - --panel-2
    InkOrdinate  PanelLifted      = Covering(0x141414u);   // [-] - --panel-3
    InkOrdinate  HeaderGround     = Covering(0x070707u);   // [-] - --header
    InkOrdinate  StageGround      = Covering(0x050608u);   // [-] - the canvas stage
    InkOrdinate  TabGround        = Covering(0x050505u);   // [-] - the tab bar
    InkOrdinate  TabRoused        = Covering(0x161618u);   // [-] - tab hover

    InkOrdinate  RowRoused        = Covering(0x1A1A1Au);   // [-] - --row-hover
    InkOrdinate  RowTakenGround   = Covering(0x1F1F1Fu);   // [-] - --row-selected
    InkOrdinate  RowTakenEdge     = Covering(0x3A3A3Au);   // [-] - --row-selected-bd

    InkOrdinate  InkPrimary       = Covering(0xEDEDEDu);   // [-] - --text
    InkOrdinate  InkMuted         = Covering(0x8A8A8Au);   // [-] - --text-dim
    InkOrdinate  InkFaint         = Covering(0x5A5A5Au);   // [-] - --text-faint
    InkOrdinate  HairEdge         = Covering(0x1C1C1Cu);   // [-] - --border

    InkOrdinate  Accent           = Covering(0xFFFFFFu);   // [-] - --accent
    InkOrdinate  CadSoft          = Partial(0xFFFFFFu, 0.14);   // [-] - --cad-soft
    InkOrdinate  CadEdge          = Partial(0xFFFFFFu, 0.42);   // [-] - --cad-bd

    InkOrdinate  Green            = Covering(0x22C55Eu);   // [-] - --green (Live)
    InkOrdinate  Red              = Covering(0xEF4444u);   // [-] - --red
    InkOrdinate  Blue             = Covering(0x3B82F6u);   // [-] - --blue
    InkOrdinate  Amber            = Covering(0xF59E0Bu);   // [-] - --amber
    InkOrdinate  Purple           = Covering(0xA855F7u);   // [-] - --purple

    // The seeded plane swatches the CAD browser tree carries.
    static constexpr std::uint32_t FrontSwatch  = 0xEF4444u;   // [-] - Front Plane
    static constexpr std::uint32_t TopSwatch    = 0x22C55Eu;   // [-] - Top Plane
    static constexpr std::uint32_t RightSwatch  = 0x3B82F6u;   // [-] - Right Plane
};

}   // namespace Reference
}   // namespace Slate
