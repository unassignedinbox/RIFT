//============================================================================================================================================
//                                                          OUTLINERPANEL.H
//============================================================================================================================================
// 🧩 The scene directory — the CAD panel's outliner, transcribed from DirectoryPane.tsx — declarations in, one recorded tree out.

#pragma once

#include "Contract/Api/PanelContract.h"
#include "SlateUI/Interface/IconDepot/Api/IconDepot.h"
#include "SlateUI/Interface/PanelExchange/Api/PanelExchange.h"
#include "SlateUI/Interface/ThemeSpecification/Api/ThemeSpecification.h"

#include <cstdint>

namespace Rift
{

//------------------------------------------------------------------------------------------------------------------------
//                                                   THE CLASSIFICATIONS
//------------------------------------------------------------------------------------------------------------------------

/// 🧩 Which constituent of the part one directory row declares — drives the dummy glyph's tint.
/// tag   contract, nonallocating, nonthrowing
enum class DirectoryClassification : std::uint32_t
{
    Scene              = 0u,   // [-] - #7ec8ff  `scene`
    Enclosure          = 1u,   // [-] - #b98bff  `folder`
    Sketch             = 2u,   // [-] - #37d6d6
    Solid              = 3u,   // [-] - #ffb24d
    Cylinder           = 4u,   // [-] - #4fd18b
    Sphere             = 5u,   // [-] - #ff7ab8
    Cone               = 6u,   // [-] - #ff6b6b
    Revolve            = 7u,   // [-] - #c99b6a
    Loft               = 8u,   // [-] - #5b8cff
    ClassificationCount = 9u   // [-] - the closed count, never a classification
};

/// 🧩 The tint a classification carries, verbatim from CLASSIFICATION_HUE.
/// cost  ✔️
/// tag   api, nonallocating, nonthrowing
InkOrdinate ClassificationTint(DirectoryClassification Classification);

/// 🧩 The caption a classification spells in the inspector and metadata panes (CLASSIFICATION_LABEL).
/// cost  ✔️
/// tag   api, nonallocating, nonthrowing
const char* ClassificationLabel(DirectoryClassification Classification);

/// 🧩 The two-letter abbreviation a history group spells (CLASSIFICATION_ABBR).
/// cost  ✔️
/// tag   api, nonallocating, nonthrowing
const char* ClassificationAbbr(DirectoryClassification Classification);

//------------------------------------------------------------------------------------------------------------------------
//                                                    THE ROW DECLARATION
//------------------------------------------------------------------------------------------------------------------------

/// 🧩 One row the directory seats — borrowed caption, classification, host-owned disclosure and presence,
///       and the enclosed forest beneath it. No presented datum is owned.
/// tag   contract, nonallocating, nonthrowing
struct OutlinerRowDeclaration
{
    const char*                   Caption         = "";           // [-] - borrowed; outlives the tick
    const char*                   Identity        = "";           // [-] - borrowed; the record token
    DirectoryClassification       Classification  = DirectoryClassification::Solid;
    bool*                         Expanded        = nullptr;      // [-] - host-owned disclosure
    bool*                         Hidden          = nullptr;      // [-] - host-owned presence
    const OutlinerRowDeclaration* Enclosed        = nullptr;      // [-] - borrowed forest beneath
    std::uint32_t                 EnclosureCount  = 0u;           // [-] - rows directly enclosed
};

/// 🧩 The composition caption pair the head carries.
/// tag   contract, nonallocating, nonthrowing
struct OutlinerComposition
{
    const char* TitleRun   = "Directory";     // [-] - borrowed
    const char* ContextRun = "Bracket_Rev4";  // [-] - borrowed
};

//------------------------------------------------------------------------------------------------------------------------
//                                                     THE DIRECTORY
//------------------------------------------------------------------------------------------------------------------------

/// 🧩 The scene directory — head, retention field, the disclosure forest, the count foot.
/// note  Retention retains a row when its caption carries the run case-insensitively, or any enclosed row
///       is retained; while a run stands, every branch presents open. Selection is additive under the
///       control gesture, exactly as the reference's ctrl-click toggles tokens in the selection set.
/// tag   contract, nonallocating, nonthrowing
class OutlinerPanel
{
public:

    static constexpr std::uint32_t SelectionCapacity = 8u;   // [-] - additive selection ceiling

    OutlinerPanel()                                = default;
    OutlinerPanel(const OutlinerPanel&)            = delete;
    OutlinerPanel& operator=(const OutlinerPanel&) = delete;
    ~OutlinerPanel()                               = default;

    /// 🧩 Presents the directory inside the seat extent, one tick.
    /// tag   api, nonallocating, nonthrowing
    void Advance(PanelExchange& Surface, const PlaneExtent& Seat,
                 const OutlinerRowDeclaration* Rows, std::uint32_t RowCount,
                 const OutlinerComposition& Composition, const IconDepot& Depot);

    /// 🧩 The retention run, host-readable and host-writable.
    /// cost  ✔️
    /// tag   api, nonallocating, nonthrowing
    char RetentionRun[64] = "";   // [-] - the live run

    /// 🧩 The taken tokens, in take order; the head pill spells the count past one.
    /// cost  ✔️
    /// tag   api, nonallocating, nonthrowing
    char TakenIdentities[SelectionCapacity][16] = {};   // [-] - taken record tokens
    std::uint32_t TakenCount = 0u;                       // [-] - standing selection size

    /// 🧩 Raised for one tick when a row's inspect gesture (double press) lands.
    /// cost  ✔️
    /// tag   api, nonallocating, nonthrowing
    bool InspectRaised = false;   // [-] - double-press edge

    /// 🧩 Whether the token stands taken.
    /// cost  ✔️
    /// tag   api, nonallocating, nonthrowing
    bool TokenTaken(const char* Identity) const;

    /// 🧩 Seats exactly one taken token.
    /// cost  ✔️
    /// tag   api, nonallocating, nonthrowing
    void SeatTaken(const char* Identity);

private:

    /// 🧩 Presents one row and its enclosed forest.
    /// tag   internal, nonallocating, nonthrowing
    void PresentRow(PanelExchange& Surface, const PlaneExtent& Body, const OutlinerRowDeclaration& Row,
                    std::uint32_t Depth, bool RetentionStanding, const IconDepot& Depot);

    /// 🧩 Whether the row, or any row it encloses, carries the retention run.
    /// cost  🚩
    /// tag   internal, nonallocating, nonthrowing
    bool Retained(const OutlinerRowDeclaration& Row) const;

    const IconDepot* Depot          = nullptr;   // [-] - the dummy glyph depot
    float            ScrollAcross   = 0.0f;      // [px] - body scroll ordinate
    std::uint32_t    PresentedCount = 0u;        // [-]  - rows presented this tick
};

}   // namespace Rift
