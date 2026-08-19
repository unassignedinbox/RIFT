//============================================================================================================================================
//                                                        TEXTUREPAINTPANEL.H
//============================================================================================================================================
// 🧩 The layer stack, channel property seat and mask property seat, transcribed from TexturePaint.tsx exactly as it stands.

#pragma once

#include "Contract/Api/PanelContract.h"
#include "SlateUI/Interface/IconDepot/Api/IconDepot.h"
#include "SlateUI/Interface/PanelExchange/Api/PanelExchange.h"
#include "SlateUI/Interface/ThemeSpecification/Api/ThemeSpecification.h"

#include <cstdint>

namespace Slate
{
namespace Reference
{

//------------------------------------------------------------------------------------------------------------------------
//                                                    THE CHANNEL SLOTS
//------------------------------------------------------------------------------------------------------------------------

/// 🧩 How one channel's content is authored.
/// tag   contract
enum class ChannelSource : std::uint32_t
{
    Value      = 0u,   // [-] - flat authored
    Texture    = 1u,   // [-] - painted by hand
    Generator  = 2u    // [-] - procedural
};

/// 🧩 What one channel slot edits.
/// tag   contract
enum class ChannelEdit : std::uint32_t
{
    Scalar    = 0u,   // [-] - one amount
    Colour    = 1u,   // [-] - one colour
    Derived   = 2u    // [-] - nothing — derived from another channel
};

/// 🧩 One channel slot the property seat presents, verbatim from CHANNEL_SLOTS.
/// tag   contract, nonallocating, nonthrowing
struct ChannelSlotDeclaration
{
    const char*      Key         = "";        // [-] - borrowed identity
    const char*      Label       = "";        // [-] - borrowed caption
    const char*      Group       = "";        // [-] - borrowed group caption
    std::uint32_t    HuePacked   = 0u;        // [-] - 0xRRGGBB chip tint
    ChannelEdit      Edit        = ChannelEdit::Scalar;
    double           Minimum     = 0.0;       // [-]
    double           Maximum     = 1.0;       // [-]
    const char*      Unit        = "-";       // [-] - the unit segment run
    const char*      PlacementRun = "";       // [-] - borrowed atlas placement
};

/// 🧩 The closed channel slot record, in CHANNEL_ORDER order.
/// cost  ✔️
/// tag   api, nonallocating, nonthrowing
const ChannelSlotDeclaration* ChannelSlotRecord(std::uint32_t& SlotCount);

/// 🧩 One generator the seats pick from, verbatim from GENERATOR_CATALOGUE.
/// tag   contract, nonallocating, nonthrowing
struct GeneratorDeclaration
{
    const char*      Group    = "";     // [-] - borrowed; empty for entries
    const char*      Key      = "";     // [-] - borrowed identity
    const char*      Label    = "";     // [-] - borrowed caption
    const char*      NoteRun  = "";     // [-] - borrowed note
    std::uint32_t    ParameterCount = 0u;   // [-]
    const char*      ParameterLabels[3] = { "", "", "" };   // [-] - borrowed
    double           ParameterDefaults[3] = { 0.0, 0.0, 0.0 };   // [-]
};

/// 🧩 The closed generator record.
/// cost  ✔️
/// tag   api, nonallocating, nonthrowing
const GeneratorDeclaration* GeneratorRecord(std::uint32_t& GeneratorCount);

//------------------------------------------------------------------------------------------------------------------------
//                                                     THE LAYER STACK
//------------------------------------------------------------------------------------------------------------------------

/// 🧩 One layer's mask ordinates.
/// tag   contract, nonallocating, nonthrowing
struct LayerMaskOrdinates
{
    bool           Enabled   = false;    // [-] - stands a mask
    char           Source[24] = "";      // [-] - the source caption run
    double         Strength  = 100.0;    // [%]  - mask strength
    bool           Invert    = false;    // [-]
    bool           Shown     = true;     // [-]
};

/// 🧩 One layer's ordinates, seated at the reference's mockLayers values.
/// tag   contract, nonallocating, nonthrowing
struct LayerOrdinates
{
    char              Name[32]     = "";       // [-]
    std::uint32_t     Content      = 0u;       // [-] - 0 paint, 1 material, 2 generator
    char              Transfer[16] = "Normal"; // [-] - the transfer caption (the reference's `blend`)
    double            Opacity      = 100.0;    // [%]
    bool              Shown        = true;     // [-]
    std::uint32_t     PaintPacked  = 0u;       // [-] - the swatch tint
    std::uint32_t     TagPacked    = 0u;       // [-] - the spine tint
    const char* const* Channels    = nullptr;  // [-] - borrowed channel captions
    std::uint32_t     ChannelCount = 0u;       // [-]
    LayerMaskOrdinates Mask;                   // [-]

    bool              Expanded     = false;    // [-] - folded properties disclosure
    bool              Removed      = false;    // [-] - deleted from the stack
};

/// 🧩 The layer stack — head, toolbar, the spine-and-card list, the count foot.
/// tag   contract, nonallocating, nonthrowing
class LayerStackPanel
{
public:

    /// 🧩 Presents the stack inside the seat extent, one tick.
    /// tag   api, nonallocating, nonthrowing
    void Advance(PanelExchange& Surface, const PlaneExtent& Seat, LayerOrdinates* Layers, std::uint32_t LayerCount,
                 const IconDepot& Depot);

    char           RetentionRun[48] = "";   // [-] - the live filter run
    std::uint32_t  ActiveLayer      = 0u;   // [-] - taken layer ordinal
    bool           ActiveTargetMask = false;   // [-] - taken target: layer or its mask
    bool           InspectRaised    = false;   // [-] - the inspect gesture edge

private:

    float          ScrollAcross  = 0.0f;    // [px] - list scroll ordinate
    std::int32_t   DragOrdinal   = -1;      // [-]  - the layer under a live reorder drag
    float          DragBoundaryAcross = -1.0e9f;   // [px] - the drop boundary's across ordinate

    /// 🧩 Whether a reorder drag stands held this tick.
    /// cost  ✔️
    /// tag   internal, nonallocating, nonthrowing
    bool DragStanding() const   { return DragOrdinal >= 0; }
};


//------------------------------------------------------------------------------------------------------------------------
//                                                 THE CHANNEL PROPERTY SEAT
//------------------------------------------------------------------------------------------------------------------------

/// 🧩 Every channel ordinate the property seat writes through, seated at the reference's defaults.
/// tag   contract, nonallocating, nonthrowing
struct ChannelOrdinates
{
    char          LayerName[32] = "Brushed Copper";   // [-]
    std::uint32_t Classification = 1u;                // [-] - the head's caption pair (0 paint, 1 material, 2 generator)

    bool          Enabled[14];        // [-] - channel presence
    bool          Collapsed[14];      // [-] - card disclosure
    std::uint32_t Source[14];         // [-] - ChannelSource ordinal
    double        Amount[14];         // [-] - the scalar amount
    std::uint32_t Colour[14];         // [-] - 0xRRGGBB colour
    std::uint32_t Strokes[14];        // [-] - painted stroke count
    std::uint32_t Generator[14];      // [-] - generator ordinal into the record
    double        GeneratorParameters[14][3];   // [-]
    bool          TextureSeated[14];  // [-] - an imported base stands
};

/// 🧩 Seats every channel ordinate at the reference's declared defaults.
/// cost  ✔️
/// tag   api, nonallocating, nonthrowing
void SeatChannelOrdinates(ChannelOrdinates& Ordinates);

/// 🧩 The channel property seat — head, chips region, channel cards, foot.
/// tag   contract, nonallocating, nonthrowing
class ChannelPropertyPanel
{
public:

    /// 🧩 Presents the seat inside the extent, one tick.
    /// tag   api, nonallocating, nonthrowing
    void Advance(PanelExchange& Surface, const PlaneExtent& Seat, ChannelOrdinates& Ordinates, const IconDepot& Depot);

private:

    float ScrollAcross = 0.0f;   // [px] - body scroll ordinate
};

//------------------------------------------------------------------------------------------------------------------------
//                                                  THE MASK PROPERTY SEAT
//------------------------------------------------------------------------------------------------------------------------

/// 🧩 Every mask ordinate the property seat writes through, seated at the reference's defaults.
/// tag   contract, nonallocating, nonthrowing
struct MaskOrdinates
{
    char          LayerName[32] = "Edge Wear";   // [-]
    bool          Present  = true;    // [-] - the mask stands
    bool          Collapsed = false;  // [-] - section disclosure
    std::uint32_t BaseFill = 0u;      // [-] - 0 white, 1 black
    bool          Invert   = false;   // [-]
    double        Strength = 0.92;    // [-] - 0..1
    std::uint32_t SourceMode = 1u;    // [-] - 0 texture, 1 generator
    std::uint32_t Strokes  = 0u;      // [-]
    bool          TextureSeated = false;   // [-] - an imported base stands
    std::uint32_t Generator = 2u;    // [-] - generator ordinal (Metal Edge Wear)
    double        GeneratorParameters[3] = { 0.6, 0.3, 0.45 };   // [-]
};

/// 🧩 The mask property seat — head, the one foldable section, source slots, generator parameters.
/// tag   contract, nonallocating, nonthrowing
class MaskPropertyPanel
{
public:

    /// 🧩 Presents the seat inside the extent, one tick.
    /// tag   api, nonallocating, nonthrowing
    void Advance(PanelExchange& Surface, const PlaneExtent& Seat, MaskOrdinates& Ordinates, const IconDepot& Depot);
};

}   // namespace Reference
}   // namespace Slate
