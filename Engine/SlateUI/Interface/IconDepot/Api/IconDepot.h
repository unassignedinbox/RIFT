//============================================================================================================================================
//                                                             ICONDEPOT.H
//============================================================================================================================================
// 🧩 The single dummy glyph every classification icon seat presents — rasterised once from its SVG geometry, tinted per classification.

#pragma once

#include "Contract/Api/PanelContract.h"
#include "SlateUI/Interface/PanelExchange/Api/PanelExchange.h"
#include "SlateUI/Interface/ThemeSpecification/Api/ThemeSpecification.h"

#include <cstdint>
#include <vector>

namespace Rift
{

//------------------------------------------------------------------------------------------------------------------------
//                                                     THE DUMMY GLYPH
//------------------------------------------------------------------------------------------------------------------------

/// 🧩 The picture depot: holds the one dummy glyph the reference panels seat in every classification icon
///       slot, rasterised once from `Api/DummyGlyph.svg` and tinted at presentation time.
/// note  🔴 No icon artwork is authored, generated or imported — the dummy glyph is the single placeholder,
///       and classification is carried by the tint, exactly as the seats below it carry no identity either.
///       Affordance glyphs (chevrons, eyes, plus, search) are drawn strokes on the PanelExchange, not
///       depot pictures.
/// tag   contract, nonallocating, nonthrowing
class IconDepot
{
public:

    static constexpr std::uint32_t GlyphExtent = 96u;   // [px] - the rasterised glyph edge

    IconDepot()                          = default;
    IconDepot(const IconDepot&)          = delete;
    IconDepot& operator=(const IconDepot&) = delete;
    ~IconDepot()                         = default;

    /// 🧩 Rasterises the dummy glyph into the atlas the vendored library holds.
    /// out   Deliver  [-]  refuses with CapabilityAbsent when no interface context is current
    /// cost  🚩
    /// tag   api, nonthrowing
    Deliver<bool> Construct();

    /// 🧩 Adopts a renderer-issued picture identity over the raster-issued one.
    /// note  A windowed host uploads the glyph once and hands its platform handle here; the headless
    ///       codec resolves the raster-issued identity against the depot ordinates directly.
    /// cost  ✔️
    /// tag   api, nonallocating, nonthrowing
    void AdoptIdentity(void* ArrivingIdentity)   { GlyphSeat = ArrivingIdentity; }

    /// 🧩 Seats the vector presentation — the glyph is drawn as primitives, no picture seat at all.
    /// note  The engine's own icons are vector-drawn; the windowed seat rides that same grain.
    /// cost  ✔️
    /// tag   api, nonallocating, nonthrowing
    void SeatVectorGlyph()   { GlyphSeat = nullptr; }



    /// 🧩 The vendor picture identity of the rasterised dummy glyph, for Picture recordings.
    /// cost  ✔️
    /// tag   api, nonallocating, nonthrowing
    void* GlyphIdentity() const   { return GlyphSeat; }

    /// 🧩 The rasterised RGBA ordinates of the glyph, for the raster codec to resolve.
    /// cost  ✔️
    /// tag   api, nonallocating, nonthrowing
    const std::uint8_t* PictureOrdinates() const   { return RasterOrdinates.data(); }

    /// 🧩 Presents the dummy glyph, tinted, fitted and centred inside the seat extent.
    /// tag   api, nonallocating, nonthrowing
    void PresentGlyph(PanelExchange& Surface, const PlaneExtent& Seat, const InkOrdinate& Tint) const;

    /// 🧩 Presents the dummy glyph, tinted, centred on a point at the given edge extent.
    /// tag   api, nonallocating, nonthrowing
    void PresentGlyphCentred(PanelExchange& Surface, float CentreAlong, float CentreAcross, float EdgeExtent, const InkOrdinate& Tint) const;

private:

    std::vector<std::uint8_t>  RasterOrdinates;   // [-] - RGBA coverage of the glyph, white premultiplied by coverage
    void*                      GlyphSeat = nullptr;   // [-] - the vendor picture identity
};

}   // namespace Rift
