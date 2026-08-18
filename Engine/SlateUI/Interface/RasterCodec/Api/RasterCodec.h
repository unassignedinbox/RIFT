//============================================================================================================================================
//                                                             RASTERCODEC.H
//============================================================================================================================================
// 🧩 Translates the recorded draw data of one tick into display pixels — stream translation, image out — with no window and no GPU.

#pragma once

#include "Engine/Contract/Api/DeliveryContract.h"

#include <cstdint>
#include <vector>

namespace Slate
{

//------------------------------------------------------------------------------------------------------------------------
//                                                     THE PIXEL EXTENT
//------------------------------------------------------------------------------------------------------------------------

/// 🧩 One bounded RGBA byte extent the codec resolves into, eight bits per component.
/// tag   contract, nonallocating, nonthrowing
struct PixelSpace
{
    std::uint32_t           AlongExtent  = 0u;    // [px]
    std::uint32_t           AcrossExtent = 0u;    // [px]
    std::vector<std::uint8_t> Ordinates;          // [-] - RGBA, tightly packed row-major
};

/// 🧩 One seated picture the codec resolves identities against — a depot glyph.
/// tag   contract, nonallocating, nonthrowing
struct PictureDeclaration
{
    void*            Identity     = nullptr;   // [-] - the vendor picture identity
    std::uint32_t    AlongExtent  = 0u;        // [px]
    std::uint32_t    AcrossExtent = 0u;        // [px]
    const std::uint8_t* Ordinates = nullptr;   // [-] - borrowed RGBA
};

//------------------------------------------------------------------------------------------------------------------------
//                                                        THE CODEC
//------------------------------------------------------------------------------------------------------------------------

/// 🧩 The software raster translation: clip-rect-scissored, textured, alpha-blended triangles into a pixel extent.
/// note  The one rendering path the standalone hosts use headlessly; a windowed host would hand the same
///       recorded draw data to a platform renderer instead, unchanged.
/// tag   contract, nonthrowing
class RasterCodec
{
public:

    static constexpr std::uint32_t RawMarkerExtent = 16u;   // [B] - the raw dump marker extent

    /// 🧩 Seats the picture identities the codec resolves. Call before the first tick.
    /// cost  ✔️
    /// tag   api, nonallocating, nonthrowing
    void SeatPicture(const PictureDeclaration& Declared)   { Seated.push_back(Declared); }

    /// 🧩 Resolves the vendored typeface atlas identity against the atlas the context holds.
    /// cost  🚩
    /// tag   api, nonthrowing
    Deliver<bool> SeatAtlas(void* Identity);

    /// 🧩 Translates one tick's recorded draw data into the seated pixel extent.
    /// in    RecordedDrawData  [-]  borrowed; the vendor draw data of the tick
    /// cost  🚩
    /// tag   api, nonthrowing
    void Rasterize(const void* RecordedDrawData, PixelSpace& Extent);

    /// 🧩 Writes the pixel extent as one marked raw dump, for the proof encoder.
    /// cost  🚩
    /// tag   api, nonthrowing
    Deliver<bool> WriteRawDump(const PixelSpace& Extent, const char* Path);

private:

    std::vector<PictureDeclaration>  Seated;          // [-] - identity → picture
    void*                            AtlasSeat = nullptr;   // [-] - the typeface atlas identity
    std::vector<std::uint8_t>        AtlasOrdinates;        // [-] - the atlas RGBA, resolved once
    std::uint32_t                    AtlasAlongExtent = 0u; // [px]
    std::uint32_t                    AtlasAcrossExtent = 0u;   // [px]
};

}   // namespace Slate
