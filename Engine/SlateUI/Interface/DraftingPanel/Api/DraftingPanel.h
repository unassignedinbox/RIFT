//============================================================================================================================================
//                                                          DRAFTINGPANEL.H
//============================================================================================================================================
// 🧩 The CAD panel's drafting composition — the scene directory beside the metadata pane, transcribed from page.tsx's drafting seat.

#pragma once

#include "Engine/Contract/Api/DeliveryContract.h"
#include "Engine/SlateUI/Interface/IconDepot/Api/IconDepot.h"
#include "Engine/SlateUI/Interface/OutlinerPanel/Api/OutlinerPanel.h"
#include "Engine/SlateUI/Interface/PropertiesPanel/Api/PropertiesPanel.h"
#include "Engine/SlateUI/Interface/RecordingSurface/Api/RecordingSurface.h"

#include <cstdint>

namespace Slate
{

//------------------------------------------------------------------------------------------------------------------------
//                                                     THE DRAFTING SEAT
//------------------------------------------------------------------------------------------------------------------------

/// 🧩 The drafting composition — directory column, the Properties & Actions bar, the metadata pane beneath it.
/// note  The directory column is the very OutlinerPanel the standalone host presents, reused as the CAD
///       panel's scene director — one outliner, two seats.
/// tag   contract, nonallocating, nonthrowing
class DraftingPanel
{
public:

    /// 🧩 Presents the drafting composition inside the seat extent, one tick.
    /// tag   api, nonallocating, nonthrowing
    void Advance(RecordingSurface& Surface, const PlaneExtent& Seat, OutlinerPanel& Directory,
                 const OutlinerRowDeclaration* Rows, std::uint32_t RowCount,
                 const OutlinerRowDeclaration* Inspected, ProfileOrdinates& Profile, const IconDepot& Depot);

    /// 🧩 Raised for one tick when the metadata pane's advance action is pressed.
    /// cost  ✔️
    /// tag   api, nonallocating, nonthrowing
    bool AdvanceRaised = false;   // [-] - the Properties & History action edge

private:

    /// 🧩 Presents the metadata pane — hero, stats, albedo, the advance action, the action list, the foot.
    /// tag   internal, nonallocating, nonthrowing
    void PresentMetadata(RecordingSurface& Surface, const PlaneExtent& Seat, const OutlinerRowDeclaration* Declared,
                         const ProfileOrdinates& Profile, const IconDepot& Depot);
};

}   // namespace Slate
