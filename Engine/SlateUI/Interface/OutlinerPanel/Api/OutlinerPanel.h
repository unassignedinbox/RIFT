//============================================================================================================================================
//                                                          OUTLINERPANEL.H
//============================================================================================================================================
// 🧩 The general-purpose world outliner and entry inspector, transcribed from GameOutliner.tsx — declarations in, one recorded tree out.

#pragma once

#include "Engine/Contract/Api/DeliveryContract.h"
#include "Engine/SlateUI/Interface/IconDepot/Api/IconDepot.h"
#include "Engine/SlateUI/Interface/RecordingSurface/Api/RecordingSurface.h"
#include "Engine/SlateUI/Interface/ThemeSpecification/Api/ThemeSpecification.h"

#include <cstdint>

namespace Slate
{

//------------------------------------------------------------------------------------------------------------------------
//                                                   THE CLASSIFICATIONS
//------------------------------------------------------------------------------------------------------------------------

/// 🧩 Which constituent of the world a row declares — drives the dummy glyph's tint.
/// tag   contract, nonallocating, nonthrowing
enum class OutlinerClassification : std::uint32_t
{
    Level                = 0u,   // [-] - #eab308
    Enclosure            = 1u,   // [-] - #8a8a8a  (the reference's `folder`)
    Actor                = 2u,   // [-] - #3b82f6
    Camera               = 3u,   // [-] - #ec4899
    Light                = 4u,   // [-] - #f59e0b
    Audio                = 5u,   // [-] - #8b5cf6
    Particle             = 6u,   // [-] - #10b981
    Trigger              = 7u,   // [-] - #ef4444
    Script               = 8u,   // [-] - #06b6d4
    ClassificationCount  = 9u    // [-] - the closed count, never a classification
};

/// 🧩 The tint a classification carries, verbatim from the reference's colour record.
/// cost  ✔️
/// tag   api, nonallocating, nonthrowing
InkOrdinate ClassificationTint(OutlinerClassification Classification);

/// 🧩 The lowercase run a classification spells in the inspector subtitle.
/// cost  ✔️
/// tag   api, nonallocating, nonthrowing
const char* ClassificationRun(OutlinerClassification Classification);

//------------------------------------------------------------------------------------------------------------------------
//                                                    THE ROW DECLARATION
//------------------------------------------------------------------------------------------------------------------------

/// 🧩 One row the outliner seats — borrowed caption, classification, host-owned disclosure and presence,
///       and the enclosed forest beneath it. No presented datum is owned.
/// tag   contract, nonallocating, nonthrowing
struct OutlinerRowDeclaration
{
    const char*                   Caption         = "";           // [-] - borrowed; outlives the tick
    const char*                   Identity        = "";           // [-] - borrowed; the host's token
    OutlinerClassification        Classification  = OutlinerClassification::Actor;
    bool*                         Expanded        = nullptr;      // [-] - host-owned disclosure
    bool*                         Hidden          = nullptr;      // [-] - host-owned presence
    const OutlinerRowDeclaration* Enclosed        = nullptr;      // [-] - borrowed forest beneath
    std::uint32_t                 EnclosureCount  = 0u;           // [-] - rows directly enclosed
};

/// 🧩 The composition caption pair the head carries.
/// tag   contract, nonallocating, nonthrowing
struct OutlinerComposition
{
    const char* TitleRun   = "World Outliner";   // [-] - borrowed
    const char* ContextRun = "Level_01_City";    // [-] - borrowed
};

/// 🧩 One inspected entry's ordinates, seated at the reference's declared defaults.
/// note  🔴 The host owns these; the inspector writes through its reference, exactly as the sheet's
///       ordinates belong to the host in the engine's validation seat.
/// tag   contract, nonallocating, nonthrowing
struct EntryOrdinates
{
    double  Position[3]     = { 0.0, 0.0, 0.0 };        // [m]   - Transform.Position
    double  Rotation[3]     = { 0.0, 0.0, 0.0 };        // [deg] - Transform.Rotation
    double  Scale[3]        = { 1.0, 1.0, 1.0 };        // [-]   - Transform.Scale

    std::uint32_t  Intensity    = 100000u;               // [lm]  - light
    std::uint8_t   LightColour[4] = { 255u, 240u, 220u, 255u };   // [-] - light
    bool           CastShadows  = true;                  // [-]   - light

    std::uint32_t  Projection   = 0u;                    // [-]   - camera (0 perspective, 1 orthographic)
    double         FieldOfView  = 90.0;                  // [deg] - camera
    double         NearClip     = 0.1;                   // [m]   - camera
    double         FarClip      = 10000.0;               // [m]   - camera

    double         Volume       = 0.8;                   // [-]   - audio
    bool           Looping      = true;                  // [-]   - audio / particle
    bool           Spatial      = false;                 // [-]   - audio

    double         EmitRate     = 50.0;                  // [/s]  - particle
    double         LifeTime     = 5.0;                   // [s]   - particle

    char           EventTag[32] = "Spawn";               // [-]   - trigger
    double         TriggerRadius = 50.0;                 // [m]   - trigger

    std::uint32_t  ScriptState  = 0u;                    // [-]   - script (0 playing, 1 paused, 2 stopped)
    std::uint32_t  Difficulty   = 1u;                    // [-]   - script

    bool           StaticMesh   = true;                  // [-]   - actor
    bool           SimulatePhysics   = false;            // [-]   - actor
    bool           GenerateOverlaps  = true;             // [-]   - actor

    bool           WorldPartition    = true;             // [-]   - level
    bool           EditorOnly        = false;            // [-]   - enclosure
};

//------------------------------------------------------------------------------------------------------------------------
//                                                     THE OUTLINER PANEL
//------------------------------------------------------------------------------------------------------------------------

/// 🧩 The general-purpose outliner — head, retention field, the disclosure forest, and the count foot.
/// note  Retention retains a row when its caption carries the run case-insensitively, or any enclosed row
///       is retained; while a run stands, every branch presents open, exactly as the reference filters.
/// tag   contract, nonallocating, nonthrowing
class OutlinerPanel
{
public:

    OutlinerPanel()                             = default;
    OutlinerPanel(const OutlinerPanel&)         = delete;
    OutlinerPanel& operator=(const OutlinerPanel&) = delete;
    ~OutlinerPanel()                            = default;

    /// 🧩 Binds the glyph depot whose dummy glyph the classification seats present.
    /// cost  ✔️
    /// tag   api, nonallocating, nonthrowing
    Deliver<bool> Construct(const IconDepot& ArrivingDepot);

    /// 🧩 Presents the outliner inside the seat extent, one tick.
    /// tag   api, nonallocating, nonthrowing
    void Advance(RecordingSurface& Surface, const PlaneExtent& Seat,
                 const OutlinerRowDeclaration* Rows, std::uint32_t RowCount,
                 const OutlinerComposition& Composition);

    /// 🧩 The retention run, host-readable and host-writable.
    /// cost  ✔️
    /// tag   api, nonallocating, nonthrowing
    char RetentionRun[64] = "";   // [-] - the live run

    /// 🧩 The identity of the taken row, or the vacated run when nothing stands taken.
    /// cost  ✔️
    /// tag   api, nonallocating, nonthrowing
    char TakenIdentity[32] = "";   // [-] - taken row token

    /// 🧩 Raised for one tick when a row's inspect gesture (double press) lands.
    /// cost  ✔️
    /// tag   api, nonallocating, nonthrowing
    bool InspectRaised = false;   // [-] - double-press edge

private:

    /// 🧩 Presents one row and its enclosed forest; reports whether any row was taken or inspected.
    /// tag   internal, nonallocating, nonthrowing
    void PresentRow(RecordingSurface& Surface, const PlaneExtent& Body, const OutlinerRowDeclaration& Row,
                    std::uint32_t Depth, bool RetentionStanding);

    /// 🧩 Whether the row, or any row it encloses, carries the retention run.
    /// cost  🚩
    /// tag   internal, nonallocating, nonthrowing
    bool Retained(const OutlinerRowDeclaration& Row) const;

    /// 🧩 Counts the row and every row beneath it.
    /// cost  🚩
    /// tag   internal, nonallocating, nonthrowing
    std::uint32_t CountEnclosed(const OutlinerRowDeclaration* Rows, std::uint32_t RowCount) const;

    const IconDepot*  Depot        = nullptr;   // [-] - the dummy glyph depot
    float             ScrollAcross = 0.0f;      // [px] - body scroll ordinate
    std::uint32_t     PresentedCount = 0u;      // [-]  - rows presented this tick (for the foot)
};

//------------------------------------------------------------------------------------------------------------------------
//                                                   THE ENTRY INSPECTOR
//------------------------------------------------------------------------------------------------------------------------

/// 🧩 The entry inspector — the reference's GamePropertiesPane: head, transform card, classification card.
/// tag   contract, nonallocating, nonthrowing
class EntryInspectorPanel
{
public:

    /// 🧩 Presents the inspector for one declared row inside the seat extent.
    /// in    Declared  [-]  the row whose components are presented; nullptr presents the vacated seat
    /// tag   api, nonallocating, nonthrowing
    void Advance(RecordingSurface& Surface, const PlaneExtent& Seat,
                 const OutlinerRowDeclaration* Declared, EntryOrdinates& Ordinates,
                 const IconDepot& Depot);

    /// 🧩 Raised for one tick when the back action is pressed.
    /// cost  ✔️
    /// tag   api, nonallocating, nonthrowing
    bool BackRaised = false;   // [-] - the back action edge

    /// 🧩 Per-entry colour swatch presentation seat, raised for one tick when the swatch is pressed.
    /// cost  ✔️
    /// tag   api, nonallocating, nonthrowing
    bool ColourSeatedOpen = false;   // [-] - light colour swatch
};

}   // namespace Slate
