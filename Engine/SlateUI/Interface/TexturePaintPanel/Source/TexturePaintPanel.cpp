//============================================================================================================================================
//                                                        TEXTUREPAINTPANEL.CPP
//============================================================================================================================================
// 🧩 Layer stack, channel cards and mask section — the texture-paint reference transcribed onto the recording seam.

#include "Engine/SlateUI/Interface/TexturePaintPanel/Api/TexturePaintPanel.h"
#include "Engine/SlateUI/Interface/ControlPanel/Api/ControlPanel.h"

#include "imgui.h"

#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>

namespace Slate
{

//------------------------------------------------------------------------------------------------------------------------
//                                                  THE CLOSED SLOT RECORDS
//------------------------------------------------------------------------------------------------------------------------

namespace
{

const ChannelSlotDeclaration Slots[14] =
{
    { "baseColour",       "Base Colour",       "Surface",     0xB87333u, ChannelEdit::Colour,  0.0,   1.0,   "-", "Colour atlas \xC2\xB7 RGB"     },
    { "metallic",         "Metallic",          "Surface",     0x8B5CF6u, ChannelEdit::Scalar,  0.0,   1.0,   "-", "Material atlas \xC2\xB7 R"     },
    { "roughness",        "Roughness",         "Surface",     0x3B82F6u, ChannelEdit::Scalar,  0.0,   1.0,   "-", "Material atlas \xC2\xB7 G"     },
    { "height",           "Height",            "Surface",     0x8A8A8Au, ChannelEdit::Scalar,  0.0,   1.0,   "-", "Material atlas \xC2\xB7 B"     },
    { "normal",           "Normal",            "Surface",     0x10B981u, ChannelEdit::Derived, 0.0,   1.0,   "-", "No storage \xC2\xB7 derived"   },
    { "opacity",          "Opacity",           "Surface",     0x94A3B8u, ChannelEdit::Scalar,  0.0,   1.0,   "-", "Material atlas \xC2\xB7 A"     },
    { "emission",         "Emissive",          "Radiance",    0xF59E0Bu, ChannelEdit::Colour,  0.0,   1.0,   "-", "Emissive atlas \xC2\xB7 RGB"   },
    { "ambientOcclusion", "Ambient Occlusion", "Radiance",    0x6B7280u, ChannelEdit::Scalar,  0.0,   1.0,   "-", "Emissive atlas \xC2\xB7 A"     },
    { "anisotropy",       "Anisotropy",        "Reflectance", 0x22D3EEu, ChannelEdit::Scalar,  0.0,   1.0,   "-", "Reflect atlas \xC2\xB7 R"      },
    { "anisotropyAngle",  "Anisotropy Angle",  "Reflectance", 0x0EA5E9u, ChannelEdit::Scalar,  0.0, 360.0, "\xC2\xB0", "Reflect atlas \xC2\xB7 G"   },
    { "clearcoat",        "Clearcoat",         "Reflectance", 0xE2E8F0u, ChannelEdit::Scalar,  0.0,   1.0,   "-", "Reflect atlas \xC2\xB7 B"      },
    { "refractionIndex",  "Refraction Index",  "Reflectance", 0xA78BFAu, ChannelEdit::Scalar,  1.0,   3.0,   "-", "Reflect atlas \xC2\xB7 A"      },
    { "sheen",            "Sheen",             "Scattering",  0xF472B6u, ChannelEdit::Colour,  0.0,   1.0,   "-", "Scatter atlas \xC2\xB7 RGB"    },
    { "subsurface",       "Subsurface",        "Scattering",  0xFB7185u, ChannelEdit::Colour,  0.0,   1.0,   "-", "Sheen atlas \xC2\xB7 RGB"      }
};

const GeneratorDeclaration Generators[13] =
{
    { "Mask", "", "", "", 0u, { "", "", "" }, { 0.0, 0.0, 0.0 } },
    { "", "CurvatureEdges",   "Curvature",         "Convex edge wear",   3u, { "Balance", "Contrast", "Radius" },      { 0.5, 0.7, 0.25 } },
    { "", "AmbientOcclusion", "Ambient Occlusion", "Cavity dirt",        2u, { "Spread", "Contrast", "" },              { 0.4, 0.6, 0.0 } },
    { "", "Thickness",        "Thickness",         "Translucent falloff",2u, { "Depth", "Contrast", "" },               { 0.5, 0.5, 0.0 } },
    { "", "PositionGradient", "Position Gradient", "World-axis ramp",    2u, { "Origin", "Falloff", "" },               { 0.5, 0.35, 0.0 } },
    { "Wear", "", "", "", 0u, { "", "", "" }, { 0.0, 0.0, 0.0 } },
    { "", "MetalEdgeWear",    "Metal Edge Wear",   "Curvature + grunge", 3u, { "Intensity", "Softness", "Grain" },      { 0.6, 0.3, 0.45 } },
    { "", "DirtAccumulation", "Dirt",              "Occlusion-driven",   2u, { "Amount", "Scale", "" },                { 0.5, 0.3, 0.0 } },
    { "", "WaterRunoff",      "Water Runoff",      "Gravity streaks",    3u, { "Length", "Density", "Gravity" },       { 0.55, 0.4, 0.8 } },
    { "Procedural", "", "", "", 0u, { "", "", "" }, { 0.0, 0.0, 0.0 } },
    { "", "PerlinNoise",      "Perlin Noise",      "Fractal value noise",3u, { "Scale", "Octaves", "Contrast" },       { 0.4, 0.5, 0.5 } },
    { "", "VoronoiCells",     "Voronoi",           "Cellular partition", 2u, { "Density", "Jitter", "" },              { 0.35, 0.7, 0.0 } },
    { "", "BrushedAnisotropy","Brushed Metal",     "Anisotropic streaks",2u, { "Angle", "Grain", "" },                 { 0.0, 0.6, 0.0 } }
};

const char* const TransferCaptions[7] = { "Normal", "Multiply", "Screen", "Overlay", "Add", "Darken", "Linear Dodge" };

/// 🧩 The tint a layer content carries (KINDS).
/// cost  ✔️
std::uint32_t ContentTint(std::uint32_t Content)
{
    if (Content == 0u) return 0xF97316u;   // paint
    if (Content == 1u) return 0x8B5CF6u;   // material
    return 0x10B981u;                      // generator
}

/// 🧩 The caption a layer content carries (KINDS.Label).
/// cost  ✔️
const char* ContentRun(std::uint32_t Content)
{
    if (Content == 0u) return "Paint";
    if (Content == 1u) return "Material";
    return "Generator";
}

/// 🧩 One interaction seat; reports the pressed edge.
/// cost  ✔️
bool PresentSeat(const PlaneExtent& Seat, const char* PushIdentity, bool& Roused)
{
    ImGui::PushID(PushIdentity);
    ImGui::SetCursorScreenPos(ImVec2(Seat.LeastAlong, Seat.LeastAcross));
    ImGui::InvisibleButton("seat", ImVec2(Seat.SpanAlong(), Seat.SpanAcross()));
    Roused = ImGui::IsItemHovered();
    const bool Clicked = ImGui::IsItemClicked();
    ImGui::PopID();
    return Clicked;
}

/// 🧩 The channel sheet's slider — capsule of 92 px, centred numeral, 30 px unit segment, 19 px track.
/// tag   internal
void PresentChannelSlider(RecordingSurface& Surface, const PlaneExtent& Row, double& Amount, double Minimum, double Maximum,
                          std::uint32_t Figures, const char* Unit, const char* PushIdentity)
{
    ChannelInk Sheet;
    const double Span = Maximum - Minimum;
    double Fraction = Span > 0.0 ? (Amount - Minimum) / Span : 0.0;
    if (Fraction < 0.0) Fraction = 0.0;
    if (Fraction > 1.0) Fraction = 1.0;

    const PlaneExtent Capsule = Spanning(Row.LeastAlong, CentredAcross(Row, 26.0f), 92.0f, 26.0f);
    const PlaneExtent Track   = Spanning(Capsule.MostAlong + 9.0f, CentredAcross(Row, 19.0f), Row.MostAlong - Capsule.MostAlong - 9.0f, 19.0f);

    bool Roused = false;
    if (PresentSeat(Track, PushIdentity, Roused))
    {
        // ① The press lands — jump the knob beneath it.
        ImGuiIO& VendorIO = ImGui::GetIO();
        const float Local = (VendorIO.MousePos.x - Track.LeastAlong) / Track.SpanAlong();
        const float Confined = Local < 0.0f ? 0.0f : (Local > 1.0f ? 1.0f : Local);
        Amount = Minimum + Confined * Span;
        Fraction = Confined;
    }
    if (ImGui::IsItemActive() && Roused)
    {
        ImGuiIO& VendorIO = ImGui::GetIO();
        const float Local = (VendorIO.MousePos.x - Track.LeastAlong) / Track.SpanAlong();
        const float Confined = Local < 0.0f ? 0.0f : (Local > 1.0f ? 1.0f : Local);
        Amount = Minimum + Confined * Span;
        Fraction = Confined;
    }

    // ② Capsule — black ground, centred numeral, unit segment.
    Surface.Ground(Capsule, Sheet.FieldSunken, 13.0f);
    const PlaneExtent UnitSeat = Spanning(Capsule.MostAlong - 30.0f, Capsule.LeastAcross, 30.0f, Capsule.SpanAcross());
    Surface.Ground(UnitSeat, Sheet.FieldUnit, 13.0f, CornerSelection::UpperTrailing);
    Surface.Ground(UnitSeat, Sheet.FieldUnit, 13.0f, CornerSelection::LowerTrailing);
    Surface.Ground(Spanning(UnitSeat.LeastAlong, UnitSeat.LeastAcross, 15.0f, UnitSeat.SpanAcross()), Sheet.FieldUnit, 0.0f);

    char Numeral[32];
    if (Figures == 0u) std::snprintf(Numeral, sizeof Numeral, "%.0f", Amount);
    else if (Figures == 1u) std::snprintf(Numeral, sizeof Numeral, "%.1f", Amount);
    else std::snprintf(Numeral, sizeof Numeral, "%.2f", Amount);
    const float NumeralSize = 12.5f;
    const float NumeralExtent = Surface.MeasureRun(Numeral, NumeralSize);
    Surface.TextRun(Capsule.LeastAlong + (Capsule.SpanAlong() - 30.0f - NumeralExtent) * 0.5f,
                    CentredAcross(Capsule, Surface.RunExtent(NumeralSize)), Numeral, Sheet.KnobInk, NumeralSize);
    const float UnitSize = 10.5f;
    const float UnitExtent = Surface.MeasureRun(Unit, UnitSize);
    Surface.TextRun(UnitSeat.LeastAlong + (UnitSeat.SpanAlong() - UnitExtent) * 0.5f,
                    CentredAcross(UnitSeat, Surface.RunExtent(UnitSize)), Unit, Sheet.InkMuted, UnitSize);

    // ③ Track, fill, knob.
    Surface.Ground(Track, Sheet.TrackGround, Track.SpanAcross() * 0.5f);
    Surface.Ground(Spanning(Track.LeastAlong, Track.LeastAcross, Track.SpanAlong() * static_cast<float>(Fraction), Track.SpanAcross()),
                   Sheet.TrackFill, Track.SpanAcross() * 0.5f);
    const float KnobAlong = Track.LeastAlong + Track.SpanAlong() * static_cast<float>(Fraction);
    const float KnobAcross = Track.LeastAcross + Track.SpanAcross() * 0.5f;
    Surface.Medallion(KnobAlong, KnobAcross, 13.5f, InkOrdinate{ 0u, 0u, 0u, 89u });
    Surface.Medallion(KnobAlong, KnobAcross, 10.5f, Sheet.KnobInk);
}

/// 🧩 The channel sheet's segment row — a 999-radius pill, the taken segment inverted.
/// tag   internal
void PresentChannelSegments(RecordingSurface& Surface, const PlaneExtent& Seat, const char* const* Captions, std::uint32_t Count,
                            std::uint32_t& Taken, const char* PushIdentity)
{
    ChannelInk Sheet;
    float Segments[4];
    const float GapExtent = 3.0f;
    const float PillPad = 14.0f;
    float SpanRemaining = Seat.SpanAlong();
    const float Gaps = static_cast<float>(Count) * GapExtent;
    for (std::uint32_t Ordinal = 0u; Ordinal < Count; ++Ordinal)
        SpanRemaining -= PillPad * 2.0f;
    for (std::uint32_t Ordinal = 0u; Ordinal < Count; ++Ordinal)
    {
        const float Share = (SpanRemaining - Gaps) / Count;
        Segments[Ordinal] = Share + PillPad * 2.0f;
    }

    float Leading = Seat.LeastAlong;
    for (std::uint32_t Ordinal = 0u; Ordinal < Count; ++Ordinal)
    {
        const PlaneExtent Segment = Spanning(Leading, CentredAcross(Seat, 26.0f), Segments[Ordinal], 26.0f);
        char Identity[48];
        std::snprintf(Identity, sizeof Identity, "%s.%u", PushIdentity, Ordinal);
        bool Roused = false;
        if (PresentSeat(Segment, Identity, Roused))
            Taken = Ordinal;

        if (Taken == Ordinal)
            Surface.Ground(Segment, Sheet.Accent, 13.0f, CornerSelection::UpperLeading);
        else
            Surface.Ground(Segment, Sheet.TileGround, 13.0f, CornerSelection::UpperLeading);
        if (Taken == Ordinal)
            Surface.Ground(Segment, Sheet.Accent, 13.0f, CornerSelection::LowerLeading);
        else
            Surface.Ground(Segment, Sheet.TileGround, 13.0f, CornerSelection::LowerLeading);
        const float Size = 10.5f;
        Surface.TextRun(Surface.CentredAlong(Segment, Captions[Ordinal], Size),
                        CentredAcross(Segment, Surface.RunExtent(Size)), Captions[Ordinal],
                        Taken == Ordinal ? Sheet.OnAccent : Sheet.InkMuted, Size);
        Leading += Segments[Ordinal] + GapExtent;
    }
    // ① The pill's hair edge, drawn once around the whole row.
    Surface.Edge(Seat.Inset(-1.0f, 0.0f), Sheet.HairEdge, 1.0f, 13.0f);
}

/// 🧩 The pickbar — a pill head opening a styled option list, for generators and transfer captions.
/// tag   internal
void PresentPickbar(RecordingSurface& Surface, const PlaneExtent& Seat, const char* CurrentRun, const char* PushIdentity)
{
    ChannelInk Sheet;
    bool Roused = false;
    const bool Clicked = PresentSeat(Seat, PushIdentity, Roused);

    Surface.Ground(Seat, Sheet.TileGround, 13.0f);
    Surface.Edge(Seat, Roused ? Sheet.HairEdgeStrong : Sheet.HairEdge, 1.0f, 13.0f);
    Surface.TextRunClipped(Seat.LeastAlong + 10.0f, CentredAcross(Seat, Surface.RunExtent(11.0f)), CurrentRun, Sheet.InkPrimary, 11.0f,
                           Seat.SpanAlong() - 30.0f);
    Surface.Chevron(Seat.MostAlong - 14.0f, Seat.LeastAcross + Seat.SpanAcross() * 0.5f - 1.0f, 3.0f, Sheet.InkFaint, true);

    ImGui::PushID(PushIdentity);
    if (Clicked)
        ImGui::OpenPopup("pick");
    ImGui::PopID();
}

/// 🧩 The 8 px checkerboard a texture thumb seats.
/// tag   internal
void PresentCheckerTile(RecordingSurface& Surface, const PlaneExtent& Seat)
{
    ChannelInk Sheet;
    const InkOrdinate Dark = Covering(0x0A0A0Au);
    const InkOrdinate Light = Covering(0x1A1A1Au);
    Surface.Ground(Seat, Dark, 6.0f);
    for (float Across = 0.0f; Across < Seat.SpanAcross(); Across += 8.0f)
        for (float Along = 0.0f; Along < Seat.SpanAlong(); Along += 8.0f)
        {
            const std::uint32_t Step = static_cast<std::uint32_t>(Across / 8.0f + Along / 8.0f);
            if (Step % 2u == 0u)
                Surface.Ground(Spanning(Seat.LeastAlong + Along, Seat.LeastAcross + Across,
                                        Along + 8.0f < Seat.SpanAlong() ? 8.0f : Seat.SpanAlong() - Along,
                                        Across + 8.0f < Seat.SpanAcross() ? 8.0f : Seat.SpanAcross() - Across), Light, 0.0f);
        }
    Surface.Edge(Seat, Sheet.HairEdge, 1.0f, 6.0f);
}

}   // namespace

//------------------------------------------------------------------------------------------------------------------------
//                                                     THE SLOT RECORDS
//------------------------------------------------------------------------------------------------------------------------

const ChannelSlotDeclaration* ChannelSlotRecord(std::uint32_t& SlotCount)
{
    SlotCount = 14u;
    return Slots;
}

const GeneratorDeclaration* GeneratorRecord(std::uint32_t& GeneratorCount)
{
    GeneratorCount = 13u;
    return Generators;
}

void SeatChannelOrdinates(ChannelOrdinates& Ordinates)
{
    for (std::uint32_t Ordinal = 0u; Ordinal < 14u; ++Ordinal)
    {
        Ordinates.Enabled[Ordinal]   = false;
        Ordinates.Collapsed[Ordinal] = true;
        Ordinates.Source[Ordinal]    = 0u;
        Ordinates.Amount[Ordinal]    = 0.0;
        Ordinates.Colour[Ordinal]    = 0u;
        Ordinates.Strokes[Ordinal]   = 0u;
        Ordinates.Generator[Ordinal] = 0u;
        Ordinates.TextureSeated[Ordinal] = false;
        Ordinates.GeneratorParameters[Ordinal][0] = 0.0;
        Ordinates.GeneratorParameters[Ordinal][1] = 0.0;
        Ordinates.GeneratorParameters[Ordinal][2] = 0.0;
    }

    // ① The reference's seated layer: nine enabled channels, three open cards.
    Ordinates.Enabled[0] = true;    Ordinates.Collapsed[0] = false;    // baseColour
    Ordinates.Source[0]  = 1u;      Ordinates.Colour[0] = 0xB87333u;
    Ordinates.TextureSeated[0] = true;
    Ordinates.Strokes[0] = 148u;

    Ordinates.Enabled[1] = true;    Ordinates.Collapsed[1] = false;    // metallic
    Ordinates.Source[1]  = 0u;      Ordinates.Amount[1] = 0.94;

    Ordinates.Enabled[2] = true;    Ordinates.Collapsed[2] = false;    // roughness
    Ordinates.Source[2]  = 2u;      Ordinates.Generator[2] = 6u;
    Ordinates.GeneratorParameters[2][0] = 0.6;
    Ordinates.GeneratorParameters[2][1] = 0.3;
    Ordinates.GeneratorParameters[2][2] = 0.45;

    Ordinates.Enabled[3] = true;    Ordinates.Amount[3] = 0.50;        // height, collapsed
    Ordinates.Strokes[3] = 62u;     Ordinates.Source[3] = 0u;

    Ordinates.Enabled[4] = true;                                         // normal, derived, collapsed
    Ordinates.Enabled[7] = true;    Ordinates.Source[7] = 2u;            // ambient occlusion, collapsed
    Ordinates.Generator[7] = 2u;
    Ordinates.GeneratorParameters[7][0] = 0.4;
    Ordinates.GeneratorParameters[7][1] = 0.6;

    Ordinates.Enabled[8] = true;                                         // anisotropy, collapsed
    Ordinates.Amount[8] = 0.72;
    Ordinates.Enabled[9] = true;    Ordinates.Amount[9] = 35.0;         // anisotropy angle, collapsed
    Ordinates.Enabled[10] = true;   Ordinates.Amount[10] = 0.15;        // clearcoat, collapsed
}

//------------------------------------------------------------------------------------------------------------------------
//                                                      THE LAYER STACK
//------------------------------------------------------------------------------------------------------------------------

void LayerStackPanel::Advance(RecordingSurface& Surface, const PlaneExtent& Seat, LayerOrdinates* Layers, std::uint32_t LayerCount,
                              const IconDepot& Depot)
{
    ChannelInk Sheet;
    InspectRaised = false;

    Surface.Ground(Seat, Sheet.StackGround, 0.0f);

    // ① Head — black tile, dummy glyph, caption pair, count pill.
    const PlaneExtent Head = Spanning(Seat.LeastAlong, Seat.LeastAcross, Seat.SpanAlong(), 46.0f);
    Surface.Ground(Head, Sheet.StackHead, 0.0f);
    Surface.Rule(Head.LeastAlong, Head.MostAcross - 1.0f, Head.SpanAlong(), 1.0f, Sheet.StackEdge);
    const PlaneExtent Tile = Spanning(Head.LeastAlong + 10.0f, CentredAcross(Head, 24.0f), 24.0f, 24.0f);
    Surface.Ground(Tile, Covering(0x000000u), 6.0f);
    Surface.Edge(Tile, Sheet.StackEdge, 1.0f, 6.0f);
    Depot.PresentGlyph(Surface, Tile.Inset(5.0f, 5.0f), Sheet.InkMuted);
    Surface.TextRun(Tile.MostAlong + 8.0f, Head.LeastAcross + 8.0f, "Layer Stack", Sheet.InkPrimary, 12.5f);
    Surface.TextRun(Tile.MostAlong + 8.0f, Head.LeastAcross + 25.0f, "Suzanne \xC2\xB7 one material + paint", Sheet.InkMuted, 10.0f);

    char CountRun[12];
    std::snprintf(CountRun, sizeof CountRun, "%u", LayerCount);
    const float CountExtent = Surface.MeasureRun(CountRun, 10.0f);
    const PlaneExtent CountPill = Spanning(Head.MostAlong - 12.0f - CountExtent - 12.0f, CentredAcross(Head, 20.0f), CountExtent + 12.0f, 20.0f);
    Surface.Ground(CountPill, Covering(0x1B1B1Bu), 10.0f);
    Surface.Edge(CountPill, Covering(0x2A2A2Au), 1.0f, 10.0f);
    Surface.TextRun(Surface.CentredAlong(CountPill, CountRun, 10.0f), CentredAcross(CountPill, Surface.RunExtent(10.0f)), CountRun, Sheet.InkMuted, 10.0f);

    // ② Toolbar — add action and the filter field.
    const PlaneExtent Toolbar = Spanning(Seat.LeastAlong + 7.0f, Head.MostAcross + 7.0f, Seat.SpanAlong() - 14.0f, 28.0f);
    bool AddRoused = false;
    if (PresentSeat(Toolbar, "stack.add", AddRoused))
    {
        // ①① A new layer seats at the head with the next colour of the reference's cycle.
        static const std::uint32_t Cycle[7] = { 0x4A90E2u, 0xF59E0Bu, 0x10B981u, 0x8B5CF6u, 0xEC4899u, 0x06B6D4u, 0xEAB308u };
        const std::uint32_t Tint = Cycle[LayerCount % 7u];
        LayerOrdinates Fresh;
        std::snprintf(Fresh.Name, sizeof Fresh.Name, "New Layer");
        Fresh.Content = 0u;
        std::snprintf(Fresh.Transfer, sizeof Fresh.Transfer, "Normal");
        Fresh.Opacity = 100.0;  Fresh.Shown = true;  Fresh.PaintPacked = Tint;  Fresh.TagPacked = Tint;
        static const char* const FreshChannels[1] = { "Base Colour" };
        Fresh.Channels = FreshChannels;  Fresh.ChannelCount = 1u;
        for (std::uint32_t Ordinal = LayerCount; Ordinal > 0u; --Ordinal)
            Layers[Ordinal] = Layers[Ordinal - 1u];
        Layers[0] = Fresh;
    }
    Surface.Ground(Toolbar, Covering(0x141414u), 7.0f);
    Surface.Edge(Toolbar, AddRoused ? Covering(0x3A3A3Au) : Covering(0x242424u), 1.0f, 7.0f);
    Surface.PlusGlyph(Toolbar.LeastAlong + Surface.CentredAlong(Toolbar, "Add layer", 11.0f) - 14.0f,
                      Toolbar.LeastAcross + 14.0f, 4.0f, AddRoused ? Sheet.InkPrimary : Sheet.InkMuted);
    Surface.TextRun(Toolbar.LeastAlong + Surface.CentredAlong(Toolbar, "Add layer", 11.0f), CentredAcross(Toolbar, Surface.RunExtent(11.0f)),
                    "Add layer", AddRoused ? Sheet.InkPrimary : Sheet.InkMuted, 11.0f);

    const PlaneExtent FieldSeat = Spanning(Seat.LeastAlong + 7.0f, Toolbar.MostAcross + 5.0f, Seat.SpanAlong() - 14.0f, 26.0f);
    PresentRetentionField(Surface, FieldSeat, RetentionRun, 48u, "Filter layers...", Covering(0x121214u), Covering(0x242424u),
                          Sheet.InkPrimary, Covering(0x4A4A4Au));

    // ③ The list — spine, ordinal medallion, split card.
    const PlaneExtent List = Spanning(Seat.LeastAlong + 3.0f, FieldSeat.MostAcross + 2.0f, Seat.SpanAlong() - 6.0f,
                                      Seat.MostAcross - 26.0f - (FieldSeat.MostAcross + 2.0f));
    if (Surface.PointerWithin(List))
        ScrollAcross -= ImGui::GetIO().MouseWheel * 32.0f;

    std::uint32_t StandingCount = 0u;
    for (std::uint32_t Ordinal = 0u; Ordinal < LayerCount; ++Ordinal)
        if (!Layers[Ordinal].Removed)
            ++StandingCount;

    // ①① Card extents are fixed until a card expands, so the cursor runs linearly.
    float CursorAcross = List.LeastAcross - ScrollAcross;
    std::uint32_t OrdinalSeen = 0u;

    // ②② A held drag ends when the primary button rises; the splice lands at the tracked boundary.
    const bool DragReleased = DragOrdinal >= 0 && !ImGui::GetIO().MouseDown[0];
    std::uint32_t PresentedOrdinals[16];
    std::uint32_t PresentedCountLocal = 0u;   // [-]  - presented layer ordinals, in list order
    std::uint32_t InsertionPresented  = 0u;   // [-]  - insertion seat among the presented ordinals
    float InsertionBoundary = 1.0e9f;         // [px] - the rail's across ordinate
    const float PointerAcross = Surface.Pointer().Across;
    bool BoundaryTaken = false;
    for (std::uint32_t Ordinal = 0u; Ordinal < LayerCount; ++Ordinal)
    {
        LayerOrdinates& Layer = Layers[Ordinal];
        if (Layer.Removed)
            continue;

        // 📝 Retention matches the caption, case-insensitively, as the reference filters.
        char LoweredName[48];
        char LoweredRun[48];
        {
            std::uint32_t Index = 0u;
            while (Layer.Name[Index] != '\0' && Index < 47u) { LoweredName[Index] = static_cast<char>(std::tolower(static_cast<unsigned char>(Layer.Name[Index]))); ++Index; }
            LoweredName[Index] = '\0';
            Index = 0u;
            while (RetentionRun[Index] != '\0' && Index < 47u) { LoweredRun[Index] = static_cast<char>(std::tolower(static_cast<unsigned char>(RetentionRun[Index]))); ++Index; }
            LoweredRun[Index] = '\0';
        }
        const bool Retained = RetentionRun[0] == '\0' || std::strstr(LoweredName, LoweredRun) != nullptr;
        if (!Retained)
            continue;

        // ①① Every seat identity carries the layer ordinal — cards never collide in the id space.
        char CardIdentity[32];
        std::snprintf(CardIdentity, sizeof CardIdentity, "layer%u", Ordinal);
        char SeatMould[48];

        const bool TakenLayer = ActiveLayer == Ordinal && !ActiveTargetMask;
        const bool TakenMask  = ActiveLayer == Ordinal && ActiveTargetMask;
        const bool Expanded   = Layer.Expanded;
        const float CardExtent = 44.0f + (Expanded ? 118.0f : 0.0f);
        const PlaneExtent Spine = Spanning(Seat.LeastAlong + 5.0f, CursorAcross, 30.0f, CardExtent + 5.0f);
        const PlaneExtent Card  = Spanning(Spine.MostAlong + 4.0f, CursorAcross, Seat.SpanAlong() - Spine.SpanAlong() - 4.0f - 13.0f, CardExtent);

        if (PresentedCountLocal < 16u)
            PresentedOrdinals[PresentedCountLocal] = Ordinal;
        ++PresentedCountLocal;

        // ②② The insertion boundary — the first card whose centre lies below the pointer.
        if (DragOrdinal >= 0 && Ordinal != static_cast<std::uint32_t>(DragOrdinal) && !BoundaryTaken)
        {
            const float CardCentre = Card.LeastAcross + CardExtent * 0.5f;
            if (PointerAcross < CardCentre)
            {
                InsertionBoundary = Card.LeastAcross;
                InsertionPresented = PresentedCountLocal - 1u;
                BoundaryTaken = true;
            }
        }

        if (Card.LeastAcross >= List.MostAcross || Card.MostAcross <= List.LeastAcross)
        {
            CursorAcross += CardExtent + 5.0f;
            ++OrdinalSeen;
            continue;
        }



        // ② Spine — the 3 px bar and the ordinal medallion.
        const InkOrdinate SpineInk = Layer.Shown ? Covering(Layer.TagPacked) : Sheet.StackSpineVacant;
        Surface.Ground(Spanning(Spine.LeastAlong + 13.5f, Spine.LeastAcross + 22.0f, 3.0f, CardExtent - 20.0f), SpineInk, 2.0f);
        const InkOrdinate MedallionInk = Layer.Shown ? Covering(Layer.TagPacked) : Sheet.StackMedallion;
        const PlaneExtent MedallionSeat = Spanning(Spine.LeastAlong + 5.0f, Spine.LeastAcross + 22.0f - 10.0f, 20.0f, 20.0f);
        Surface.Medallion(MedallionSeat.LeastAlong + 10.0f, MedallionSeat.LeastAcross + 10.0f, 13.0f, Sheet.StackGround);
        Surface.Medallion(MedallionSeat.LeastAlong + 10.0f, MedallionSeat.LeastAcross + 10.0f, 10.0f, MedallionInk);
        char OrdinalRun[8];
        std::snprintf(OrdinalRun, sizeof OrdinalRun, "%02u", StandingCount - OrdinalSeen);
        Surface.TextRun(MedallionSeat.LeastAlong + 10.0f - Surface.MeasureRun(OrdinalRun, 10.0f) * 0.5f,
                        MedallionSeat.LeastAcross + 6.0f, OrdinalRun, InkOrdinate{ 255u, 255u, 255u, 255u }, 10.0f);

        // ③ Card ground — taken carries the marker accent and its soft ground.
        const bool TakenCard = ActiveLayer == Ordinal;
        const InkOrdinate CardGround = TakenCard ? Partial(0xFFFFFFu, 0.12) : Sheet.TileGround;
        Surface.Ground(Card, CardGround, 8.0f);
        Surface.Edge(Card, TakenCard ? Sheet.Marker : Covering(0x2A2A2Eu), 1.0f, 8.0f);

        const PlaneExtent TopRow = Spanning(Card.LeastAlong, Card.LeastAcross, Card.SpanAlong(), 44.0f);
        const float HalfExtent = (TopRow.SpanAlong() - 1.0f) * 0.5f;
        const PlaneExtent LayerZone = Spanning(TopRow.LeastAlong + 4.0f, TopRow.LeastAcross, HalfExtent - 4.0f, 44.0f);
        const PlaneExtent DividerColumn = Spanning(TopRow.LeastAlong + HalfExtent, TopRow.LeastAcross + 6.0f, 1.0f, 32.0f);
        const PlaneExtent MaskZone = Spanning(TopRow.LeastAlong + HalfExtent + 5.0f, TopRow.LeastAcross, TopRow.MostAlong - (TopRow.LeastAlong + HalfExtent + 5.0f) - 2.0f, 44.0f);

        // ④ Layer zone.
        if (TakenLayer)
            Surface.Ground(LayerZone, Partial(0xFFFFFFu, 0.06), 7.0f, CornerSelection::UpperLeading);
        float ZoneCursor = LayerZone.LeastAlong;
        const PlaneExtent TwistSeat = Spanning(ZoneCursor, Card.LeastAcross + 12.0f, 20.0f, 20.0f);
        bool TwistRoused = false;
        if (std::snprintf(SeatMould, sizeof SeatMould, "%s.twist", CardIdentity),
            PresentSeat(TwistSeat, SeatMould, TwistRoused))
            Layer.Expanded = !Layer.Expanded;
        Surface.Chevron(TwistSeat.LeastAlong + 10.0f, TwistSeat.LeastAcross + 10.0f, 4.0f, TwistRoused ? Sheet.InkPrimary : Sheet.InkMuted, Expanded);
        ZoneCursor += 22.0f;

        const PlaneExtent EyeSeat = Spanning(ZoneCursor, Card.LeastAcross + 12.0f, 20.0f, 20.0f);
        bool EyeRoused = false;
        if (std::snprintf(SeatMould, sizeof SeatMould, "%s.eye", CardIdentity),
            PresentSeat(EyeSeat, SeatMould, EyeRoused))
            Layer.Shown = !Layer.Shown;
        Surface.EyeGlyph(EyeSeat.LeastAlong + 10.0f, EyeSeat.LeastAcross + 10.0f, 7.0f, EyeRoused ? Sheet.InkPrimary : Sheet.InkMuted, !Layer.Shown);
        ZoneCursor += 22.0f;

        const PlaneExtent Swatch = Spanning(ZoneCursor, CentredAcross(TopRow, 26.0f), 26.0f, 26.0f);
        Surface.Ground(Swatch, Covering(0x000000u), 6.0f);
        Surface.Edge(Swatch, Sheet.StackEdge, 1.0f, 6.0f);
        Surface.Ground(Swatch.Inset(6.0f, 6.0f), Covering(Layer.PaintPacked), 3.0f);
        Surface.Medallion(Swatch.MostAlong - 4.0f, Swatch.LeastAcross + 4.0f, 2.0f, Covering(ContentTint(Layer.Content)));
        ZoneCursor += 32.0f;

        char SubRun[48];
        std::snprintf(SubRun, sizeof SubRun, "%s \xC2\xB7 %.0f%%", Layer.Transfer, Layer.Opacity);
        Surface.TextRunClipped(ZoneCursor, Card.LeastAcross + 7.0f, Layer.Name, Sheet.InkPrimary, 11.5f, LayerZone.MostAlong - ZoneCursor - 6.0f);
        Surface.TextRunClipped(ZoneCursor, Card.LeastAcross + 22.0f, SubRun, Sheet.InkFaint, 10.0f, LayerZone.MostAlong - ZoneCursor - 6.0f);

        bool LayerZoneRoused = false;
        if (std::snprintf(SeatMould, sizeof SeatMould, "%s.zone", CardIdentity),
            PresentSeat(Spanning(LayerZone.LeastAlong, LayerZone.LeastAcross, LayerZone.SpanAlong(), 44.0f), SeatMould, LayerZoneRoused))
        {
            ActiveLayer = Ordinal;  ActiveTargetMask = false;
        }
        if (ImGui::IsItemActive() && DragOrdinal < 0 &&
            ImGui::GetIO().MouseDown[0] && ImGui::GetIO().MouseDragMaxDistanceSqr[0] > 36.0f)
        {
            DragOrdinal = static_cast<std::int32_t>(Ordinal);   // 📝 the reorder drag — the press stays held past the zone
        }
        if (LayerZoneRoused && ImGui::IsMouseDoubleClicked(0))
            InspectRaised = true;
        if (TakenLayer)
            Surface.Ground(Spanning(LayerZone.LeastAlong + 6.0f, LayerZone.MostAcross - 2.0f, LayerZone.SpanAlong() - 6.0f, 2.0f), Sheet.Marker, 1.0f);

        // ⑤ Divider and mask zone.
        Surface.Ground(DividerColumn, Partial(0xFFFFFFu, 0.06), 0.0f);
        float MaskCursor = MaskZone.LeastAlong;
        if (!Layer.Mask.Enabled)
        {
            const PlaneExtent Dashed = Spanning(MaskCursor, CentredAcross(TopRow, 26.0f), 26.0f, 26.0f);
            Surface.Edge(Dashed, Partial(0x8A8A8Au, 0.4), 1.0f, 5.0f);
            Surface.PlusGlyph(Dashed.LeastAlong + 13.0f, Dashed.LeastAcross + 13.0f, 3.5f, Partial(0x8A8A8Au, 0.4));
            MaskCursor += 32.0f;
            Surface.TextRun(MaskCursor, CentredAcross(TopRow, Surface.RunExtent(10.5f)), "No Mask", Partial(0x8A8A8Au, 0.4), 10.5f);
        }
        else
        {
            const PlaneExtent MaskEye = Spanning(MaskCursor, Card.LeastAcross + 12.0f, 20.0f, 20.0f);
            bool MaskEyeRoused = false;
            if (std::snprintf(SeatMould, sizeof SeatMould, "%s.maskeye", CardIdentity),
            PresentSeat(MaskEye, SeatMould, MaskEyeRoused))
                Layer.Mask.Shown = !Layer.Mask.Shown;
            Surface.EyeGlyph(MaskEye.LeastAlong + 10.0f, MaskEye.LeastAcross + 10.0f, 7.0f, MaskEyeRoused ? Sheet.InkPrimary : Sheet.InkMuted, !Layer.Mask.Shown);
            MaskCursor += 22.0f;

            const PlaneExtent Tone = Spanning(MaskCursor, CentredAcross(TopRow, 26.0f), 26.0f, 26.0f);
            Surface.Ground(Tone, Covering(0x000000u), 6.0f);
            Surface.Edge(Tone, Sheet.StackEdge, 1.0f, 6.0f);
            const double ToneOpacity = Layer.Mask.Shown ? Layer.Mask.Strength / 100.0 : 0.1;
            Surface.Ground(Tone.Inset(2.0f, 2.0f), Partial(0xFFFFFFu, ToneOpacity), 3.0f);
            MaskCursor += 32.0f;

            Surface.TextRun(MaskCursor, Card.LeastAcross + 7.0f, "Mask", Layer.Mask.Shown ? Sheet.InkPrimary : Partial(0xEDEDEDu, 0.4), 11.5f);
            char MaskSub[24];
            std::snprintf(MaskSub, sizeof MaskSub, "%.0f%%", Layer.Mask.Strength);
            Surface.TextRun(MaskCursor, Card.LeastAcross + 22.0f, MaskSub, Layer.Mask.Shown ? Sheet.InkFaint : Partial(0xEDEDEDu, 0.4), 10.0f);

            const PlaneExtent MaskDismiss = Spanning(MaskZone.MostAlong - 22.0f, Card.LeastAcross + 12.0f, 20.0f, 20.0f);
            bool DismissRoused = false;
            if (std::snprintf(SeatMould, sizeof SeatMould, "%s.dismiss", CardIdentity),
            PresentSeat(MaskDismiss, SeatMould, DismissRoused))
                Layer.Mask.Enabled = false;
            Surface.CrossGlyph(MaskDismiss.LeastAlong + 10.0f, MaskDismiss.LeastAcross + 10.0f, 3.5f, DismissRoused ? Sheet.Danger : Sheet.InkMuted);
        }
        const PlaneExtent TrashSeat = Spanning(MaskZone.MostAlong - 20.0f, Card.LeastAcross + 12.0f, 20.0f, 20.0f);
        bool TrashRoused = false;
        if (std::snprintf(SeatMould, sizeof SeatMould, "%s.remove", CardIdentity),
            PresentSeat(TrashSeat, SeatMould, TrashRoused))
            Layer.Removed = true;
        Surface.TrashGlyph(TrashSeat.LeastAlong + 10.0f, TrashSeat.LeastAcross + 10.0f, 6.0f, TrashRoused ? Sheet.Danger : Sheet.InkMuted);

        bool MaskZoneRoused = false;
        if (std::snprintf(SeatMould, sizeof SeatMould, "%s.maskzone", CardIdentity),
            PresentSeat(Spanning(MaskZone.LeastAlong, MaskZone.LeastAcross, MaskZone.SpanAlong() - 22.0f, 44.0f), SeatMould, MaskZoneRoused))
        {
            ActiveLayer = Ordinal;  ActiveTargetMask = true;
        }
        if (MaskZoneRoused && ImGui::IsMouseDoubleClicked(0))
            InspectRaised = true;
        if (TakenMask)
            Surface.Ground(Spanning(MaskZone.LeastAlong, MaskZone.MostAcross - 2.0f, MaskZone.SpanAlong() - 20.0f, 2.0f), Sheet.Marker, 1.0f);

        // ⑥ The folded properties — transfer, opacity, channels, source; mask strength and invert.
        if (Expanded)
        {
            const PlaneExtent Fold = Spanning(Card.LeastAlong, Card.LeastAcross + 44.0f, Card.SpanAlong(), CardExtent - 44.0f);
            Surface.Ground(Fold, Partial(0x000000u, 0.15), 7.0f, CornerSelection::LowerLeading);
            Surface.Ground(Fold, Partial(0x000000u, 0.15), 7.0f, CornerSelection::LowerTrailing);
            Surface.Ground(Spanning(Fold.LeastAlong + 7.0f, Fold.LeastAcross + 7.0f, Fold.SpanAlong() - 14.0f, Fold.SpanAcross() - 7.0f), Partial(0x000000u, 0.15), 0.0f);
            Surface.Rule(Fold.LeastAlong, Fold.LeastAcross, Fold.SpanAlong(), 1.0f, Partial(0xFFFFFFu, 0.06));

            const float ColumnExtent = (Fold.SpanAlong() - 1.0f) * 0.5f;
            const PlaneExtent LeftColumn = Spanning(Fold.LeastAlong, Fold.LeastAcross, ColumnExtent, Fold.SpanAcross());
            const PlaneExtent RightColumn = Spanning(Fold.LeastAlong + ColumnExtent + 1.0f, Fold.LeastAcross, ColumnExtent, Fold.SpanAcross());
            Surface.Ground(Spanning(LeftColumn.MostAlong, Fold.LeastAcross + 8.0f, 1.0f, Fold.SpanAcross() - 16.0f), Partial(0xFFFFFFu, 0.06), 0.0f);

            float RowAcross = LeftColumn.LeastAcross + 8.0f;
            Surface.TextRun(LeftColumn.LeastAlong + 8.0f, CentredAcross(Spanning(0, RowAcross, 0, 26), Surface.RunExtent(10.0f)), "Transfer", Sheet.InkMuted, 10.0f);
            {
                // ① The transfer pickbar with its list.
                const PlaneExtent PickSeat = Spanning(LeftColumn.LeastAlong + 60.0f, RowAcross, LeftColumn.SpanAlong() - 70.0f, 26.0f);
                std::uint32_t TransferOrdinal = 0u;
                for (std::uint32_t Seek = 0u; Seek < 7u; ++Seek)
                    if (std::strcmp(TransferCaptions[Seek], Layer.Transfer) == 0) TransferOrdinal = Seek;
                PresentPickbar(Surface, PickSeat, TransferCaptions[TransferOrdinal], CardIdentity);
                ImGui::PushID(CardIdentity);
                if (ImGui::BeginPopup("pick"))
                {
                    for (std::uint32_t Seek = 0u; Seek < 7u; ++Seek)
                        if (ImGui::Selectable(TransferCaptions[Seek], Seek == TransferOrdinal))
                            std::snprintf(Layer.Transfer, sizeof Layer.Transfer, "%s", TransferCaptions[Seek]);
                    ImGui::EndPopup();
                }
                ImGui::PopID();
            }
            RowAcross += 32.0f;
            Surface.TextRun(LeftColumn.LeastAlong + 8.0f, CentredAcross(Spanning(0, RowAcross, 0, 26), Surface.RunExtent(10.0f)), "Opac", Sheet.InkMuted, 10.0f);
            PresentChannelSlider(Surface, Spanning(LeftColumn.LeastAlong + 60.0f, RowAcross, LeftColumn.SpanAlong() - 68.0f, 26.0f),
                                 Layer.Opacity, 0.0, 100.0, 0u, "%", CardIdentity);
            RowAcross += 34.0f;
            Surface.TextRun(LeftColumn.LeastAlong + 8.0f, RowAcross + 4.0f, "CHANNELS", Sheet.InkFaint, 10.0f);
            RowAcross += 20.0f;
            float ChipAlong = LeftColumn.LeastAlong + 8.0f;
            for (std::uint32_t ChannelOrdinal = 0u; ChannelOrdinal < Layer.ChannelCount; ++ChannelOrdinal)
            {
                const float Size = 10.0f;
                const float ChipExtent = Surface.MeasureRun(Layer.Channels[ChannelOrdinal], Size) + 18.0f;
                if (ChipAlong + ChipExtent > LeftColumn.MostAlong - 8.0f)
                    break;
                const PlaneExtent Chip = Spanning(ChipAlong, RowAcross, ChipExtent, 20.0f);
                Surface.Ground(Chip, Covering(0x1C1C1Cu), 4.0f);
                Surface.Edge(Chip, Covering(0x2A2A2Au), 1.0f, 4.0f);
                Surface.TextRun(Chip.LeastAlong + 9.0f, CentredAcross(Chip, Surface.RunExtent(Size)), Layer.Channels[ChannelOrdinal], Sheet.InkMuted, Size);
                ChipAlong += ChipExtent + 4.0f;
            }
            RowAcross += 26.0f;
            const PlaneExtent FullSeat = Spanning(LeftColumn.LeastAlong + 8.0f, RowAcross, LeftColumn.SpanAlong() - 16.0f, 26.0f);
            bool FullRoused = false;
            if (PresentSeat(FullSeat, CardIdentity, FullRoused))
            {
                ActiveLayer = Ordinal;  ActiveTargetMask = false;  InspectRaised = true;
            }
            Surface.Ground(FullSeat, Sheet.TileGround, 6.0f);
            Surface.Edge(FullSeat, FullRoused ? Covering(0x3A3A3Au) : Partial(0xFFFFFFu, 0.06), 1.0f, 6.0f);
            Surface.TextRun(Surface.CentredAlong(FullSeat, "Full Properties", 10.0f), CentredAcross(FullSeat, Surface.RunExtent(10.0f)),
                            "Full Properties", Sheet.InkPrimary, 10.0f);
            Surface.Chevron(FullSeat.MostAlong - 14.0f, FullSeat.LeastAcross + 13.0f, 3.0f, Sheet.InkMuted, false);

            float MaskRowAcross = RightColumn.LeastAcross + 8.0f;
            if (!Layer.Mask.Enabled)
            {
                const PlaneExtent AddSeat = Spanning(RightColumn.LeastAlong + 14.0f, RightColumn.LeastAcross + 40.0f, 88.0f, 26.0f);
                bool AddMaskRoused = false;
                if (PresentSeat(AddSeat, CardIdentity, AddMaskRoused))
                    Layer.Mask.Enabled = true;
                Surface.Ground(AddSeat, Sheet.Marker, 6.0f);
                Surface.TextRun(Surface.CentredAlong(AddSeat, "Add Mask", 10.0f), CentredAcross(AddSeat, Surface.RunExtent(10.0f)), "Add Mask",
                                InkOrdinate{ 255u, 255u, 255u, 255u }, 10.0f);
            }
            else
            {
                Surface.TextRun(RightColumn.LeastAlong + 8.0f, CentredAcross(Spanning(0, MaskRowAcross, 0, 26), Surface.RunExtent(10.0f)), "Str", Sheet.InkMuted, 10.0f);
                PresentChannelSlider(Surface, Spanning(RightColumn.LeastAlong + 60.0f, MaskRowAcross, RightColumn.SpanAlong() - 68.0f, 26.0f),
                                     Layer.Mask.Strength, 0.0, 100.0, 0u, "%", CardIdentity);
                MaskRowAcross += 32.0f;
                Surface.TextRun(RightColumn.LeastAlong + 8.0f, CentredAcross(Spanning(0, MaskRowAcross, 0, 14), Surface.RunExtent(10.0f)), "Invert", Sheet.InkMuted, 10.0f);
                const PlaneExtent MiniSwitch = Spanning(RightColumn.LeastAlong + 60.0f, CentredAcross(Spanning(0, MaskRowAcross, 0, 26), 14.0f), 26.0f, 14.0f);
                bool InvertRoused = false;
                if (PresentSeat(MiniSwitch, CardIdentity, InvertRoused))
                    Layer.Mask.Invert = !Layer.Mask.Invert;
                Surface.Ground(MiniSwitch, Layer.Mask.Invert ? Sheet.Marker : Covering(0x2A2A2Au), 7.0f);
                Surface.Edge(MiniSwitch, Layer.Mask.Invert ? Sheet.Marker : Covering(0x3A3A3Au), 1.0f, 7.0f);
                Surface.Medallion(Layer.Mask.Invert ? MiniSwitch.MostAlong - 6.0f : MiniSwitch.LeastAlong + 6.0f,
                                  MiniSwitch.LeastAcross + 7.0f, 5.0f, Sheet.InkPrimary);
                MaskRowAcross += 26.0f;
                Surface.TextRun(RightColumn.LeastAlong + 8.0f, MaskRowAcross + 4.0f, "SOURCES", Sheet.InkFaint, 10.0f);
                MaskRowAcross += 20.0f;
                const PlaneExtent SourceChip = Spanning(RightColumn.LeastAlong + 8.0f, MaskRowAcross, RightColumn.SpanAlong() - 16.0f, 24.0f);
                Surface.Ground(SourceChip, Covering(0x101012u), 6.0f);
                Surface.Edge(SourceChip, Partial(0xFFFFFFu, 0.06), 1.0f, 6.0f);
                Surface.Ground(Spanning(SourceChip.LeastAlong + 6.0f, CentredAcross(SourceChip, 12.0f), 12.0f, 12.0f), Covering(0x10B981u), 2.0f);
                Surface.TextRun(SourceChip.LeastAlong + 26.0f, CentredAcross(SourceChip, Surface.RunExtent(10.0f)),
                                Layer.Mask.Source[0] != '\0' ? Layer.Mask.Source : "Generated", Sheet.InkPrimary, 10.0f);
                MaskRowAcross += 30.0f;
                const PlaneExtent FullMaskSeat = Spanning(RightColumn.LeastAlong + 8.0f, MaskRowAcross, RightColumn.SpanAlong() - 16.0f, 26.0f);
                bool FullMaskRoused = false;
                if (PresentSeat(FullMaskSeat, CardIdentity, FullMaskRoused))
                {
                    ActiveLayer = Ordinal;  ActiveTargetMask = true;  InspectRaised = true;
                }
                Surface.Ground(FullMaskSeat, Sheet.TileGround, 6.0f);
                Surface.Edge(FullMaskSeat, FullMaskRoused ? Covering(0x3A3A3Au) : Partial(0xFFFFFFu, 0.06), 1.0f, 6.0f);
                Surface.TextRun(Surface.CentredAlong(FullMaskSeat, "Full Mask", 10.0f), CentredAcross(FullMaskSeat, Surface.RunExtent(10.0f)),
                                "Full Mask", Sheet.InkPrimary, 10.0f);
                Surface.Chevron(FullMaskSeat.MostAlong - 14.0f, FullMaskSeat.LeastAcross + 13.0f, 3.0f, Sheet.InkMuted, false);
            }
        }

        // ②② The dragged card ghosts in place at the reference's opacity-40.
        if (DragOrdinal >= 0 && Ordinal == static_cast<std::uint32_t>(DragOrdinal))
            Surface.Ground(Card, Partial(0x0B0B0Bu, 0.60), 8.0f);

        // ②② A drag-over card carries the marker rail across its upper edge, painted over the card.
        if (DragOrdinal >= 0 && Ordinal != static_cast<std::uint32_t>(DragOrdinal) && Surface.PointerWithin(Card))
            Surface.Ground(Spanning(Card.LeastAlong + 4.0f, Card.LeastAcross - 1.0f, Card.SpanAlong() - 8.0f, 2.0f), Sheet.Marker, 0.0f);

        CursorAcross += CardExtent + 5.0f;
        ++OrdinalSeen;
    }

    // ②② The append boundary when the pointer rests past every card.
    if (DragOrdinal >= 0 && !BoundaryTaken)
    {
        InsertionBoundary = CursorAcross - 2.5f;
        InsertionPresented = PresentedCountLocal;
    }

    // ②② Draw the rail, then splice on release.
    if (DragOrdinal >= 0 && InsertionBoundary > -1.0e8f && InsertionBoundary < 1.0e8f)
        Surface.Ground(Spanning(List.LeastAlong + 6.0f, InsertionBoundary, List.SpanAlong() - 12.0f, 2.0f), Sheet.Marker, 0.0f);

    if (DragReleased)
    {
        const std::uint32_t DraggedOrdinal = static_cast<std::uint32_t>(DragOrdinal);
        const bool DropOnSelfAfter  = InsertionPresented > 0u &&
                                      PresentedOrdinals[InsertionPresented - 1u] == DraggedOrdinal;
        const bool DropOnSelfBefore = InsertionPresented < PresentedCountLocal &&
                                      PresentedOrdinals[InsertionPresented] == DraggedOrdinal;
        if (!DropOnSelfAfter && !DropOnSelfBefore)
        {
            // ① splice: carry the dragged layer out, open its seat, lower it in.
            const LayerOrdinates Carried = Layers[DraggedOrdinal];
            for (std::uint32_t Seek = DraggedOrdinal; Seek + 1u < LayerCount; ++Seek)
                Layers[Seek] = Layers[Seek + 1u];
            std::uint32_t SeatOrdinal = LayerCount - 1u;
            if (InsertionPresented < PresentedCountLocal)
            {
                const std::uint32_t TargetOrdinal = PresentedOrdinals[InsertionPresented];
                SeatOrdinal = TargetOrdinal > DraggedOrdinal ? TargetOrdinal - 1u : TargetOrdinal;
            }
            for (std::uint32_t Seek = LayerCount - 1u; Seek > SeatOrdinal; --Seek)
                Layers[Seek] = Layers[Seek - 1u];
            Layers[SeatOrdinal] = Carried;
        }
        DragOrdinal = -1;
    }

    const float ContentExtent = static_cast<float>(OrdinalSeen) * 49.0f + 8.0f;
    const float Ceiling = ContentExtent > List.SpanAcross() ? ContentExtent - List.SpanAcross() : 0.0f;
    if (ScrollAcross < 0.0f)    ScrollAcross = 0.0f;
    if (ScrollAcross > Ceiling) ScrollAcross = Ceiling;

    // ⑦ Foot — shown and hidden counts.
    const PlaneExtent Foot = Spanning(Seat.LeastAlong, Seat.MostAcross - 26.0f, Seat.SpanAlong(), 26.0f);
    Surface.Ground(Foot, Sheet.StackHead, 0.0f);
    Surface.Rule(Foot.LeastAlong, Foot.LeastAcross, Foot.SpanAlong(), 1.0f, Sheet.StackEdge);
    std::uint32_t ShownCount = 0u;
    for (std::uint32_t Ordinal = 0u; Ordinal < LayerCount; ++Ordinal)
        if (!Layers[Ordinal].Removed && Layers[Ordinal].Shown)
            ++ShownCount;
    char ShownRun[48];
    std::snprintf(ShownRun, sizeof ShownRun, "%u shown", ShownCount);
    char HiddenRun[32];
    std::snprintf(HiddenRun, sizeof HiddenRun, "%u hidden", StandingCount - ShownCount);
    Surface.Ground(Spanning(Foot.LeastAlong + 11.0f, CentredAcross(Foot, 7.0f) + 3.5f, 7.0f, 7.0f), Covering(0x94A3B8u), 2.0f);
    const float FootAcross = CentredAcross(Foot, Surface.RunExtent(10.0f));
    Surface.TextRun(Foot.LeastAlong + 24.0f, FootAcross, ShownRun, Sheet.InkMuted, 10.0f);
    Surface.Medallion(Foot.LeastAlong + 24.0f + Surface.MeasureRun(ShownRun, 10.0f) + 7.0f, Foot.LeastAcross + 13.0f, 1.0f, Covering(0x2C2C2Cu));
    Surface.TextRun(Foot.LeastAlong + 24.0f + Surface.MeasureRun(ShownRun, 10.0f) + 15.0f, FootAcross, HiddenRun, Sheet.InkMuted, 10.0f);
    Surface.TextRun(Foot.MostAlong - Surface.MeasureRun("drag to reorder", 10.0f) - 11.0f, FootAcross, "drag to reorder", Sheet.InkMuted, 10.0f);
}

//------------------------------------------------------------------------------------------------------------------------
//                                                  THE CHANNEL PROPERTY SEAT
//------------------------------------------------------------------------------------------------------------------------

void ChannelPropertyPanel::Advance(RecordingSurface& Surface, const PlaneExtent& Seat, ChannelOrdinates& Ordinates, const IconDepot& Depot)
{
    ChannelInk Sheet;
    Surface.Ground(Seat, Sheet.DeskGround, 0.0f);

    // ① Head.
    const PlaneExtent Head = Spanning(Seat.LeastAlong, Seat.LeastAcross, Seat.SpanAlong(), 46.0f);
    Surface.Ground(Head, Sheet.SunkenGround, 0.0f);
    Surface.Rule(Head.LeastAlong, Head.MostAcross - 1.0f, Head.SpanAlong(), 1.0f, Sheet.HairEdge);
    const PlaneExtent Tile = Spanning(Head.LeastAlong + 10.0f, CentredAcross(Head, 24.0f), 24.0f, 24.0f);
    Surface.Ground(Tile, Sheet.DeskGround, 6.0f);
    Surface.Edge(Tile, Sheet.HairEdge, 1.0f, 6.0f);
    Depot.PresentGlyph(Surface, Tile.Inset(5.0f, 5.0f), Sheet.InkMuted);
    Surface.TextRun(Tile.MostAlong + 9.0f, Head.LeastAcross + 8.0f, Ordinates.LayerName, Sheet.InkPrimary, 12.5f);
    char SubRun[64];
    std::snprintf(SubRun, sizeof SubRun, "%s \xC2\xB7 %s", ContentRun(Ordinates.Classification), "Normal");
    Surface.TextRun(Tile.MostAlong + 9.0f, Head.LeastAcross + 25.0f, SubRun, Sheet.InkFaint, 10.0f);

    const PlaneExtent BackSeat = Spanning(Head.MostAlong - 76.0f, CentredAcross(Head, 28.0f), 66.0f, 28.0f);
    Surface.Ground(BackSeat, Sheet.TileGround, 6.0f);
    Surface.Edge(BackSeat, Sheet.HairEdge, 1.0f, 6.0f);
    Surface.Chevron(BackSeat.LeastAlong + 12.0f, BackSeat.LeastAcross + 14.0f, 4.0f, Sheet.InkPrimary, false);
    Surface.TextRun(BackSeat.LeastAlong + 22.0f, CentredAcross(BackSeat, Surface.RunExtent(11.0f)), "Back", Sheet.InkPrimary, 11.0f);

    // ② Body, scrolled by wheel.
    const PlaneExtent Body = Spanning(Seat.LeastAlong, Head.MostAcross, Seat.SpanAlong(), Seat.MostAcross - 28.0f - Head.MostAcross);
    if (Surface.PointerWithin(Body))
        ScrollAcross -= ImGui::GetIO().MouseWheel * 32.0f;
    float CursorAcross = Body.LeastAcross - ScrollAcross + 8.0f;

    // ③ Chips region.
    std::uint32_t EnabledCount = 0u;
    for (std::uint32_t Ordinal = 0u; Ordinal < 14u; ++Ordinal)
        if (Ordinates.Enabled[Ordinal]) ++EnabledCount;

    const float ChipRows = 1.0f + (EnabledCount > 3 ? (EnabledCount - 1) / 3 : 0);
    const float ChipsHeight = 18.0f + ChipRows * 33.0f + 9.0f;
    const PlaneExtent ChipsRegion = Spanning(Seat.LeastAlong + 8.0f, CursorAcross, Seat.SpanAlong() - 16.0f, ChipsHeight);
    Surface.Ground(ChipsRegion, Sheet.SunkenGround, 12.0f);
    Surface.Edge(ChipsRegion, Sheet.HairEdge, 1.0f, 12.0f);

    Surface.TextRun(ChipsRegion.LeastAlong + 10.0f, ChipsRegion.LeastAcross + 9.0f, "CHANNELS", Sheet.InkFaint, 10.0f);
    char CountFigures[8];
    std::snprintf(CountFigures, sizeof CountFigures, "%u", EnabledCount);
    const PlaneExtent CountBadge = Spanning(ChipsRegion.LeastAlong + 10.0f + Surface.MeasureRun("CHANNELS", 10.0f) + 8.0f, ChipsRegion.LeastAcross + 7.0f,
                                            Surface.MeasureRun(CountFigures, 10.0f) + 12.0f, 14.0f);
    Surface.Ground(CountBadge, Sheet.Accent, 9.0f);
    Surface.TextRun(Surface.CentredAlong(CountBadge, CountFigures, 10.0f), CentredAcross(CountBadge, Surface.RunExtent(10.0f)), CountFigures, Sheet.OnAccent, 10.0f);
    Surface.TextRun(ChipsRegion.MostAlong - Surface.MeasureRun("Clear all", 10.0f) - 10.0f, ChipsRegion.LeastAcross + 9.0f, "Clear all", Sheet.InkFaint, 10.0f);

    float ChipAlong = ChipsRegion.LeastAlong + 10.0f;
    float ChipAcross = ChipsRegion.LeastAcross + 26.0f;
    for (std::uint32_t Ordinal = 0u; Ordinal < 14u; ++Ordinal)
    {
        if (!Ordinates.Enabled[Ordinal])
            continue;
        const float Size = 11.5f;
        const float ChipExtent = Surface.MeasureRun(Slots[Ordinal].Label, Size) + 10.0f + 9.0f + 6.0f + 17.0f + 6.0f + 5.0f;
        if (ChipAlong + ChipExtent > ChipsRegion.MostAlong - 40.0f)
        {
            ChipAlong = ChipsRegion.LeastAlong + 10.0f;
            ChipAcross += 33.0f;
        }
        const PlaneExtent Chip = Spanning(ChipAlong, ChipAcross, ChipExtent, 27.0f);
        Surface.Ground(Chip, Sheet.TileGround, 13.0f);
        Surface.Edge(Chip, Sheet.HairEdge, 1.0f, 13.0f);
        Surface.Medallion(Chip.LeastAlong + 15.0f, Chip.LeastAcross + 13.5f, 4.5f, Covering(Slots[Ordinal].HuePacked));
        Surface.TextRun(Chip.LeastAlong + 25.0f, CentredAcross(Chip, Surface.RunExtent(Size)), Slots[Ordinal].Label, Sheet.InkPrimary, Size);
        const PlaneExtent RemoveSeat = Spanning(Chip.MostAlong - 22.0f, Chip.LeastAcross + 5.0f, 17.0f, 17.0f);
        bool RemoveRoused = false;
        char Identity[32];
        std::snprintf(Identity, sizeof Identity, "chip.%u", Ordinal);
        if (PresentSeat(RemoveSeat, Identity, RemoveRoused) && Ordinal != 0u)
            Ordinates.Enabled[Ordinal] = false;
        Surface.Medallion(RemoveSeat.LeastAlong + 8.5f, RemoveSeat.LeastAcross + 8.5f, 8.5f, Sheet.TileTaken);
        Surface.CrossGlyph(RemoveSeat.LeastAlong + 8.5f, RemoveSeat.LeastAcross + 8.5f, 3.0f, RemoveRoused ? Sheet.Danger : Sheet.InkMuted);
        ChipAlong += ChipExtent + 6.0f;
    }
    const PlaneExtent AddSeat = Spanning(ChipAlong, ChipAcross, 27.0f, 27.0f);
    Surface.Ring(AddSeat.LeastAlong + 13.5f, AddSeat.LeastAcross + 13.5f, 13.0f, 1.0f, Sheet.HairEdge);
    Surface.PlusGlyph(AddSeat.LeastAlong + 13.5f, AddSeat.LeastAcross + 13.5f, 4.5f, Sheet.InkMuted);

    CursorAcross = ChipsRegion.MostAcross + 6.0f;

    // ④ Channel cards.
    float CardCeiling = CursorAcross;
    for (std::uint32_t Ordinal = 0u; Ordinal < 14u; ++Ordinal)
    {
        if (!Ordinates.Enabled[Ordinal])
            continue;
        const ChannelSlotDeclaration& Slot = Slots[Ordinal];
        const bool Collapsed = Ordinates.Collapsed[Ordinal];
        // ①① Card extents seat their content exactly — head, pads, every row band — so no row spills under the next card.
        float CardExtent = 29.0f + 6.0f;
        if (!Collapsed)
        {
            if (Slot.Edit == ChannelEdit::Derived)
            {
                CardExtent += 20.0f + 34.0f + 8.0f;
            }
            else
            {
                CardExtent += 36.0f;   // [px] - source segments
                if (Ordinates.Source[Ordinal] == 1u)
                    CardExtent += 56.0f + 16.0f + (Ordinates.TextureSeated[Ordinal] ? 56.0f : 48.0f);
                else if (Ordinates.Source[Ordinal] == 2u)
                    CardExtent += 34.0f + static_cast<float>(Generators[Ordinates.Generator[Ordinal]].ParameterCount) * 32.0f + 6.0f;
                else
                    CardExtent += 32.0f;
                CardExtent += 42.0f + 8.0f;   // [px] - preview row and trailing pad
            }
        }
        const PlaneExtent Card = Spanning(Seat.LeastAlong + 8.0f, CursorAcross, Seat.SpanAlong() - 16.0f, CardExtent);

        if (Card.LeastAcross < Body.MostAcross && Card.MostAcross > Body.LeastAcross)
        {
            Surface.Ground(Card, Sheet.StandingGround, 10.0f);
            Surface.Edge(Card, Sheet.HairEdge, 1.0f, 10.0f);

            // ① Card head — chevron, hue dot, title, source run.
            const PlaneExtent CardHead = Spanning(Card.LeastAlong, Card.LeastAcross, Card.SpanAlong(), 29.0f);
            const PlaneExtent TwistSeat = Spanning(Card.LeastAlong + 9.0f, Card.LeastAcross, 12.0f, 29.0f);
            char Identity[32];
            std::snprintf(Identity, sizeof Identity, "card.%u.twist", Ordinal);
            bool TwistRoused = false;
            if (PresentSeat(Spanning(Card.LeastAlong + 8.0f, Card.LeastAcross, 120.0f, 29.0f), Identity, TwistRoused))
                Ordinates.Collapsed[Ordinal] = !Ordinates.Collapsed[Ordinal];
            Surface.Chevron(TwistSeat.LeastAlong + 5.0f, Card.LeastAcross + 14.5f, 3.5f, TwistRoused ? Sheet.InkPrimary : Sheet.InkFaint, !Collapsed);
            Surface.Medallion(Card.LeastAlong + 28.0f, Card.LeastAcross + 14.5f, 3.0f, Covering(Slot.HuePacked));
            Surface.TextRun(Card.LeastAlong + 38.0f, CentredAcross(CardHead, Surface.RunExtent(11.0f)), Slot.Label, Sheet.InkPrimary, 11.0f);

            const char* SourceRun = Ordinates.Source[Ordinal] == 0u ? "Value" : (Ordinates.Source[Ordinal] == 1u ? "Texture" : "Generator");
            if (Slot.Edit == ChannelEdit::Derived) SourceRun = "Derived";
            Surface.TextRun(Card.MostAlong - Surface.MeasureRun(SourceRun, 10.0f) - 9.0f, CentredAcross(CardHead, Surface.RunExtent(10.0f)),
                            SourceRun, Sheet.InkFaint, 10.0f);

            if (!Collapsed)
            {
                Surface.Rule(Card.LeastAlong, Card.LeastAcross + 29.0f, Card.SpanAlong(), 1.0f, Sheet.HairEdge);
                const PlaneExtent CardBody = Spanning(Card.LeastAlong + 9.0f, Card.LeastAcross + 29.0f + 6.0f, Card.SpanAlong() - 18.0f, CardExtent - 29.0f - 14.0f);
                float RowAcross = CardBody.LeastAcross;

                const auto PropertyRow = [&](const char* Caption, float ExtentAlong)
                {
                    Surface.TextRun(CardBody.LeastAlong, CentredAcross(Spanning(0, RowAcross, 0, 32), Surface.RunExtent(11.5f)), Caption, Sheet.InkMuted, 11.5f);
                    return Spanning(CardBody.LeastAlong + 98.0f, RowAcross, CardBody.SpanAlong() - 98.0f - 4.0f, ExtentAlong);
                };

                if (Slot.Edit == ChannelEdit::Derived)
                {
                    Surface.TextRun(CardBody.LeastAlong, RowAcross, "Derived from the painted height. No value to author.", Sheet.InkFaint, 10.5f);
                    RowAcross += 20.0f;
                    PresentCheckerTile(Surface, Spanning(CardBody.LeastAlong, RowAcross, 34.0f, 34.0f));
                    Surface.TextRun(CardBody.LeastAlong + 44.0f, RowAcross + 2.0f, "Derived preview", Sheet.InkMuted, 10.5f);
                    Surface.TextRun(CardBody.LeastAlong + 44.0f, RowAcross + 16.0f, Slot.PlacementRun, Sheet.InkFaint, 10.0f);
                }
                else
                {
                    // ② Source segments.
                    static const char* const SourceCaptions[3] = { "Value", "Texture", "Generator" };
                    PresentChannelSegments(Surface, PropertyRow("Source", 26.0f), SourceCaptions, 3u, Ordinates.Source[Ordinal], Identity);
                    RowAcross += 36.0f;

                    if (Ordinates.Source[Ordinal] == 1u)
                    {
                        // ③ Texture — the stroke slot and the imported base.
                        const PlaneExtent StrokeSlot = Spanning(CardBody.LeastAlong, RowAcross, CardBody.SpanAlong(), 50.0f);
                        Surface.Ground(StrokeSlot, Sheet.TileGround, 9.0f);
                        Surface.Edge(StrokeSlot, Sheet.HairEdge, 1.0f, 9.0f);
                        const PlaneExtent Thumb = Spanning(StrokeSlot.LeastAlong + 6.0f, StrokeSlot.LeastAcross + 6.0f, 38.0f, 38.0f);
                        if (Ordinates.Strokes[Ordinal] > 0u)
                        {
                            Surface.Ground(Thumb, Covering(Slot.HuePacked), 6.0f);
                            Surface.Edge(Thumb, Sheet.HairEdge, 1.0f, 6.0f);
                        }
                        else
                            PresentCheckerTile(Surface, Thumb);
                        char NameRun[48];
                        char MetaRun[64];
                        if (Ordinates.Strokes[Ordinal] > 0u)
                        {
                            std::snprintf(NameRun, sizeof NameRun, "Painted strokes");
                            std::snprintf(MetaRun, sizeof MetaRun, "%u strokes \xC2\xB7 2048 \xC3\x97 2048 atlas", Ordinates.Strokes[Ordinal]);
                        }
                        else
                        {
                            std::snprintf(NameRun, sizeof NameRun, "No strokes yet");
                            std::snprintf(MetaRun, sizeof MetaRun, "Atlas allocates on the first stroke");
                        }
                        Surface.TextRun(StrokeSlot.LeastAlong + 54.0f, StrokeSlot.LeastAcross + 9.0f, NameRun, Sheet.InkPrimary, 11.0f);
                        Surface.TextRun(StrokeSlot.LeastAlong + 54.0f, StrokeSlot.LeastAcross + 25.0f, MetaRun, Sheet.InkFaint, 10.0f);
                        const PlaneExtent StrokeVerb = Spanning(StrokeSlot.MostAlong - 30.0f, StrokeSlot.LeastAcross + 13.0f, 24.0f, 24.0f);
                        Surface.Ground(StrokeVerb, Sheet.SunkenGround, 6.0f);
                        Surface.Edge(StrokeVerb, Sheet.HairEdge, 1.0f, 6.0f);
                        Surface.TrashGlyph(StrokeVerb.LeastAlong + 12.0f, StrokeVerb.LeastAcross + 12.0f, 6.0f, Sheet.InkMuted);
                        RowAcross += 56.0f;

                        Surface.TextRun(CardBody.LeastAlong, RowAcross, Ordinates.TextureSeated[Ordinal] ? "IMPORTED BASE" : "IMPORTED BASE \xE2\x80\x94 OPTIONAL", Sheet.InkFaint, 10.0f);
                        RowAcross += 16.0f;
                        if (Ordinates.TextureSeated[Ordinal])
                        {
                            const PlaneExtent BaseSlot = Spanning(CardBody.LeastAlong, RowAcross, CardBody.SpanAlong(), 50.0f);
                            Surface.Ground(BaseSlot, Sheet.TileGround, 9.0f);
                            Surface.Edge(BaseSlot, Sheet.HairEdge, 1.0f, 9.0f);
                            PresentCheckerTile(Surface, Spanning(BaseSlot.LeastAlong + 6.0f, BaseSlot.LeastAcross + 6.0f, 38.0f, 38.0f));
                            Surface.TextRun(BaseSlot.LeastAlong + 54.0f, BaseSlot.LeastAcross + 9.0f, "copper_basecolour.png", Sheet.InkPrimary, 11.0f);
                            Surface.TextRun(BaseSlot.LeastAlong + 54.0f, BaseSlot.LeastAcross + 25.0f, "2048 \xC3\x97 2048 \xC2\xB7 sRGB 8", Sheet.InkFaint, 10.0f);
                        }
                        else
                        {
                            const PlaneExtent EmptySeat = Spanning(CardBody.LeastAlong, RowAcross, CardBody.SpanAlong(), 44.0f);
                            bool EmptyRoused = false;
                            if (PresentSeat(EmptySeat, Identity, EmptyRoused))
                                Ordinates.TextureSeated[Ordinal] = true;
                            Surface.Edge(EmptySeat, Sheet.HairEdge, 1.0f, 9.0f);
                            Surface.PlusGlyph(EmptySeat.LeastAlong + Surface.CentredAlong(EmptySeat, "Import base texture", 11.0f) - 40.0f,
                                              EmptySeat.LeastAcross + 22.0f, 5.0f, Sheet.InkMuted);
                            Surface.TextRun(EmptySeat.LeastAlong + Surface.CentredAlong(EmptySeat, "Import base texture", 11.0f),
                                            CentredAcross(EmptySeat, Surface.RunExtent(11.0f)), "Import base texture", Sheet.InkMuted, 11.0f);
                        }
                        RowAcross += Ordinates.TextureSeated[Ordinal] ? 56.0f : 48.0f;   // 📝 the base slot's band, before the preview
                    }
                    else if (Ordinates.Source[Ordinal] == 2u)
                    {
                        // ④ Generator — pickbar and parameters.
                        const GeneratorDeclaration& Chosen = Generators[Ordinates.Generator[Ordinal]];
                        PresentPickbar(Surface, PropertyRow("Generator", 26.0f), Chosen.Key[0] != '\0' ? Chosen.Label : "Choose generator", Identity);
                        RowAcross += 34.0f;
                        Surface.Rule(CardBody.LeastAlong, RowAcross - 6.0f, CardBody.SpanAlong(), 1.0f, Sheet.HairEdge);
                        for (std::uint32_t ParameterOrdinal = 0u; ParameterOrdinal < Chosen.ParameterCount; ++ParameterOrdinal)
                        {
                            PresentChannelSlider(Surface, PropertyRow(Chosen.ParameterLabels[ParameterOrdinal], 26.0f),
                                                 Ordinates.GeneratorParameters[Ordinal][ParameterOrdinal], 0.0, 1.0, 2u, "-", Identity);
                            RowAcross += 32.0f;
                        }
                    }
                    else if (Slot.Edit == ChannelEdit::Colour)
                    {
                        // ⑤ Colour bar.
                        const PlaneExtent Bar = PropertyRow("Colour", 26.0f);
                        Surface.Ground(Bar, Sheet.TileGround, 13.0f);
                        Surface.Edge(Bar, Sheet.HairEdge, 1.0f, 13.0f);
                        Surface.Medallion(Bar.LeastAlong + 16.0f, Bar.LeastAcross + 13.0f, 9.0f, Covering(Ordinates.Colour[Ordinal]));
                        char ColourRun[16];
                        std::snprintf(ColourRun, sizeof ColourRun, "#%06X", Ordinates.Colour[Ordinal] & 0xFFFFFFu);
                        Surface.TextRun(Bar.LeastAlong + 32.0f, CentredAcross(Bar, Surface.RunExtent(11.0f)), ColourRun, Sheet.KnobInk, 11.0f);
                        Surface.Chevron(Bar.MostAlong - 14.0f, Bar.LeastAcross + 13.0f - 1.0f, 3.0f, Sheet.InkFaint, true);
                        RowAcross += 32.0f;
                    }
                    else
                    {
                        // ⑥ Scalar amount.
                        PresentChannelSlider(Surface, PropertyRow("Amount", 26.0f), Ordinates.Amount[Ordinal], Slot.Minimum, Slot.Maximum,
                                             std::strcmp(Slot.Unit, "\xC2\xB0") == 0 ? 0u : 2u, Slot.Unit, Identity);
                        RowAcross += 32.0f;
                    }

                    // ⑦ Preview row.
                    const PlaneExtent PrevTile = Spanning(CardBody.LeastAlong, RowAcross, 34.0f, 34.0f);
                    if (Slot.Edit == ChannelEdit::Colour && Ordinates.Source[Ordinal] == 0u)
                        Surface.Ground(PrevTile, Covering(Ordinates.Colour[Ordinal]), 7.0f);
                    else if (Slot.Edit == ChannelEdit::Scalar && Ordinates.Source[Ordinal] == 0u)
                    {
                        const double Fraction = (Ordinates.Amount[Ordinal] - Slot.Minimum) / (Slot.Maximum - Slot.Minimum);
                        const std::uint8_t Lumen = static_cast<std::uint8_t>(Fraction * 255.0 + 0.5);
                        Surface.Ground(PrevTile, InkOrdinate{ Lumen, Lumen, Lumen, 255u }, 7.0f);
                    }
                    else
                        PresentCheckerTile(Surface, PrevTile);
                    Surface.TextRun(CardBody.LeastAlong + 44.0f, RowAcross + 3.0f, Ordinates.Source[Ordinal] == 0u ? "Value preview" :
                                    (Ordinates.Source[Ordinal] == 1u ? "Texture preview" : "Generator preview"), Sheet.InkMuted, 10.5f);
                    Surface.TextRun(CardBody.LeastAlong + 44.0f, RowAcross + 17.0f, Slot.PlacementRun, Sheet.InkFaint, 10.0f);
                }
            }
        }
        CursorAcross += CardExtent + 5.0f;
        CardCeiling = CursorAcross;
    }

    const float Ceiling = CardCeiling - ScrollAcross > Body.SpanAcross() ? CardCeiling - ScrollAcross - Body.SpanAcross() : 0.0f;
    if (ScrollAcross < 0.0f) ScrollAcross = 0.0f;
    if (ScrollAcross > Ceiling) ScrollAcross = Ceiling;

    // ⑤ Foot.
    const PlaneExtent Foot = Spanning(Seat.LeastAlong, Seat.MostAcross - 28.0f, Seat.SpanAlong(), 28.0f);
    Surface.Ground(Foot, Sheet.SunkenGround, 0.0f);
    Surface.Rule(Foot.LeastAlong, Foot.LeastAcross, Foot.SpanAlong(), 1.0f, Sheet.HairEdge);
    char ChannelsRun[32];
    std::snprintf(ChannelsRun, sizeof ChannelsRun, "%u channels", EnabledCount);
    char AtlasRun[32];
    std::snprintf(AtlasRun, sizeof AtlasRun, "%u atlases", 5u);
    Surface.TextRun(Foot.LeastAlong + 11.0f, CentredAcross(Foot, Surface.RunExtent(10.0f)), ChannelsRun, Sheet.InkFaint, 10.0f);
    Surface.TextRun(Foot.MostAlong - Surface.MeasureRun(AtlasRun, 10.0f) - 11.0f, CentredAcross(Foot, Surface.RunExtent(10.0f)), AtlasRun, Sheet.InkFaint, 10.0f);
}

//------------------------------------------------------------------------------------------------------------------------
//                                                   THE MASK PROPERTY SEAT
//------------------------------------------------------------------------------------------------------------------------

void MaskPropertyPanel::Advance(RecordingSurface& Surface, const PlaneExtent& Seat, MaskOrdinates& Ordinates, const IconDepot& Depot)
{
    ChannelInk Sheet;
    Surface.Ground(Seat, Sheet.StandingGround, 0.0f);

    // ① Head.
    const PlaneExtent Head = Spanning(Seat.LeastAlong, Seat.LeastAcross, Seat.SpanAlong(), 46.0f);
    Surface.Ground(Head, Sheet.SunkenGround, 0.0f);
    Surface.Rule(Head.LeastAlong, Head.MostAcross - 1.0f, Head.SpanAlong(), 1.0f, Sheet.HairEdge);
    const PlaneExtent Tile = Spanning(Head.LeastAlong + 10.0f, CentredAcross(Head, 24.0f), 24.0f, 24.0f);
    Surface.Ground(Tile, Sheet.DeskGround, 6.0f);
    Surface.Edge(Tile, Sheet.HairEdge, 1.0f, 6.0f);
    Depot.PresentGlyph(Surface, Tile.Inset(5.0f, 5.0f), Sheet.InkMuted);
    Surface.TextRun(Tile.MostAlong + 9.0f, Head.LeastAcross + 8.0f, Ordinates.LayerName, Sheet.InkPrimary, 12.5f);
    Surface.TextRun(Tile.MostAlong + 9.0f, Head.LeastAcross + 25.0f, "Mask \xC2\xB7 Multiply", Sheet.InkFaint, 10.0f);

    const PlaneExtent BackSeat = Spanning(Head.MostAlong - 76.0f, CentredAcross(Head, 28.0f), 66.0f, 28.0f);
    Surface.Ground(BackSeat, Sheet.TileGround, 6.0f);
    Surface.Edge(BackSeat, Sheet.HairEdge, 1.0f, 6.0f);
    Surface.Chevron(BackSeat.LeastAlong + 12.0f, BackSeat.LeastAcross + 14.0f, 4.0f, Sheet.InkPrimary, false);
    Surface.TextRun(BackSeat.LeastAlong + 22.0f, CentredAcross(BackSeat, Surface.RunExtent(11.0f)), "Back", Sheet.InkPrimary, 11.0f);

    // ② The one foldable section.
    const float SectionExtent = Ordinates.Collapsed ? 29.0f : 420.0f;
    const PlaneExtent Section = Spanning(Seat.LeastAlong + 8.0f, Head.MostAcross + 8.0f, Seat.SpanAlong() - 16.0f, SectionExtent);
    Surface.Ground(Section, Sheet.StandingGround, 10.0f);
    Surface.Edge(Section, Sheet.HairEdge, 1.0f, 10.0f);

    const PlaneExtent SectionHead = Spanning(Section.LeastAlong, Section.LeastAcross, Section.SpanAlong(), 29.0f);
    bool TwistRoused = false;
    if (PresentSeat(SectionHead, "mask.twist", TwistRoused))
        Ordinates.Collapsed = !Ordinates.Collapsed;
    Surface.Chevron(Section.LeastAlong + 14.0f, Section.LeastAcross + 14.5f, 3.5f, TwistRoused ? Sheet.InkPrimary : Sheet.InkFaint, !Ordinates.Collapsed);
    Surface.Medallion(Section.LeastAlong + 32.0f, Section.LeastAcross + 14.5f, 3.0f, Covering(0xA78BFAu));
    Surface.TextRun(Section.LeastAlong + 42.0f, CentredAcross(SectionHead, Surface.RunExtent(11.0f)), "Mask", Sheet.InkPrimary, 11.0f);
    Surface.TextRun(Section.MostAlong - Surface.MeasureRun(Ordinates.SourceMode == 0u ? "Texture" : "Generator", 10.0f) - 9.0f,
                    CentredAcross(SectionHead, Surface.RunExtent(10.0f)), Ordinates.SourceMode == 0u ? "Texture" : "Generator", Sheet.InkFaint, 10.0f);

    if (Ordinates.Collapsed)
        return;
    Surface.Rule(Section.LeastAlong, Section.MostAcross, Section.SpanAlong(), 1.0f, Sheet.HairEdge);

    float RowAcross = Section.LeastAcross + 40.0f;
    const PlaneExtent Body = Spanning(Section.LeastAlong + 9.0f, RowAcross, Section.SpanAlong() - 18.0f, 0.0f);

    const auto PropertyRow = [&](const char* Caption, float ExtentAlong) -> PlaneExtent
    {
        Surface.TextRun(Body.LeastAlong, CentredAcross(Spanning(0, RowAcross, 0, 32), Surface.RunExtent(11.5f)), Caption, Sheet.InkMuted, 11.5f);
        return Spanning(Body.LeastAlong + 98.0f, RowAcross, Body.SpanAlong() - 98.0f - 4.0f, ExtentAlong);
    };

    // ③ Base fill, invert, strength.
    static const char* const BaseCaptions[2] = { "White", "Black" };
    PresentChannelSegments(Surface, PropertyRow("Base Mask", 26.0f), BaseCaptions, 2u, Ordinates.BaseFill, "mask.base");
    RowAcross += 40.0f;
    static const char* const InvertCaptions[2] = { "Off", "On" };
    std::uint32_t InvertOrdinal = Ordinates.Invert ? 1u : 0u;
    PresentChannelSegments(Surface, PropertyRow("Invert", 26.0f), InvertCaptions, 2u, InvertOrdinal, "mask.invert");
    Ordinates.Invert = InvertOrdinal == 1u;
    RowAcross += 40.0f;
    PresentChannelSlider(Surface, PropertyRow("Strength", 26.0f), Ordinates.Strength, 0.0, 1.0, 2u, "-", "mask.strength");
    RowAcross += 44.0f;
    Surface.Rule(Body.LeastAlong, RowAcross - 6.0f, Body.SpanAlong(), 1.0f, Sheet.HairEdge);

    // ④ Source.
    static const char* const SourceCaptions[2] = { "Texture", "Generator" };
    PresentChannelSegments(Surface, PropertyRow("Source", 26.0f), SourceCaptions, 2u, Ordinates.SourceMode, "mask.source");
    RowAcross += 44.0f;

    if (Ordinates.SourceMode == 0u)
    {
        // ⑤ The stroke slot and the imported base.
        const PlaneExtent StrokeSlot = Spanning(Body.LeastAlong, RowAcross, Body.SpanAlong(), 50.0f);
        Surface.Ground(StrokeSlot, Sheet.TileGround, 9.0f);
        Surface.Edge(StrokeSlot, Sheet.HairEdge, 1.0f, 9.0f);
        PresentCheckerTile(Surface, Spanning(StrokeSlot.LeastAlong + 6.0f, StrokeSlot.LeastAcross + 6.0f, 38.0f, 38.0f));
        char StrokesRun[64];
        if (Ordinates.Strokes > 0u)
            std::snprintf(StrokesRun, sizeof StrokesRun, "%u painted strokes \xC2\xB7 2048 \xC3\x97 2048", Ordinates.Strokes);
        else
            std::snprintf(StrokesRun, sizeof StrokesRun, "No strokes yet \xC2\xB7 atlas allocates on the first");
        Surface.TextRun(StrokeSlot.LeastAlong + 54.0f, StrokeSlot.LeastAcross + 9.0f, Ordinates.Strokes > 0u ? "Painted strokes" : "No strokes yet", Sheet.InkPrimary, 11.0f);
        Surface.TextRun(StrokeSlot.LeastAlong + 54.0f, StrokeSlot.LeastAcross + 25.0f, StrokesRun, Sheet.InkFaint, 10.0f);
        RowAcross += 56.0f;
    }
    const GeneratorDeclaration& Chosen = Generators[Ordinates.Generator];
    if (Ordinates.SourceMode != 0u)
    {
        // ⑥ The generator pickbar and its parameters.
        PresentPickbar(Surface, PropertyRow("Generator", 26.0f), Chosen.Label, "mask.generator");
        RowAcross += 34.0f;
        Surface.Rule(Body.LeastAlong, RowAcross - 6.0f, Body.SpanAlong(), 1.0f, Sheet.HairEdge);
        Surface.TextRun(Body.LeastAlong, RowAcross - 24.0f, Chosen.NoteRun, Sheet.InkFaint, 10.0f);
        for (std::uint32_t ParameterOrdinal = 0u; ParameterOrdinal < Chosen.ParameterCount; ++ParameterOrdinal)
        {
            PresentChannelSlider(Surface, PropertyRow(Chosen.ParameterLabels[ParameterOrdinal], 26.0f),
                                 Ordinates.GeneratorParameters[ParameterOrdinal], 0.0, 1.0, 2u, "-", "mask.param");
            RowAcross += 32.0f;
        }
        RowAcross += 6.0f;
    }

    // ⑦ Preview row and foot.
    PresentCheckerTile(Surface, Spanning(Body.LeastAlong, RowAcross, 34.0f, 34.0f));
    Surface.TextRun(Body.LeastAlong + 44.0f, RowAcross + 3.0f, Ordinates.SourceMode == 0u ? "Texture preview" : "Generator preview", Sheet.InkMuted, 10.5f);
    Surface.TextRun(Body.LeastAlong + 44.0f, RowAcross + 17.0f, Ordinates.SourceMode == 0u ? "Mask atlas \xC2\xB7 R" : Chosen.NoteRun, Sheet.InkFaint, 10.0f);

    const PlaneExtent Foot = Spanning(Seat.LeastAlong, Seat.MostAcross - 28.0f, Seat.SpanAlong(), 28.0f);
    Surface.Ground(Foot, Sheet.SunkenGround, 0.0f);
    Surface.Rule(Foot.LeastAlong, Foot.LeastAcross, Foot.SpanAlong(), 1.0f, Sheet.HairEdge);
    char StrokesFoot[48];
    std::snprintf(StrokesFoot, sizeof StrokesFoot, "%u strokes", Ordinates.Strokes);
    Surface.TextRun(Foot.LeastAlong + 11.0f, CentredAcross(Foot, Surface.RunExtent(10.0f)), StrokesFoot, Sheet.InkFaint, 10.0f);
    const PlaneExtent DeleteSeat = Spanning(Foot.MostAlong - 92.0f, CentredAcross(Foot, 22.0f), 82.0f, 22.0f);
    bool DeleteRoused = false;
    if (PresentSeat(DeleteSeat, "mask.delete", DeleteRoused))
        Ordinates.Present = false;
    Surface.Ground(DeleteSeat, Partial(0xE05A5Au, DeleteRoused ? 0.22 : 0.0), 6.0f);
    Surface.Edge(DeleteSeat, Partial(0xE05A5Au, 0.4), 1.0f, 6.0f);
    Surface.TrashGlyph(DeleteSeat.LeastAlong + 16.0f, DeleteSeat.LeastAcross + 11.0f, 5.0f, Sheet.Danger);
    Surface.TextRun(DeleteSeat.LeastAlong + 26.0f, CentredAcross(DeleteSeat, Surface.RunExtent(10.0f)), "Delete", Sheet.Danger, 10.0f);
}

}   // namespace Slate
