//============================================================================================================================================
//                                                             CADPANEL.H
//============================================================================================================================================
// 🧩 The Frontier CAD workspace — tab strip, header, browser dock, model stage, inspector dock — transcribed from References/Cad.

#pragma once

#include "Engine/Contract/Api/DeliveryContract.h"
#include "Engine/SlateUI/Interface/IconDepot/Api/IconDepot.h"
#include "Engine/SlateUI/Interface/RecordingSurface/Api/RecordingSurface.h"
#include "Engine/SlateUI/Interface/ThemeSpecification/Api/ThemeSpecification.h"

#include <cstdint>

namespace Slate
{

//------------------------------------------------------------------------------------------------------------------------
//                                                    THE BROWSER ROWS
//------------------------------------------------------------------------------------------------------------------------

/// 🧩 Which constituent of the part one browser row declares.
/// tag   contract
enum class BrowserClassification : std::uint32_t
{
    Datums    = 0u,   // [-] - the Origin enclosure
    Plane     = 1u,   // [-] - a construction plane
    Enclosure = 2u,   // [-] - Sketches, Bodies
    Solid     = 3u,   // [-] - a solid body
    Surface   = 4u    // [-] - a surface body
};

/// 🧩 One browser row declaration, borrowed, with host-owned disclosure and presence.
/// tag   contract, nonallocating, nonthrowing
struct BrowserRowDeclaration
{
    const char*                  Caption        = "";    // [-] - borrowed
    BrowserClassification        Classification = BrowserClassification::Enclosure;
    std::uint32_t                Swatch         = 0u;    // [-] - 0xRRGGBB, zero for none
    const char*                  AxisRun        = "";    // [-] - borrowed; the plane's axis pair
    bool*                        Expanded       = nullptr;   // [-] - host-owned
    bool*                        Hidden         = nullptr;   // [-] - host-owned
    bool*                        Locked         = nullptr;   // [-] - host-owned, for the Origin datums
    const BrowserRowDeclaration* Enclosed       = nullptr;   // [-] - borrowed forest
    std::uint32_t                EnclosureCount = 0u;       // [-]
};

/// 🧩 The assembled part composition the CAD workspace seats.
/// tag   contract, nonallocating, nonthrowing
struct CadComposition
{
    const char* PartCaption  = "Part01";   // [-] - borrowed
    const char* TabCaption   = "Part01";   // [-] - borrowed
    std::uint32_t BodyCount  = 0u;         // [-] - solid and surface bodies
    std::uint32_t FeatureCount = 0u;       // [-] - timeline features
};

//------------------------------------------------------------------------------------------------------------------------
//                                                    THE CAD WORKSPACE
//------------------------------------------------------------------------------------------------------------------------

/// 🧩 The whole CAD workspace seat — every dock, the stage and the condition strip, one tick.
/// note  The left dock seats the browser and the tools; the right dock seats properties, the feature
///       timeline and the revision list; the carousel segments switch each.
/// tag   contract, nonallocating, nonthrowing
class CadWorkspacePanel
{
public:

    /// 🧩 Presents the workspace inside the seat extent, one tick.
    /// tag   api, nonallocating, nonthrowing
    void Advance(RecordingSurface& Surface, const PlaneExtent& Seat, const BrowserRowDeclaration* Rows, std::uint32_t RowCount,
                 const CadComposition& Composition, const IconDepot& Depot);

private:

    /// 🧩 Presents one browser row and its forest.
    /// tag   internal, nonallocating, nonthrowing
    void PresentBrowserRow(RecordingSurface& Surface, const PlaneExtent& Body, const BrowserRowDeclaration& Row,
                           std::uint32_t Depth, const IconDepot& Depot, float& CursorAcross);

    /// 🧩 Presents the model stage — dot lattice, origin triad, pills, display bar, condition strip.
    /// tag   internal, nonallocating, nonthrowing
    void PresentStage(RecordingSurface& Surface, const PlaneExtent& Stage, const CadComposition& Composition, const IconDepot& Depot);

    bool           LeftDockOpen     = true;    // [-] - the browser dock stands open
    bool           RightDockOpen    = true;    // [-] - the inspector dock stands open
    std::uint32_t  LeftCarousel     = 0u;      // [-] - 0 browser, 1 tools
    std::uint32_t  RightCarousel    = 0u;      // [-] - 0 properties, 1 features, 2 revision
    char           TakenIdentity[24] = "";     // [-] - the taken browser row
    float          BrowserScroll    = 0.0f;    // [px] - browser scroll ordinate
    std::uint32_t  PresentedRows    = 0u;      // [-]  - rows presented this tick
};

}   // namespace Slate
