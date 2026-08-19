//============================================================================================================================================
//                                                         PROPERTIESPANEL.H
//============================================================================================================================================
// 🧩 The record inspector with the Properties / History carousel, transcribed from Inspector.tsx — every field a ControlPanel widget.

#pragma once

#include "Contract/Api/PanelContract.h"
#include "SlateUI/Interface/IconDepot/Api/IconDepot.h"
#include "SlateUI/Interface/OutlinerPanel/Api/OutlinerPanel.h"
#include "SlateUI/Interface/PanelExchange/Api/PanelExchange.h"
#include "SlateUI/Interface/ThemeSpecification/Api/ThemeSpecification.h"

#include <cstdint>

namespace Rift
{

//------------------------------------------------------------------------------------------------------------------------
//                                                    THE RECORD PROFILE
//------------------------------------------------------------------------------------------------------------------------

/// 🧩 Every profile ordinate the inspector writes through — RecordProfile, seated by establishProfile.
/// tag   contract, nonallocating, nonthrowing
struct ProfileOrdinates
{
    char          Name[32]        = "";              // [-]
    bool          Visible         = true;            // [-]
    double        Position[3]     = { 0.0, 0.0, 0.0 };   // [mm]
    double        Rotation[3]     = { 0.0, 0.0, 0.0 };   // [deg]
    double        Scale[3]        = { 1.0, 1.0, 1.0 };   // [-]
    std::uint8_t  Albedo[4]       = { 214u, 216u, 222u, 255u };   // [-]
    double        Roughness       = 0.42;            // [-]
    double        Metalness       = 0.08;            // [-]
    std::uint32_t ShadingMode     = 0u;              // [-] - smooth, faceted, flat
    bool          Selectable      = true;            // [-]

    std::uint32_t Units           = 1u;              // [-] - scene: millimetres seated
    double        ToleranceLinear  = 0.01;           // [mm]
    double        ToleranceAngular = 0.5;            // [deg]
    char          DocumentPath[64] = "/Projects/Bracket_Rev4.wsdoc";   // [-]

    std::uint32_t BooleanMode     = 0u;              // [-] - union, subtract, intersect
    bool          Suppressed      = false;           // [-]
    double        NestedTally     = 0.0;             // [ct]

    std::uint32_t PlaneChoice     = 0u;              // [-] - sketch
    double        ConstraintTally = 12.0;            // [ct]
    double        CurveTally      = 8.0;             // [ct]
    bool          FullyConstrained = false;          // [-]
    double        GridSnap        = 0.5;             // [mm]

    double        ExtrudeDepth    = 12.5;            // [mm] - solid
    double        DraftAngle      = 0.0;             // [deg]
    double        WallThickness   = 2.5;             // [mm]
    bool          CappedEnds      = true;            // [-]

    double        Radius          = 6.25;            // [mm] - cylinder / sphere
    double        Height          = 18.0;            // [mm] - cylinder
    double        SegmentTally    = 32.0;            // [ct]
    double        RingTally       = 24.0;            // [ct] - sphere

    double        SweepAngle      = 360.0;           // [deg] - revolve
    std::uint32_t AxisChoice      = 1u;              // [-]
    bool          ProfileClosed   = true;            // [-]

    double        SectionTally    = 3.0;             // [ct] - loft
    double        TangencyStart   = 0.0;             // [-]
    double        TangencyEnd     = 0.0;             // [-]
    bool          Ruled           = false;           // [-]
};

/// 🧩 Seats the profile at the reference's establishProfile defaults for the classification.
/// cost  ✔️
/// tag   api, nonallocating, nonthrowing
void SeatProfile(ProfileOrdinates& Profile, const OutlinerRowDeclaration& Row);

//------------------------------------------------------------------------------------------------------------------------
//                                                       THE REVISIONS
//------------------------------------------------------------------------------------------------------------------------

/// 🧩 Which revision category an entry declares — carries the subtitle glyph and its hue.
/// tag   contract
enum class RevisionCategory : std::uint32_t
{
    Start      = 0u,   // [-] - Created
    Feature    = 1u,   // [-] - #ffb24d
    Parameter  = 2u,   // [-] - #4fd18b
    Sketch     = 3u,   // [-] - #37d6d6
    Relocate   = 4u,   // [-] - #5b8cff
    Grouping   = 5u,   // [-] - #b98bff
    Create     = 6u,   // [-] - #7ec8ff
    Edit       = 7u,   // [-] - #c99b6a
    Drop       = 8u,   // [-] - #ff6b6b
    CategoryCount = 9u
};

/// 🧩 One revision entry, borrowed runs with host-owned fold disclosure.
/// tag   contract, nonallocating, nonthrowing
struct RevisionDeclaration
{
    const char*      Identity   = "";    // [-] - borrowed; the record token
    RevisionCategory Category   = RevisionCategory::Edit;
    const char*      TitleRun   = "";    // [-] - borrowed
    const char*      SubtitleRun = "";   // [-] - borrowed
    const char*      CommentRun = "";    // [-] - borrowed; may stand vacated
    const char*      AuthorRun  = "";    // [-] - borrowed
    const char*      EditRun    = "";    // [-] - borrowed; may stand vacated
    const char*      ClockRun   = "";    // [-] - borrowed; the time run
    const char*      DateRun    = "";    // [-] - borrowed; the date run
    bool*            Expanded   = nullptr;   // [-] - host-owned fold
};

/// 🧩 The revision category's caption and subtitle glyph.
/// cost  ✔️
/// tag   api, nonallocating, nonthrowing
const char* RevisionCategoryRun(RevisionCategory Category);

/// 🧩 The revision category's subtitle tint, verbatim from REVISION_HUE.
/// cost  ✔️
/// tag   api, nonallocating, nonthrowing
InkOrdinate RevisionCategoryTint(RevisionCategory Category);

//------------------------------------------------------------------------------------------------------------------------
//                                                     THE INSPECTOR
//------------------------------------------------------------------------------------------------------------------------

/// 🧩 The record inspector — head, the Properties / History carousel, fold cards, revision stack, field foot.
/// tag   contract, nonallocating, nonthrowing
class PropertiesPanel
{
public:

    /// 🧩 Presents the inspector for one declared row inside the seat extent, both carousel pages.
    /// in    Declared   [-]  the inspected row; nullptr presents the vacated seat
    /// in    Revisions  [-]  borrowed; the whole revision record, filtered to the declared walk
    /// tag   api, nonallocating, nonthrowing
    void Advance(PanelExchange& Surface, const PlaneExtent& Seat, const OutlinerRowDeclaration* Declared,
                 ProfileOrdinates& Profile, const IconDepot& Depot,
                 const RevisionDeclaration* Revisions = nullptr, std::uint32_t RevisionCount = 0u,
                 const OutlinerRowDeclaration* Forest = nullptr, std::uint32_t ForestCount = 0u);

    std::uint32_t CarouselMode = 0u;   // [-] - 0 properties, 1 history
    bool          BackRaised   = false;   // [-] - the back action edge

private:

    /// 🧩 Presents the history page — the revision stack for the declared row and everything it encloses.
    /// tag   internal, nonallocating, nonthrowing
    void PresentHistory(PanelExchange& Surface, const PlaneExtent& Seat, const OutlinerRowDeclaration* Declared,
                        const RevisionDeclaration* Revisions, std::uint32_t RevisionCount,
                        const OutlinerRowDeclaration* Forest, std::uint32_t ForestCount, const IconDepot& Depot);

    float ScrollAcross = 0.0f;   // [px] - body scroll ordinate
};

}   // namespace Rift
