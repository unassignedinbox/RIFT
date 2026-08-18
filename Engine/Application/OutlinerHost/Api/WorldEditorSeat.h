//============================================================================================================================================
//                                                     WORLDEDITORSEAT.H
//============================================================================================================================================
// 🧩 The world-editor composition — top bar, options menu, viewport, docked inspector — shared by every outliner host.

#pragma once

#include "Engine/SlateUI/Interface/IconDepot/Api/IconDepot.h"
#include "Engine/SlateUI/Interface/OutlinerPanel/Api/OutlinerPanel.h"
#include "Engine/SlateUI/Interface/RecordingSurface/Api/RecordingSurface.h"

#include <cstdint>

namespace Rift
{

/// 🧩 Every host-owned ordinate and disclosure the seed forest carries, seated at the reference's values.
/// tag   contract, nonallocating, nonthrowing
struct SeedStand
{
    bool ExpandedLighting    = true;    // [-] - g_02
    bool ExpandedEnvironment = true;    // [-] - g_07
    bool ExpandedSystems     = true;    // [-] - g_11
    bool HiddenLighting      = false;   // [-] - g_02
    bool HiddenSun           = false;   // [-] - g_03

    Slate::EntryOrdinates Sun;            // [-] - g_03
    Slate::EntryOrdinates Atmosphere;     // [-] - g_04
    Slate::EntryOrdinates PlayerStart;    // [-] - g_05
    Slate::EntryOrdinates MainCamera;     // [-] - g_06
    Slate::EntryOrdinates BuildingA;      // [-] - g_08
    Slate::EntryOrdinates BuildingB;      // [-] - g_09
    Slate::EntryOrdinates FireHydrant;    // [-] - g_10
    Slate::EntryOrdinates GameManager;    // [-] - g_12
    Slate::EntryOrdinates CityNoise;      // [-] - g_13
    Slate::EntryOrdinates DustMotes;      // [-] - g_14

    SeedStand();
};

/// 🧩 The assembled seed forest, borrowed from one seed stand.
/// note  🔴 Self-referential — never returned by value; the caller owns one and AssembleForest wires it.
/// tag   contract, nonallocating, nonthrowing
struct ForestStand
{
    Slate::OutlinerRowDeclaration Root[1];          // [-] - g_01
    Slate::OutlinerRowDeclaration Lighting[2];      // [-] - g_03, g_04
    Slate::OutlinerRowDeclaration Environment[3];   // [-] - g_08..g_10
    Slate::OutlinerRowDeclaration Systems[3];       // [-] - g_12..g_14
    Slate::OutlinerRowDeclaration Enclosures[3];    // [-] - g_02, g_07, g_11
    Slate::OutlinerRowDeclaration Standing[2];      // [-] - g_05, g_06
    Slate::OutlinerRowDeclaration Enclosed[5];      // [-] - the root's direct forest
};

/// 🧩 Wires the forest against the stand's disclosures, presences and enclosed rows.
/// pre   Forest outlives every tick that presents it
/// tag   api, nonallocating, nonthrowing
void AssembleForest(SeedStand& Stand, ForestStand& Forest);

/// 🧩 Finds the row declaration carrying the identity, through the whole forest.
/// cost  🚩
/// tag   api, nonallocating, nonthrowing
const Slate::OutlinerRowDeclaration* FindRow(const Slate::OutlinerRowDeclaration* Rows, std::uint32_t RowCount, const char* Identity);

/// 🧩 The ordinates a row identity inspects.
/// cost  ✔️
/// tag   api, nonallocating, nonthrowing
Slate::EntryOrdinates* FindOrdinates(SeedStand& Stand, const char* Identity);

/// 🧩 Presents the whole seat — top bar, options, viewport, inspector — one tick.
/// tag   api, nonallocating, nonthrowing
void PresentWorldEditorSeat(Slate::RecordingSurface& Surface, const Slate::PlaneExtent& Desk, Slate::OutlinerPanel& Outliner,
                            Slate::EntryInspectorPanel& Inspector, const Slate::IconDepot& Depot, SeedStand& Stand,
                            ForestStand& Forest, bool SlideOpen);

}   // namespace Rift
