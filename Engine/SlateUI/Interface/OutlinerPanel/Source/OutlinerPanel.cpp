//============================================================================================================================================
//                                                          OUTLINERPANEL.CPP
//============================================================================================================================================
// 🧩 The world outliner tree and the entry inspector, presented row by row from borrowed declarations.

#include "Engine/SlateUI/Interface/OutlinerPanel/Api/OutlinerPanel.h"
#include "Engine/SlateUI/Interface/ControlPanel/Api/ControlPanel.h"

#include "imgui.h"

#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>

namespace Slate
{

//------------------------------------------------------------------------------------------------------------------------
//                                                     CLASSIFICATIONS
//------------------------------------------------------------------------------------------------------------------------

InkOrdinate ClassificationTint(OutlinerClassification Classification)
{
    switch (Classification)
    {
        case OutlinerClassification::Level:      return Covering(0xEAB308u);
        case OutlinerClassification::Enclosure:  return Covering(0x8A8A8Au);
        case OutlinerClassification::Actor:      return Covering(0x3B82F6u);
        case OutlinerClassification::Camera:     return Covering(0xEC4899u);
        case OutlinerClassification::Light:      return Covering(0xF59E0Bu);
        case OutlinerClassification::Audio:      return Covering(0x8B5CF6u);
        case OutlinerClassification::Particle:   return Covering(0x10B981u);
        case OutlinerClassification::Trigger:    return Covering(0xEF4444u);
        case OutlinerClassification::Script:     return Covering(0x06B6D4u);
        case OutlinerClassification::ClassificationCount: break;
    }
    return Covering(0x8A8A8Au);
}

const char* ClassificationRun(OutlinerClassification Classification)
{
    switch (Classification)
    {
        case OutlinerClassification::Level:      return "level";
        case OutlinerClassification::Enclosure:  return "enclosure";
        case OutlinerClassification::Actor:      return "actor";
        case OutlinerClassification::Camera:     return "camera";
        case OutlinerClassification::Light:      return "light";
        case OutlinerClassification::Audio:      return "audio";
        case OutlinerClassification::Particle:   return "particle";
        case OutlinerClassification::Trigger:    return "trigger";
        case OutlinerClassification::Script:     return "script";
        case OutlinerClassification::ClassificationCount: break;
    }
    return "actor";
}

namespace
{

/// 🧩 Scales an ink's coverage by the declared fraction, the reference's opacity dimming.
/// cost  ✔️
InkOrdinate Translucent(const InkOrdinate& Ink, double Fraction)
{
    InkOrdinate Dimmed = Ink;
    Dimmed.Opacity = static_cast<std::uint8_t>(Ink.Opacity * Fraction + 0.5);
    return Dimmed;
}

/// 🧩 Lowercases one run in place, bounded.
/// cost  ✔️
void LowercaseRun(char (&Seat)[64], const char* Run)
{
    std::uint32_t Ordinal = 0u;
    while (Run[Ordinal] != '\0' && Ordinal < 63u)
    {
        Seat[Ordinal] = static_cast<char>(std::tolower(static_cast<unsigned char>(Run[Ordinal])));
        ++Ordinal;
    }
    Seat[Ordinal] = '\0';
}

}   // namespace

//------------------------------------------------------------------------------------------------------------------------
//                                                      CONSTRUCTION
//------------------------------------------------------------------------------------------------------------------------

Deliver<bool> OutlinerPanel::Construct(const IconDepot& ArrivingDepot)
{
    Depot = &ArrivingDepot;
    return Deliver<bool>::Delivered(true);
}

bool OutlinerPanel::Retained(const OutlinerRowDeclaration& Row) const
{
    if (RetentionRun[0] == '\0')
        return true;

    char LoweredRow[64];
    char LoweredRun[64];
    LowercaseRun(LoweredRow, Row.Caption);
    LowercaseRun(LoweredRun, RetentionRun);
    if (std::strstr(LoweredRow, LoweredRun) != nullptr)
        return true;

    for (std::uint32_t Ordinal = 0u; Ordinal < Row.EnclosureCount; ++Ordinal)
        if (Retained(Row.Enclosed[Ordinal]))
            return true;

    return false;
}

std::uint32_t OutlinerPanel::CountEnclosed(const OutlinerRowDeclaration* Rows, std::uint32_t RowCount) const
{
    std::uint32_t Sum = RowCount;
    for (std::uint32_t Ordinal = 0u; Ordinal < RowCount; ++Ordinal)
        Sum += CountEnclosed(Rows[Ordinal].Enclosed, Rows[Ordinal].EnclosureCount);
    return Sum;
}

//------------------------------------------------------------------------------------------------------------------------
//                                                        THE PANEL
//------------------------------------------------------------------------------------------------------------------------

void OutlinerPanel::Advance(RecordingSurface& Surface, const PlaneExtent& Seat,
                            const OutlinerRowDeclaration* Rows, std::uint32_t RowCount,
                            const OutlinerComposition& Composition)
{
    WorkspaceInk Sheet;
    InspectRaised = false;

    Surface.Ground(Seat, Sheet.StandingGround, 0.0f);

    // ① Head — accent tile, dummy glyph, title and context runs.
    const PlaneExtent Head = Spanning(Seat.LeastAlong, Seat.LeastAcross, Seat.SpanAlong(), 46.0f);
    Surface.Ground(Head, Sheet.StandingGround, 0.0f);
    Surface.Rule(Head.LeastAlong, Head.MostAcross - 1.0f, Head.SpanAlong(), 1.0f, Sheet.HairEdge);

    const PlaneExtent Tile = Spanning(Head.LeastAlong + 10.0f, CentredAcross(Head, 24.0f), 24.0f, 24.0f);
    Surface.Ground(Tile, Sheet.AccentTile, 6.0f);
    if (Depot != nullptr)
        Depot->PresentGlyph(Surface, Tile.Inset(5.0f, 5.0f), InkOrdinate{ 255u, 255u, 255u, 255u });

    Surface.TextRun(Tile.MostAlong + 10.0f, Head.LeastAcross + 9.0f, Composition.TitleRun, Sheet.InkPrimary, 12.5f);
    Surface.TextRun(Tile.MostAlong + 10.0f, Head.LeastAcross + 26.0f, Composition.ContextRun, Sheet.InkFaint, 10.0f);

    // ② Retention field.
    const PlaneExtent FieldSeat = Spanning(Seat.LeastAlong + 8.0f, Head.MostAcross + 8.0f, Seat.SpanAlong() - 16.0f, 30.0f);
    const bool Focused = PresentRetentionField(Surface, FieldSeat, RetentionRun, 64u, "Filter Entities...",
                                               Sheet.SunkenGround, Sheet.HairEdge, Sheet.InkPrimary, Sheet.InkFaint);
    if (Focused)
        Surface.Edge(FieldSeat, Sheet.FieldOutline, 1.0f, 6.0f);

    // ③ Body — the disclosure forest, scrolled by wheel.
    const PlaneExtent Body = Spanning(Seat.LeastAlong + 8.0f, FieldSeat.MostAcross + 4.0f, Seat.SpanAlong() - 16.0f,
                                      Seat.MostAcross - 26.0f - (FieldSeat.MostAcross + 4.0f));
    const bool RetentionStanding = RetentionRun[0] != '\0';
    const std::uint32_t Total = CountEnclosed(Rows, RowCount);

    // ①① The wheel scrolls the body; the clamp lands after the forest has been counted.
    if (Surface.PointerWithin(Body))
        ScrollAcross -= ImGui::GetIO().MouseWheel * 32.0f;

    PresentedCount = 0u;
    for (std::uint32_t Ordinal = 0u; Ordinal < RowCount; ++Ordinal)
        PresentRow(Surface, Body, Rows[Ordinal], 0u, RetentionStanding);

    const float ContentExtent = static_cast<float>(PresentedCount) * 32.0f;
    const float Ceiling = ContentExtent > Body.SpanAcross() ? ContentExtent - Body.SpanAcross() : 0.0f;
    if (ScrollAcross < 0.0f)                 ScrollAcross = 0.0f;
    if (ScrollAcross > Ceiling)              ScrollAcross = Ceiling;

    // ④ Foot — the counted entities run.
    const PlaneExtent Foot = Spanning(Seat.LeastAlong, Seat.MostAcross - 26.0f, Seat.SpanAlong(), 26.0f);
    Surface.Ground(Foot, Sheet.SunkenGround, 0.0f);
    Surface.Rule(Foot.LeastAlong, Foot.LeastAcross, Foot.SpanAlong(), 1.0f, Sheet.HairEdge);

    // ①① The figure run in primary ink, the trailing run muted — the reference's split.
    char FigureRun[16];
    std::snprintf(FigureRun, sizeof FigureRun, "%u", Total);
    const float FigureExtent = Surface.MeasureRun(FigureRun, 10.0f);
    const float FootAcross = CentredAcross(Foot, Surface.RunExtent(10.0f));
    Surface.TextRun(Foot.LeastAlong + 10.0f, FootAcross, FigureRun, Sheet.InkPrimary, 10.0f);
    Surface.TextRun(Foot.LeastAlong + 10.0f + FigureExtent + 5.0f, FootAcross, "entities", Sheet.InkMuted, 10.0f);
}

void OutlinerPanel::PresentRow(RecordingSurface& Surface, const PlaneExtent& Body, const OutlinerRowDeclaration& Row,
                               std::uint32_t Depth, bool RetentionStanding)
{
    if (RetentionStanding && !Retained(Row))
        return;

    WorkspaceInk Sheet;
    const float RowExtent  = 32.0f;
    const float IndentAlong = 8.0f + Depth * 15.0f;
    float RowAcross = Body.LeastAcross + static_cast<float>(PresentedCount) * RowExtent - ScrollAcross;
    ++PresentedCount;

    if (RowAcross < Body.LeastAcross - RowExtent || RowAcross + RowExtent > Body.MostAcross + RowExtent)
        return;   // 📝 fully outside the body — nothing to present this tick

    const bool Branch = Row.EnclosureCount > 0u;
    const bool Open   = Branch && (Row.Expanded == nullptr || *Row.Expanded || RetentionStanding);
    const bool Taken  = std::strcmp(TakenIdentity, Row.Identity) == 0;
    const bool Roused = Surface.PointerWithin(Spanning(Body.LeastAlong + IndentAlong, RowAcross, Body.SpanAlong() - IndentAlong - 7.0f, RowExtent));

    const PlaneExtent RowSeat = Spanning(Body.LeastAlong + IndentAlong, RowAcross, Body.SpanAlong() - IndentAlong - 7.0f, RowExtent);

    // ① Row interaction — one seat for take and inspect.
    ImGui::PushID(Row.Identity);
    ImGui::SetCursorScreenPos(ImVec2(RowSeat.LeastAlong, RowSeat.LeastAcross));
    ImGui::InvisibleButton("row", ImVec2(RowSeat.SpanAlong(), RowSeat.SpanAcross()));
    const bool RowClicked = ImGui::IsItemClicked();
    const bool RowDouble  = ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0);
    const bool RowRoused  = ImGui::IsItemHovered();

    const double Dimming = Row.Hidden != nullptr && *Row.Hidden ? 0.5 : 1.0;

    if (Taken)
    {
        Surface.Ground(RowSeat, Sheet.RowTakenGround, 6.0f);
        Surface.Ground(Spanning(RowSeat.LeastAlong + 1.0f, RowAcross + RowExtent * 0.5f - 7.5f, 3.0f, 15.0f), Sheet.RowTakenRail, 2.0f);
    }
    else if (RowRoused)
    {
        Surface.Ground(RowSeat, Sheet.RowRoused, 6.0f);
    }

    // ② Disclosure chevron.
    float Cursor = RowSeat.LeastAlong;
    const PlaneExtent TwistSeat = Spanning(Cursor, RowAcross, 15.0f, RowExtent);
    bool TwistTaken = false;
    if (Branch)
    {
        const bool TwistRoused = ImGui::IsItemHovered() && Surface.PointerWithin(Spanning(TwistSeat.LeastAlong - 4.0f, TwistSeat.LeastAcross, 22.0f, RowExtent));
        if (TwistRoused && ImGui::IsMouseClicked(0) && Row.Expanded != nullptr)
        {
            *Row.Expanded = !*Row.Expanded;
            TwistTaken = true;   // 📝 disclosure swallows the press; the row is not taken with it
        }
        Surface.Chevron(TwistSeat.LeastAlong + 7.0f, RowAcross + RowExtent * 0.5f, 4.0f,
                        TwistRoused ? Sheet.InkPrimary : Sheet.InkFaint, Open);
    }
    Cursor += 15.0f;

    // ③ Classification seat — the dummy glyph, tinted.
    const InkOrdinate Tint = ClassificationTint(Row.Classification);
    const PlaneExtent GlyphSeat = Spanning(Cursor, CentredAcross(RowSeat, 18.0f), 18.0f, 18.0f);
    if (Depot != nullptr)
        Depot->PresentGlyph(Surface, GlyphSeat, Translucent(Tint, Dimming));
    Cursor += 18.0f + 6.0f;

    // ④ Caption, enclosure count.
    const PlaneExtent EyeSeat = Spanning(RowSeat.MostAlong - 22.0f, CentredAcross(RowSeat, 20.0f), 20.0f, 20.0f);
    char CountFigures[12];
    std::snprintf(CountFigures, sizeof CountFigures, "%u", Row.EnclosureCount);
    const float CountExtent = Branch ? Surface.MeasureRun(CountFigures, 10.0f) + 8.0f : 0.0f;
    const float CaptionExtent = EyeSeat.LeastAlong - Cursor - CountExtent - 4.0f;

    InkOrdinate CaptionInk = Taken ? Sheet.InkPrimary : Sheet.InkMuted;
    if (RowRoused && !Taken)
        CaptionInk = Sheet.InkPrimary;
    Surface.TextRunClipped(Cursor, CentredAcross(RowSeat, Surface.RunExtent(12.5f)), Row.Caption,
                           Translucent(CaptionInk, Dimming), 12.5f, CaptionExtent);
    if (Branch)
        Surface.TextRun(EyeSeat.LeastAlong - CountExtent + 4.0f, CentredAcross(RowSeat, Surface.RunExtent(10.0f)),
                        CountFigures, Translucent(Sheet.InkFaint, Dimming), 10.0f);

    // ⑤ Presence eye — presented when hidden or roused.
    const bool EyeRelevant = Row.Hidden != nullptr && (RowRoused || (*Row.Hidden));
    if (EyeRelevant)
    {
        const bool EyeRoused = Surface.PointerWithin(EyeSeat);
        if (EyeRoused && ImGui::IsMouseClicked(0))
        {
            const bool Vacate = !(*Row.Hidden);
            *Row.Hidden = Vacate;
            // ①① Presence cascades through everything the row encloses.
            struct Cascade { static void Apply(const OutlinerRowDeclaration& Target, bool Vacate) {
                if (Target.Hidden != nullptr) *Target.Hidden = Vacate;
                for (std::uint32_t Inner = 0u; Inner < Target.EnclosureCount; ++Inner)
                    Apply(Target.Enclosed[Inner], Vacate);
            } };
            Cascade::Apply(Row, Vacate);
        }
        Surface.EyeGlyph(EyeSeat.LeastAlong + 10.0f, EyeSeat.LeastAcross + 10.0f, 7.0f,
                         Translucent(EyeRoused ? Sheet.InkPrimary : Sheet.InkFaint, Dimming),
                         Row.Hidden != nullptr && *Row.Hidden);
    }

    if (RowClicked && !TwistTaken)
        std::snprintf(TakenIdentity, sizeof TakenIdentity, "%s", Row.Identity);
    if (RowDouble)
    {
        std::snprintf(TakenIdentity, sizeof TakenIdentity, "%s", Row.Identity);
        InspectRaised = true;
    }
    ImGui::PopID();

    if (Branch && Open)
        for (std::uint32_t Inner = 0u; Inner < Row.EnclosureCount; ++Inner)
            PresentRow(Surface, Body, Row.Enclosed[Inner], Depth + 1u, RetentionStanding);
}

//------------------------------------------------------------------------------------------------------------------------
//                                                   THE ENTRY INSPECTOR
//------------------------------------------------------------------------------------------------------------------------

void EntryInspectorPanel::Advance(RecordingSurface& Surface, const PlaneExtent& Seat,
                                  const OutlinerRowDeclaration* Declared, EntryOrdinates& Ordinates,
                                  const IconDepot& Depot)
{
    WorkspaceInk Sheet;
    const ControlSheet Controls = ControlSheetFromWorkspace(Sheet);
    BackRaised = false;

    Surface.Ground(Seat, Sheet.SunkenGround, 0.0f);

    // ① Head.
    const PlaneExtent Head = Spanning(Seat.LeastAlong, Seat.LeastAcross, Seat.SpanAlong(), 46.0f);
    Surface.Rule(Head.LeastAlong, Head.MostAcross - 1.0f, Head.SpanAlong(), 1.0f, Sheet.HairEdge);

    const PlaneExtent Tile = Spanning(Head.LeastAlong + 10.0f, CentredAcross(Head, 24.0f), 24.0f, 24.0f);
    if (Declared == nullptr)
    {
        Surface.Ground(Tile, Covering(0x111111u), 6.0f);
        Depot.PresentGlyph(Surface, Tile.Inset(5.0f, 5.0f), Sheet.InkFaint);
        Surface.TextRun(Tile.MostAlong + 10.0f, CentredAcross(Head, Surface.RunExtent(12.5f)), "Nothing selected", Sheet.InkPrimary, 12.5f);
    }
    else
    {
        const InkOrdinate Tint = ClassificationTint(Declared->Classification);
        Surface.Ground(Tile, Covering(0x111111u), 6.0f);
        Surface.Edge(Tile, Covering(0x222222u), 1.0f, 6.0f);
        Depot.PresentGlyph(Surface, Tile.Inset(5.0f, 5.0f), Tint);
        Surface.TextRun(Tile.MostAlong + 10.0f, Head.LeastAcross + 8.0f, Declared->Caption, Sheet.InkPrimary, 12.5f);
        char SubRun[48];
        std::snprintf(SubRun, sizeof SubRun, "%s entity", ClassificationRun(Declared->Classification));
        Surface.TextRun(Tile.MostAlong + 10.0f, Head.LeastAcross + 25.0f, SubRun, Tint, 10.0f);
    }

    // ② Back action.
    const PlaneExtent BackSeat = Spanning(Head.MostAlong - 74.0f, CentredAcross(Head, 28.0f), 64.0f, 28.0f);
    ImGui::PushID("inspector.back");
    ImGui::SetCursorScreenPos(ImVec2(BackSeat.LeastAlong, BackSeat.LeastAcross));
    ImGui::InvisibleButton("back", ImVec2(BackSeat.SpanAlong(), BackSeat.SpanAcross()));
    const bool BackRoused = ImGui::IsItemHovered();
    if (ImGui::IsItemClicked())
        BackRaised = true;
    ImGui::PopID();
    if (BackRoused)
        Surface.Ground(BackSeat, Sheet.TileRoused, 6.0f);
    Surface.Chevron(BackSeat.LeastAlong + 12.0f, BackSeat.LeastAcross + 14.0f, 4.0f, Sheet.InkMuted, false);
    Surface.TextRun(BackSeat.LeastAlong + 20.0f, CentredAcross(BackSeat, Surface.RunExtent(11.0f)), "Back", Sheet.InkMuted, 11.0f);

    if (Declared == nullptr)
    {
        Surface.TextRun(Surface.CentredAlong(Seat, "Select an entity to inspect its components.", 11.5f),
                        Seat.LeastAcross + 100.0f, "Select an entity to inspect its components.", Sheet.InkFaint, 11.5f);
        return;
    }

    // ③ Cards.
    const ControlRowDeclaration CaptionRun = { "", 88.0f, 13.5f };
    float CursorAcross = Head.MostAcross + 7.0f;
    const float CardInset = 7.0f;
    const float CardExtentAlong = Seat.SpanAlong() - CardInset * 2.0f;

    const auto PresentCardHead = [&](const PlaneExtent& Card, const char* TitleRun)
    {
        const PlaneExtent CardHead = Spanning(Card.LeastAlong, Card.LeastAcross, Card.SpanAlong(), 31.0f);
        Surface.Ground(CardHead, Sheet.SunkenGround, 12.0f, CornerSelection::UpperLeading);
        Surface.Ground(CardHead, Sheet.SunkenGround, 12.0f, CornerSelection::UpperTrailing);
        Surface.Ground(Spanning(CardHead.LeastAlong + 12.0f, CardHead.LeastAcross, CardHead.SpanAlong() - 24.0f, CardHead.SpanAcross()), Sheet.SunkenGround, 0.0f);
        Surface.Rule(CardHead.LeastAlong, CardHead.MostAcross - 1.0f, CardHead.SpanAlong(), 1.0f, Sheet.HairEdge);
        Surface.TextRun(CardHead.LeastAlong + 10.0f, CentredAcross(CardHead, Surface.RunExtent(10.5f)), TitleRun, Sheet.InkMuted, 10.5f);
    };

    const auto OpenCard = [&](float ExtentAcross, const char* TitleRun) -> PlaneExtent
    {
        const PlaneExtent Card = Spanning(Seat.LeastAlong + CardInset, CursorAcross, CardExtentAlong, ExtentAcross);
        Surface.Ground(Card, Covering(0x0A0A0Bu), 12.0f);
        Surface.Edge(Card, Sheet.HairEdge, 1.0f, 12.0f);
        PresentCardHead(Card, TitleRun);
        return Card;
    };

    const bool CarriesTransform = Declared->Classification != OutlinerClassification::Level &&
                                  Declared->Classification != OutlinerClassification::Enclosure &&
                                  Declared->Classification != OutlinerClassification::Script;

    if (CarriesTransform)
    {
        const PlaneExtent Card = OpenCard(3u * 44.0f + 20.0f + 12.0f, "TRANSFORM");
        const ControlRowDeclaration PositionRow = { "Position", 88.0f, 13.5f };
        const ControlRowDeclaration RotationRow = { "Rotation", 88.0f, 13.5f };
        const ControlRowDeclaration ScaleRow    = { "Scale",    88.0f, 13.5f };
        PlaneExtent RowSeat = Spanning(Card.LeastAlong + 10.0f, Card.LeastAcross + 31.0f + 10.0f, Card.SpanAlong() - 20.0f, 44.0f);
        PresentVectorRow(Surface, RowSeat, PositionRow, Ordinates.Position, 1.0, Controls, "inspector.position");
        RowSeat.LeastAcross += 44.0f;
        PresentVectorRow(Surface, RowSeat, RotationRow, Ordinates.Rotation, 1.0, Controls, "inspector.rotation");
        RowSeat.LeastAcross += 44.0f;
        PresentVectorRow(Surface, RowSeat, ScaleRow, Ordinates.Scale, 0.1, Controls, "inspector.scale");
        CursorAcross = Card.MostAcross + 6.0f;
    }

    // ④ The classification card, seated at the reference's declared rows.
    char CardTitle[48];
    std::snprintf(CardTitle, sizeof CardTitle, "%s COMPONENT", ClassificationRun(Declared->Classification));
    const std::uint32_t RowCount = 4u;
    const PlaneExtent Card = OpenCard(RowCount * 44.0f + 20.0f + 12.0f, CardTitle);
    float RowAcross = Card.LeastAcross + 31.0f + 10.0f;

    const auto SeatRow = [&]() -> PlaneExtent
    {
        const PlaneExtent Row = Spanning(Card.LeastAlong + 10.0f, RowAcross, Card.SpanAlong() - 20.0f, 40.0f);
        RowAcross += 44.0f;
        return Row;
    };
    (void)CaptionRun;

    static const char* const ProjectionCaptions[2] = { "Perspective", "Orthographic" };
    static const char* const StateCaptions[3] = { "Playing", "Paused", "Stopped" };
    const ControlRowDeclaration Row10 = { "", 88.0f, 13.5f };

    switch (Declared->Classification)
    {
        case OutlinerClassification::Light:
        {
            const ControlRowDeclaration IntensityRow = { "Intensity", 88.0f, 13.5f };
            const SliderDeclaration IntensityRange = { 0.0, 150000.0, 0u, "lm", 78.0f };
            double Amount = static_cast<double>(Ordinates.Intensity);
            PresentSliderRow(Surface, SeatRow(), IntensityRow, IntensityRange, Amount, Controls, "inspector.intensity");
            Ordinates.Intensity = static_cast<std::uint32_t>(Amount);

            const ControlRowDeclaration ShadowRow = { "Cast Shadows", 88.0f, 13.5f };
            PresentSwitchRow(Surface, SeatRow(), ShadowRow, Ordinates.CastShadows, Controls, "inspector.shadows");

            const PlaneExtent ColourSeat = SeatRow();
            Surface.TextRun(ColourSeat.LeastAlong, CentredAcross(ColourSeat, Surface.RunExtent(13.5f)), "Light Colour", Sheet.InkMuted, 13.5f);
            const PlaneExtent Swatch = Spanning(ColourSeat.LeastAlong + 98.0f, CentredAcross(ColourSeat, 24.0f), 100.0f, 24.0f);
            const InkOrdinate LightInk = InkOrdinate{ Ordinates.LightColour[0], Ordinates.LightColour[1], Ordinates.LightColour[2], 255u };
            Surface.Ground(Swatch, LightInk, 6.0f);
            Surface.Edge(Swatch, Sheet.HairEdge, 1.0f, 6.0f);
            break;
        }
        case OutlinerClassification::Camera:
        {
            const ControlRowDeclaration ProjectionRow = { "Projection", 88.0f, 13.5f };
            PresentDropdownRow(Surface, SeatRow(), ProjectionRow, ProjectionCaptions, 2u, Ordinates.Projection, Controls, "inspector.projection");

            const ControlRowDeclaration FovRow = { "Field of View", 88.0f, 13.5f };
            const SliderDeclaration FovRange = { 10.0, 170.0, 1u, "\xC2\xB0", 78.0f };
            PresentSliderRow(Surface, SeatRow(), FovRow, FovRange, Ordinates.FieldOfView, Controls, "inspector.fov");

            const ControlRowDeclaration NearRow = { "Near Clip", 88.0f, 13.5f };
            const SliderDeclaration NearRange = { 0.01, 10.0, 2u, "m", 78.0f };
            PresentSliderRow(Surface, SeatRow(), NearRow, NearRange, Ordinates.NearClip, Controls, "inspector.near");

            const ControlRowDeclaration FarRow = { "Far Clip", 88.0f, 13.5f };
            const SliderDeclaration FarRange = { 100.0, 50000.0, 0u, "m", 78.0f };
            PresentSliderRow(Surface, SeatRow(), FarRow, FarRange, Ordinates.FarClip, Controls, "inspector.far");
            break;
        }
        case OutlinerClassification::Audio:
        {
            const ControlRowDeclaration VolumeRow = { "Volume", 88.0f, 13.5f };
            const SliderDeclaration VolumeRange = { 0.0, 2.0, 2u, "\xC2\xB7", 78.0f };
            PresentSliderRow(Surface, SeatRow(), VolumeRow, VolumeRange, Ordinates.Volume, Controls, "inspector.volume");

            const ControlRowDeclaration LoopRow = { "Looping", 88.0f, 13.5f };
            PresentSwitchRow(Surface, SeatRow(), LoopRow, Ordinates.Looping, Controls, "inspector.loop");

            const ControlRowDeclaration SpatialRow = { "Spatial 3D", 88.0f, 13.5f };
            PresentSwitchRow(Surface, SeatRow(), SpatialRow, Ordinates.Spatial, Controls, "inspector.spatial");
            break;
        }
        case OutlinerClassification::Particle:
        {
            const ControlRowDeclaration EmitRow = { "Emit Rate", 88.0f, 13.5f };
            const SliderDeclaration EmitRange = { 1.0, 1000.0, 0u, "/s", 78.0f };
            PresentSliderRow(Surface, SeatRow(), EmitRow, EmitRange, Ordinates.EmitRate, Controls, "inspector.emit");

            const ControlRowDeclaration LifeRow = { "Life Time", 88.0f, 13.5f };
            const SliderDeclaration LifeRange = { 0.1, 20.0, 2u, "s", 78.0f };
            PresentSliderRow(Surface, SeatRow(), LifeRow, LifeRange, Ordinates.LifeTime, Controls, "inspector.life");

            const ControlRowDeclaration LoopRow = { "Looping", 88.0f, 13.5f };
            PresentSwitchRow(Surface, SeatRow(), LoopRow, Ordinates.Looping, Controls, "inspector.ploop");
            break;
        }
        case OutlinerClassification::Trigger:
        {
            const ControlRowDeclaration TagRow = { "Event Tag", 88.0f, 13.5f };
            PresentTextRow(Surface, SeatRow(), TagRow, Ordinates.EventTag, 32u, Controls, "inspector.tag");

            const ControlRowDeclaration RadiusRow = { "Radius", 88.0f, 13.5f };
            const SliderDeclaration RadiusRange = { 1.0, 500.0, 1u, "m", 78.0f };
            PresentSliderRow(Surface, SeatRow(), RadiusRow, RadiusRange, Ordinates.TriggerRadius, Controls, "inspector.radius");
            break;
        }
        case OutlinerClassification::Script:
        {
            const ControlRowDeclaration StateRow = { "State", 88.0f, 13.5f };
            PresentDropdownRow(Surface, SeatRow(), StateRow, StateCaptions, 3u, Ordinates.ScriptState, Controls, "inspector.state");

            const ControlRowDeclaration DifficultyRow = { "Difficulty", 88.0f, 13.5f };
            const SliderDeclaration DifficultyRange = { 1.0, 5.0, 0u, "\xC2\xB7", 78.0f };
            double Difficulty = static_cast<double>(Ordinates.Difficulty);
            PresentSliderRow(Surface, SeatRow(), DifficultyRow, DifficultyRange, Difficulty, Controls, "inspector.difficulty");
            Ordinates.Difficulty = static_cast<std::uint32_t>(Difficulty);
            break;
        }
        case OutlinerClassification::Actor:
        {
            const ControlRowDeclaration StaticRow = { "Static Mesh", 88.0f, 13.5f };
            PresentSwitchRow(Surface, SeatRow(), StaticRow, Ordinates.StaticMesh, Controls, "inspector.static");

            const ControlRowDeclaration PhysicsRow = { "Simulate Physics", 88.0f, 13.5f };
            PresentSwitchRow(Surface, SeatRow(), PhysicsRow, Ordinates.SimulatePhysics, Controls, "inspector.physics");

            const ControlRowDeclaration OverlapRow = { "Generate Overlaps", 88.0f, 13.5f };
            PresentSwitchRow(Surface, SeatRow(), OverlapRow, Ordinates.GenerateOverlaps, Controls, "inspector.overlap");
            break;
        }
        case OutlinerClassification::Level:
        {
            const ControlRowDeclaration NameRow = { "Level Name", 88.0f, 13.5f };
            char NameRun[32] = "Level_01_City";
            PresentTextRow(Surface, SeatRow(), NameRow, NameRun, 32u, Controls, "inspector.levelname");

            const ControlRowDeclaration PartitionRow = { "World Partition", 88.0f, 13.5f };
            PresentSwitchRow(Surface, SeatRow(), PartitionRow, Ordinates.WorldPartition, Controls, "inspector.partition");
            break;
        }
        case OutlinerClassification::Enclosure:
        {
            const ControlRowDeclaration NameRow = { "Folder Name", 88.0f, 13.5f };
            char NameRun[32] = "Folder";
            PresentTextRow(Surface, SeatRow(), NameRow, NameRun, 32u, Controls, "inspector.foldername");

            const ControlRowDeclaration EditorRow = { "Is Editor Only", 88.0f, 13.5f };
            PresentSwitchRow(Surface, SeatRow(), EditorRow, Ordinates.EditorOnly, Controls, "inspector.editoronly");
            break;
        }
        case OutlinerClassification::ClassificationCount:
            break;
    }
    (void)Row10;
}

}   // namespace Slate
