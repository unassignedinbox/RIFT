//============================================================================================================================================
//                                                         RECORDINGSURFACE.H
//============================================================================================================================================
// 🧩 Primitives in, recorded commands out — the standalone drawing seam over the vendored interface library, with no ImGui spelling in the Api.

#pragma once

#include "Engine/Contract/Api/DeliveryContract.h"
#include "Engine/SlateUI/Interface/ThemeSpecification/Api/ThemeSpecification.h"

#include <cstdint>

namespace Slate
{

//------------------------------------------------------------------------------------------------------------------------
//                                                     THE PLANE EXTENT
//------------------------------------------------------------------------------------------------------------------------

/// 🧩 One axis-aligned extent in display pixels, stated as its two corners.
/// note  The ordinate increases downward, as the display does.
/// tag   contract, nonallocating, nonthrowing
struct PlaneExtent
{
    float  LeastAlong  = 0.0f;   // [px] - leading edge
    float  LeastAcross = 0.0f;   // [px] - upper edge
    float  MostAlong   = 0.0f;   // [px] - trailing edge
    float  MostAcross  = 0.0f;   // [px] - lower edge

    constexpr float SpanAlong() const   { return MostAlong  - LeastAlong;  }
    constexpr float SpanAcross() const  { return MostAcross - LeastAcross; }

    constexpr bool Encloses(float Along, float Across) const
    {
        return Along >= LeastAlong && Along < MostAlong && Across >= LeastAcross && Across < MostAcross;
    }

    constexpr PlaneExtent Inset(float Along, float Across) const
    {
        return PlaneExtent{ LeastAlong + Along, LeastAcross + Across, MostAlong - Along, MostAcross - Across };
    }
};

/// 🧩 Constructs an extent from an origin and a span.
/// cost  ✔️
constexpr PlaneExtent Spanning(float Along, float Across, float ExtentAlong, float ExtentAcross)
{
    return PlaneExtent{ Along, Across, Along + ExtentAlong, Across + ExtentAcross };
}

/// 🧩 The across ordinate that centres a run of the given extent inside a row.
/// cost  ✔️
constexpr float CentredAcross(const PlaneExtent& Extent, float RunExtent)
{
    return Extent.LeastAcross + (Extent.SpanAcross() - RunExtent) * 0.5f;
}

/// 🧩 Which corners of a rounded extent carry the radius.
/// tag   contract
enum class CornerSelection : std::uint32_t
{
    None          = 0u,   // [-] - square
    All           = 1u,   // [-] - every corner
    Upper         = 2u,   // [-] - the two upper corners
    Lower         = 3u,   // [-] - the two lower corners
    UpperLeading  = 4u,   // [-] - the upper leading corner only
    UpperTrailing = 5u,   // [-] - the upper trailing corner only
    LowerLeading  = 6u,   // [-] - the lower leading corner only
    LowerTrailing = 7u    // [-] - the lower trailing corner only
};

//------------------------------------------------------------------------------------------------------------------------
//                                                    THE RECORDING SEAM
//------------------------------------------------------------------------------------------------------------------------

/// 🧩 The standalone recording seam: rounded grounds, hair edges, text runs and glyph strokes recorded into
///       the vendored interface library's draw list, with the pointer sampled once per tick.
/// note  The Api names no ImGui spelling; the Source is the one place the vendored library is addressed,
///       exactly as the engine's SlateUI unit is the only unit permitted to reference it.
/// tag   contract, nonallocating, nonthrowing
class RecordingSurface
{
public:

    /// 🧩 Which of the two shell layers a tick's recording is laid into.
    /// tag   contract
    enum class ShellLayer : std::uint32_t
    {
        Beneath      = 0u,   // [-] - behind every window; the workspace ground
        Above        = 1u,   // [-] - in front of all windows; overlays
        LayerCount   = 2u    // [-] - the closed count, never a layer
    };

    /// 🧩 The pointer condition sampled at adoption.
    /// tag   contract, nonallocating, nonthrowing
    struct PointerCondition
    {
        float  Along          = -1.0e6f;   // [px] - pointer along ordinate
        float  Across         = -1.0e6f;   // [px] - pointer across ordinate
        bool   PrimaryDown    = false;     // [-]  - primary button stands down
        bool   PrimaryPressed = false;     // [-]  - primary button edge, pressed this tick
        bool   SecondaryDown  = false;     // [-]  - secondary button stands down
    };

    RecordingSurface()                                = default;
    RecordingSurface(const RecordingSurface&)         = delete;
    RecordingSurface& operator=(const RecordingSurface&) = delete;
    ~RecordingSurface()                               = default;

    /// 🧩 Opens a tick's recording against one shell layer and samples the arrived pointer.
    /// out   Deliver  [-]  refuses with CapabilityAbsent when no interface context is current
    /// cost  ✔️
    /// tag   api, nonallocating, nonthrowing
    Deliver<bool> Adopt(ShellLayer Layer = ShellLayer::Beneath);

    /// 🧩 Closes the standing tick's recording. A no-op when nothing stands adopted.
    /// cost  ✔️
    /// tag   api, nonallocating, nonthrowing
    void Seal();

    /// 🧩 The pointer condition sampled at the last adoption.
    /// cost  ✔️
    /// tag   api, nonallocating, nonthrowing
    const PointerCondition& Pointer() const   { return Arrived; }

    /// 🧩 A pointer arrival fraction for a row: one when the pointer rests inside the extent.
    /// cost  ✔️
    /// tag   api, nonallocating, nonthrowing
    bool PointerWithin(const PlaneExtent& Extent) const
    {
        return Arrived.Along >= Extent.LeastAlong && Arrived.Along < Extent.MostAlong &&
               Arrived.Across >= Extent.LeastAcross && Arrived.Across < Extent.MostAcross;
    }

    // ═══ Recording primitives ═══

    /// 🧩 Records one rounded, filled ground.
    /// tag   api, nonallocating, nonthrowing
    void Ground(const PlaneExtent& Extent, const InkOrdinate& Ink, float Radius, CornerSelection Corners = CornerSelection::All);

    /// 🧩 Records one rounded hair edge stroked inside the extent.
    /// tag   api, nonallocating, nonthrowing
    void Edge(const PlaneExtent& Extent, const InkOrdinate& Ink, float Thickness, float Radius, CornerSelection Corners = CornerSelection::All);

/// 🧩 Records one vertical scrim, blended from the upper ink to the lower ink.
/// tag   api, nonallocating, nonthrowing
void Scrim(const PlaneExtent& Extent, const InkOrdinate& Upper, const InkOrdinate& Lower);

/// 🧩 Records one horizontal scrim, blended from the leading ink to the trailing ink.
/// tag   api, nonallocating, nonthrowing
void ScrimAlong(const PlaneExtent& Extent, const InkOrdinate& Leading, const InkOrdinate& Trailing);

    /// 🧩 Records one text run at the given presentation size, snapped to the nearest crisp size.
    /// tag   api, nonallocating, nonthrowing
    void TextRun(float Along, float Across, const char* Run, const InkOrdinate& Ink, float Size = 13.0f);

    /// 🧩 Records one text run clipped to an extent, trailing-ellipsised when it exceeds the span.
    /// tag   api, nonallocating, nonthrowing
    void TextRunClipped(float Along, float Across, const char* Run, const InkOrdinate& Ink, float Size, float ExtentAlong);

    /// 🧩 Measures one run at the given presentation size.
    /// cost  ✔️
    /// tag   api, nonallocating, nonthrowing
    float MeasureRun(const char* Run, float Size = 13.0f) const;

    /// 🧩 The crisp line extent of the nearest presentation size.
    /// cost  ✔️
    /// tag   api, nonallocating, nonthrowing
    float RunExtent(float Size = 13.0f) const;

    /// 🧩 The along ordinate that centres a run inside an extent.
    /// cost  ✔️
    /// tag   api, nonallocating, nonthrowing
    float CentredAlong(const PlaneExtent& Extent, const char* Run, float Size = 13.0f) const;

    /// 🧩 Records one filled disc.
    /// tag   api, nonallocating, nonthrowing
    void Medallion(float CentreAlong, float CentreAcross, float Radius, const InkOrdinate& Ink);

    /// 🧩 Records one tinted picture — the rasterised glyph an IconDepot seats — inside a rounded extent.
    /// in    VendorIdentity  [-]  the depot-issued picture identity
    /// tag   api, nonallocating, nonthrowing
    void Picture(const PlaneExtent& Extent, void* VendorIdentity, const InkOrdinate& Tint, float Radius, CornerSelection Corners = CornerSelection::All);

    /// 🧩 Records one disc outline.
    /// tag   api, nonallocating, nonthrowing
    void Ring(float CentreAlong, float CentreAcross, float Radius, float Thickness, const InkOrdinate& Ink);

    /// 🧩 Records one straight stroke between two ordinates.
    /// tag   api, nonallocating, nonthrowing
    void Stroke(float AlongA, float AcrossA, float AlongB, float AcrossB, float Thickness, const InkOrdinate& Ink);

    /// 🧩 Records one horizontal or vertical hair rule.
    /// tag   api, nonallocating, nonthrowing
    void Rule(float Along, float Across, float ExtentAlong, float Thickness, const InkOrdinate& Ink);

    // ═══ Affordance glyphs (drawn strokes, not icon assets) ═══

    /// 🧩 Records one chevron, rotated to point down when Down, right otherwise.
    /// tag   api, nonallocating, nonthrowing
    void Chevron(float CentreAlong, float CentreAcross, float Radius, const InkOrdinate& Ink, bool Down);

    /// 🧩 Records the eye affordance — closed with a strike when Absent.
    /// tag   api, nonallocating, nonthrowing
    void EyeGlyph(float CentreAlong, float CentreAcross, float Radius, const InkOrdinate& Ink, bool Absent);

    /// 🧩 Records the search affordance — a disc with a handle.
    /// tag   api, nonallocating, nonthrowing
    void SearchGlyph(float CentreAlong, float CentreAcross, float Radius, const InkOrdinate& Ink);

    /// 🧩 Records the plus affordance.
    /// tag   api, nonallocating, nonthrowing
    void PlusGlyph(float CentreAlong, float CentreAcross, float Radius, const InkOrdinate& Ink);

    /// 🧩 Records the cross affordance (plus rotated 45 degrees).
    /// tag   api, nonallocating, nonthrowing
    void CrossGlyph(float CentreAlong, float CentreAcross, float Radius, const InkOrdinate& Ink);

    /// 🧩 Records the check affordance.
    /// tag   api, nonallocating, nonthrowing
    void CheckGlyph(float CentreAlong, float CentreAcross, float Radius, const InkOrdinate& Ink);

    /// 🧩 Records the trash affordance — a bin outline with two uprights.
    /// tag   api, nonallocating, nonthrowing
    void TrashGlyph(float CentreAlong, float CentreAcross, float Radius, const InkOrdinate& Ink);

private:

    PointerCondition  Arrived;              // [-] - sampled at adoption
    void*             Recording   = nullptr;   // [-] - the vendored draw list, untyped in the Api
    bool              Standing    = false;     // [-] - whether a tick stands adopted
};

}   // namespace Slate
