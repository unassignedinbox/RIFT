//============================================================================================================================================
//                                                            CONTROLPANEL.H
//============================================================================================================================================
// 🧩 Reusable inspector controls transcribed from the global-interface reference control panel, with no presented datum owned.

#pragma once

#include "Engine/SlateUI/Interface/IconDepot/Api/IconDepot.h"
#include "Engine/SlateUI/Interface/RecordingSurface/Api/RecordingSurface.h"
#include "Engine/SlateUI/Interface/ThemeSpecification/Api/ThemeSpecification.h"

#include <cstdint>

namespace Slate
{

//------------------------------------------------------------------------------------------------------------------------
//                                                     THE CONTROL SHEET
//------------------------------------------------------------------------------------------------------------------------

/// 🧩 The inks a control reads, gathered from whichever sheet the seating panel draws with.
/// tag   contract, nonallocating, nonthrowing
struct ControlSheet
{
    InkOrdinate  FieldSunken;      // [-] - the numeral ground
    InkOrdinate  FieldUnit;        // [-] - the unit segment
    InkOrdinate  FieldFocus;       // [-] - the roused unit segment
    InkOrdinate  TrackGround;      // [-] - the track ground
    InkOrdinate  TrackFill;        // [-] - the track fill
    InkOrdinate  KnobInk;          // [-] - the knob
    InkOrdinate  TileGround;       // [-] - a quiet tile
    InkOrdinate  TileRoused;       // [-] - a roused tile
    InkOrdinate  TileTaken;        // [-] - a taken segment ground
    InkOrdinate  HairEdge;         // [-] - a hair edge
    InkOrdinate  HairEdgeStrong;   // [-] - a strong hair edge
    InkOrdinate  InkPrimary;       // [-] - primary run ink
    InkOrdinate  InkMuted;         // [-] - muted run ink
    InkOrdinate  InkFaint;         // [-] - faint run ink
    InkOrdinate  Accent;           // [-] - the sheet accent
    InkOrdinate  OnAccent;         // [-] - run ink over the accent
    InkOrdinate  RowRoused;        // [-] - row rousing
    float        RowExtent = 36.0f;   // [px] - --row-h
};

/// 🧩 Gathers the control sheet from the workspace sheet of globals.css.
/// cost  ✔️
ControlSheet ControlSheetFromWorkspace(const WorkspaceInk& Sheet);

/// 🧩 Gathers the control sheet from the texture-paint channel sheet.
/// cost  ✔️
ControlSheet ControlSheetFromChannel(const ChannelInk& Sheet);

/// 🧩 Gathers the control sheet from the CAD sheet.
/// cost  ✔️
ControlSheet ControlSheetFromCad(const CadInk& Sheet);

//------------------------------------------------------------------------------------------------------------------------
//                                                  CONTROL DECLARATIONS
//------------------------------------------------------------------------------------------------------------------------

/// 🧩 One captioned control row's fixed geometry, shared by every widget.
/// tag   contract, nonallocating, nonthrowing
struct ControlRowDeclaration
{
    const char*          Caption    = "";      // [-] - borrowed; the leading label
    float                CaptionExtent = 88.0f;   // [px] - the label column
    float                CaptionSize    = 13.5f;   // [px] - the label presentation size
};

/// 🧩 One numeric slider's declared range and presentation.
/// tag   contract, nonallocating, nonthrowing
struct SliderDeclaration
{
    double  Minimum   = 0.0;      // [-] - range floor
    double  Maximum   = 1.0;      // [-] - range ceiling
    std::uint32_t  Figures  = 2u;      // [-] - decimal places presented
    const char*    Unit     = "·";     // [-] - the unit segment run
    float          NumeralExtent = 78.0f;   // [px] - the value capsule extent
};

//------------------------------------------------------------------------------------------------------------------------
//                                                  THE CONTROL PRESENTATIONS
//------------------------------------------------------------------------------------------------------------------------

/// 🧩 The retention field — the reference's search pill: quiet ground, hair edge, search affordance, entry.
/// in    Placeholder  [-]  borrowed; the vacated run
/// in    ExtentAlong  [px]  the field's along span
/// out   true while the entry stands focused
/// tag   api, nonallocating, nonthrowing
bool PresentRetentionField(RecordingSurface& Surface, const PlaneExtent& Seat, char* Run, std::uint32_t RunCapacity,
                           const char* Placeholder, const InkOrdinate& FieldGround, const InkOrdinate& FieldEdge,
                           const InkOrdinate& RunInk, const InkOrdinate& VacantInk);

/// 🧩 One labelled switch row — track and nub reproduce `.switch` and `.nub` from the reference.
/// tag   api, nonallocating, nonthrowing
void PresentSwitchRow(RecordingSurface& Surface, const PlaneExtent& Row, const ControlRowDeclaration& Declared,
                      bool& Taken, const ControlSheet& Sheet, const char* PushIdentity);

/// 🧩 One labelled segment row — mutually exclusive caption pills, the taken one inverted.
/// tag   api, nonallocating, nonthrowing
void PresentSegmentRow(RecordingSurface& Surface, const PlaneExtent& Row, const ControlRowDeclaration& Declared,
                       const char* const* Captions, std::uint32_t CaptionCount, std::uint32_t& Taken,
                       const ControlSheet& Sheet, const char* PushIdentity);

/// 🧩 One labelled dropdown row — pill head with caret, opening a styled option list.
/// tag   api, nonallocating, nonthrowing
void PresentDropdownRow(RecordingSurface& Surface, const PlaneExtent& Row, const ControlRowDeclaration& Declared,
                        const char* const* Captions, std::uint32_t CaptionCount, std::uint32_t& Taken,
                        const ControlSheet& Sheet, const char* PushIdentity);

/// 🧩 One labelled value slider row — numeral capsule, unit segment, track, fill, knob.
/// tag   api, nonallocating, nonthrowing
void PresentSliderRow(RecordingSurface& Surface, const PlaneExtent& Row, const ControlRowDeclaration& Declared,
                      const SliderDeclaration& Range, double& Amount, const ControlSheet& Sheet, const char* PushIdentity);

/// 🧩 One labelled scalar row — the reference's ScalarEntry: numeral capsule beside a centred-knob track,
///       the capsule drag stepping the amount.
/// tag   api, nonallocating, nonthrowing
void PresentScalarRow(RecordingSurface& Surface, const PlaneExtent& Row, const ControlRowDeclaration& Declared,
                      const SliderDeclaration& Range, double Step, double& Amount, const ControlSheet& Sheet,
                      const char* PushIdentity);

/// 🧩 One labelled vector row — three axis capsules, each numeral editable.
/// tag   api, nonallocating, nonthrowing
void PresentVectorRow(RecordingSurface& Surface, const PlaneExtent& Row, const ControlRowDeclaration& Declared,
                      double Ordinates[3], double Step, const ControlSheet& Sheet, const char* PushIdentity);

/// 🧩 One labelled colour row — the reference's colour pill with circle swatch and run of ordinates.
/// tag   api, nonallocating, nonthrowing
void PresentColourRow(RecordingSurface& Surface, const PlaneExtent& Row, const ControlRowDeclaration& Declared,
                      std::uint8_t Ordinates[4], bool& SeatedOpen, const ControlSheet& Sheet, const char* PushIdentity);

/// 🧩 One labelled text row — the reference's pathfield pill with the round browse action.
/// tag   api, nonallocating, nonthrowing
void PresentTextRow(RecordingSurface& Surface, const PlaneExtent& Row, const ControlRowDeclaration& Declared,
                    char* Run, std::uint32_t RunCapacity, const ControlSheet& Sheet, const char* PushIdentity);

}   // namespace Slate
