//============================================================================================================================================
//                                                            ICONDEPOT.CPP
//============================================================================================================================================
// 🧩 Rasterises the dummy glyph's SVG geometry — rounded square, centred dot — into one tintable picture.

#include "Engine/SlateUI/Interface/IconDepot/Api/IconDepot.h"

#include "imgui.h"

#include <cmath>

namespace Slate
{

namespace
{

constexpr float GlyphMargin = 4.0f / 24.0f;   // [-] - the SVG rect origin 4 of 24
constexpr float GlyphEdge   = 16.0f / 24.0f;   // [-] - the SVG rect extent 16 of 24
constexpr float GlyphRadius = 4.5f / 24.0f;   // [-] - the SVG rect rx 4.5 of 24
constexpr float DotRadius   = 2.7f / 24.0f;   // [-] - the SVG circle r 2.7 of 24
constexpr float StrokeHalf  = 1.0f  / 24.0f;   // [-] - half the SVG stroke-width 2

/// 🧩 Signed distance to the rounded-square outline the SVG declares.
/// out   distance  [uv]  negative inside the stroke, zero on its centre line
/// cost  🚩
float RoundedSquareDistance(float Along, float Across)
{
    const float SquareExtent = GlyphEdge - GlyphMargin;   // [-] - rect extent in uv
    const float Half         = SquareExtent * 0.5f;
    const float Centre       = GlyphMargin + Half;

    const float dx = std::fabs(Along - Centre) - (Half - GlyphRadius);
    const float dy = std::fabs(Across - Centre) - (Half - GlyphRadius);
    const float Outside = std::sqrt(dx > 0.0f ? dx * dx : 0.0f + 0.0f) ;   // [uv] - placeholder, replaced below
    (void)Outside;

    const float ax = dx > 0.0f ? dx : 0.0f;
    const float ay = dy > 0.0f ? dy : 0.0f;
    const float CornerDistance = std::sqrt(ax * ax + ay * ay) - GlyphRadius;
    const float Interior       = std::fmax(dx, dy);
    const float Distance       = Interior > 0.0f ? CornerDistance : Interior;
    return std::fabs(Distance) - StrokeHalf;
}

/// 🧩 Signed distance to the centred dot the SVG declares.
/// cost  🚩
float DotDistance(float Along, float Across)
{
    const float Centre = 0.5f;
    const float dx = Along - Centre;
    const float dy = Across - Centre;
    return std::sqrt(dx * dx + dy * dy) - DotRadius;
}

}   // namespace

//------------------------------------------------------------------------------------------------------------------------
//                                                      CONSTRUCTION
//------------------------------------------------------------------------------------------------------------------------

Deliver<bool> IconDepot::Construct()
{
    ImGuiIO& VendorIO = ImGui::GetIO();
    if (VendorIO.Fonts == nullptr)
        return Deliver<bool>::Refuse({ RefusalReason::CapabilityAbsent, "no font atlas stands constructed" });

    // ① Rasterise at fourfold oversampling, white premultiplied by coverage, then box-filter down.
    constexpr std::uint32_t Oversample = 4u;
    const std::uint32_t Coarse = GlyphExtent;
    const std::uint32_t Fine   = Coarse * Oversample;

    std::vector<float> Coverage(Fine * Fine, 0.0f);
    for (std::uint32_t Across = 0u; Across < Fine; ++Across)
    {
        for (std::uint32_t Along = 0u; Along < Fine; ++Along)
        {
            float Accumulated = 0.0f;
            for (std::uint32_t Sub = 0u; Sub < Oversample * Oversample; ++Sub)
            {
                const float SampleAlong = (Along * Oversample + (Sub % Oversample) + 0.5f) / Fine;
                const float SampleAcross = (Across * Oversample + (Sub / Oversample) + 0.5f) / Fine;
                const float Square = RoundedSquareDistance(SampleAlong, SampleAcross);
                const float Dot    = DotDistance(SampleAlong, SampleAcross);
                const float Nearest = Square < Dot ? Square : Dot;
                Accumulated += (Nearest < 0.0f) ? 1.0f : (Nearest < 1.0f ? 1.0f - Nearest : 0.0f);
            }
            Coverage[Across * Fine + Along] = Accumulated / (Oversample * Oversample);
        }
    }

    RasterOrdinates.assign(static_cast<std::size_t>(Coarse) * Coarse * 4u, 0u);
    for (std::uint32_t Across = 0u; Across < Coarse; ++Across)
    {
        for (std::uint32_t Along = 0u; Along < Coarse; ++Along)
        {
            float Accumulated = 0.0f;
            for (std::uint32_t SubAcross = 0u; SubAcross < Oversample; ++SubAcross)
                for (std::uint32_t SubAlong = 0u; SubAlong < Oversample; ++SubAlong)
                    Accumulated += Coverage[(Across * Oversample + SubAcross) * Fine + (Along * Oversample + SubAlong)];

            const float Fraction = Accumulated / (Oversample * Oversample);
            const std::size_t Seat = (static_cast<std::size_t>(Across) * Coarse + Along) * 4u;
            RasterOrdinates[Seat + 0u] = 255u;
            RasterOrdinates[Seat + 1u] = 255u;
            RasterOrdinates[Seat + 2u] = 255u;
            RasterOrdinates[Seat + 3u] = static_cast<std::uint8_t>(Fraction * 255.0f + 0.5f);
        }
    }

    GlyphSeat = static_cast<void*>(RasterOrdinates.data());   // [-] - stable, unique picture identity; the raster codec resolves it
    return Deliver<bool>::Delivered(true);
}

void IconDepot::PresentGlyph(RecordingSurface& Surface, const PlaneExtent& Seat, const InkOrdinate& Tint) const
{
    const float Edge = Seat.SpanAlong() < Seat.SpanAcross() ? Seat.SpanAlong() : Seat.SpanAcross();
    if (Edge <= 1.0f)
        return;
    const float Along  = Seat.LeastAlong + (Seat.SpanAlong() - Edge) * 0.5f;
    const float Across = Seat.LeastAcross + (Seat.SpanAcross() - Edge) * 0.5f;
    Surface.Picture(Spanning(Along, Across, Edge, Edge), GlyphSeat, Tint, Edge * 0.18f);
}

void IconDepot::PresentGlyphCentred(RecordingSurface& Surface, float CentreAlong, float CentreAcross, float EdgeExtent, const InkOrdinate& Tint) const
{
    Surface.Picture(Spanning(CentreAlong - EdgeExtent * 0.5f, CentreAcross - EdgeExtent * 0.5f, EdgeExtent, EdgeExtent),
                    GlyphSeat, Tint, EdgeExtent * 0.18f);
}

}   // namespace Slate
