//============================================================================================================================================
//                                                          OUTLINERPANEL.CPP
//============================================================================================================================================
// 🧩 The scene directory tree, presented row by row from borrowed declarations — head, filter, forest, count foot.

#include "SlateUI/Interface/OutlinerPanel/Api/OutlinerPanel.h"
#include "SlateUI/Interface/ControlPanel/Api/ControlPanel.h"

#include "imgui.h"

#include <cctype>
#include <cstdio>
#include <cstring>

namespace Slate
{

//------------------------------------------------------------------------------------------------------------------------
//                                                     CLASSIFICATIONS
//------------------------------------------------------------------------------------------------------------------------

InkOrdinate ClassificationTint(DirectoryClassification Classification)
{
    switch (Classification)
    {
        case DirectoryClassification::Scene:     return Covering(0x7EC8FFu);
        case DirectoryClassification::Enclosure: return Covering(0xB98BFFu);
        case DirectoryClassification::Sketch:    return Covering(0x37D6D6u);
        case DirectoryClassification::Solid:     return Covering(0xFFB24Du);
        case DirectoryClassification::Cylinder:  return Covering(0x4FD18Bu);
        case DirectoryClassification::Sphere:    return Covering(0xFF7AB8u);
        case DirectoryClassification::Cone:      return Covering(0xFF6B6Bu);
        case DirectoryClassification::Revolve:   return Covering(0xC99B6Au);
        case DirectoryClassification::Loft:      return Covering(0x5B8CFFu);
        case DirectoryClassification::ClassificationCount: break;
    }
    return Covering(0x8A8A99u);
}

const char* ClassificationLabel(DirectoryClassification Classification)
{
    switch (Classification)
    {
        case DirectoryClassification::Scene:     return "Part";
        case DirectoryClassification::Enclosure: return "Body";
        case DirectoryClassification::Sketch:    return "Sketch";
        case DirectoryClassification::Solid:     return "Solid";
        case DirectoryClassification::Cylinder:  return "Cylinder";
        case DirectoryClassification::Sphere:    return "Sphere";
        case DirectoryClassification::Cone:      return "Cone";
        case DirectoryClassification::Revolve:   return "Revolve";
        case DirectoryClassification::Loft:      return "Loft";
        case DirectoryClassification::ClassificationCount: break;
    }
    return "Solid";
}

const char* ClassificationAbbr(DirectoryClassification Classification)
{
    switch (Classification)
    {
        case DirectoryClassification::Scene:     return "PT";
        case DirectoryClassification::Enclosure: return "GR";
        case DirectoryClassification::Sketch:    return "SK";
        case DirectoryClassification::Solid:     return "SO";
        case DirectoryClassification::Cylinder:  return "CY";
        case DirectoryClassification::Sphere:    return "SP";
        case DirectoryClassification::Cone:      return "CO";
        case DirectoryClassification::Revolve:   return "RV";
        case DirectoryClassification::Loft:      return "LO";
        case DirectoryClassification::ClassificationCount: break;
    }
    return "OB";
}

//------------------------------------------------------------------------------------------------------------------------
//                                                     SELECTION
//------------------------------------------------------------------------------------------------------------------------

bool OutlinerPanel::TokenTaken(const char* Identity) const
{
    for (std::uint32_t Ordinal = 0u; Ordinal < TakenCount; ++Ordinal)
        if (std::strcmp(TakenIdentities[Ordinal], Identity) == 0)
            return true;
    return false;
}

void OutlinerPanel::SeatTaken(const char* Identity)
{
    TakenCount = 1u;
    std::snprintf(TakenIdentities[0], sizeof TakenIdentities[0], "%s", Identity);
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
void LowercaseRun(char* Seat, std::uint32_t Capacity, const char* Run)
{
    std::uint32_t Ordinal = 0u;
    while (Run[Ordinal] != '\0' && Ordinal < Capacity - 1u)
    {
        Seat[Ordinal] = static_cast<char>(std::tolower(static_cast<unsigned char>(Run[Ordinal])));
        ++Ordinal;
    }
    Seat[Ordinal] = '\0';
}

}   // namespace

//------------------------------------------------------------------------------------------------------------------------
//                                                        THE PANEL
//------------------------------------------------------------------------------------------------------------------------

bool OutlinerPanel::Retained(const OutlinerRowDeclaration& Row) const
{
    if (RetentionRun[0] == '\0')
        return true;

    char LoweredRow[64];
    char LoweredRun[64];
    LowercaseRun(LoweredRow, 64u, Row.Caption);
    LowercaseRun(LoweredRun, 64u, RetentionRun);
    if (std::strstr(LoweredRow, LoweredRun) != nullptr)
        return true;

    for (std::uint32_t Ordinal = 0u; Ordinal < Row.EnclosureCount; ++Ordinal)
        if (Retained(Row.Enclosed[Ordinal]))
            return true;

    return false;
}

void OutlinerPanel::Advance(RecordingSurface& Surface, const PlaneExtent& Seat,
                            const OutlinerRowDeclaration* Rows, std::uint32_t RowCount,
                            const OutlinerComposition& Composition, const IconDepot& ArrivingDepot)
{
    WorkspaceInk Sheet;
    Depot = &ArrivingDepot;
    InspectRaised = false;

    Surface.Ground(Seat, Sheet.StandingGround, 0.0f);

    // ① Head — black tile, scene glyph in white, caption pair, selection pill.
    const PlaneExtent Head = Spanning(Seat.LeastAlong, Seat.LeastAcross, Seat.SpanAlong(), 46.0f);
    Surface.Ground(Head, Sheet.StandingGround, 0.0f);
    Surface.Rule(Head.LeastAlong, Head.MostAcross - 1.0f, Head.SpanAlong(), 1.0f, Sheet.HairEdge);

    const PlaneExtent Tile = Spanning(Head.LeastAlong + 10.0f, CentredAcross(Head, 24.0f), 24.0f, 24.0f);
    Surface.Ground(Tile, Covering(0x000000u), 6.0f);
    Depot->PresentGlyph(Surface, Tile.Inset(4.0f, 4.0f), InkOrdinate{ 255u, 255u, 255u, 255u });

    Surface.TextRun(Tile.MostAlong + 10.0f, Head.LeastAcross + 8.0f, Composition.TitleRun, Sheet.InkPrimary, 12.5f);
    Surface.TextRun(Tile.MostAlong + 10.0f, Head.LeastAcross + 25.0f, Composition.ContextRun, Sheet.InkFaint, 10.0f);

    char PillRun[16];
    if (TakenCount > 1u) std::snprintf(PillRun, sizeof PillRun, "%u sel", TakenCount);
    else                 std::snprintf(PillRun, sizeof PillRun, "0");
    const float PillSize = 10.0f;
    const float PillExtent = Surface.MeasureRun(PillRun, PillSize) + 20.0f;
    const PlaneExtent Pill = Spanning(Head.MostAlong - PillExtent - 10.0f, CentredAcross(Head, 24.0f), PillExtent, 24.0f);
    Surface.Ground(Pill, Sheet.SunkenGround, 12.0f);
    Surface.TextRun(Surface.CentredAlong(Pill, PillRun, PillSize), CentredAcross(Pill, Surface.RunExtent(PillSize)),
                    PillRun, Sheet.InkMuted, PillSize);

    // ② Retention field.
    const PlaneExtent FieldSeat = Spanning(Seat.LeastAlong + 8.0f, Head.MostAcross + 8.0f, Seat.SpanAlong() - 16.0f, 30.0f);
    const bool Focused = PresentRetentionField(Surface, FieldSeat, RetentionRun, 64u, "Filter...",
                                               Sheet.SunkenGround, Sheet.HairEdge, Sheet.InkPrimary, Sheet.InkFaint);
    if (Focused)
        Surface.Edge(FieldSeat, Sheet.FieldOutline, 1.0f, 6.0f);

    // ③ Body — the disclosure forest, scrolled by wheel.
    const PlaneExtent Body = Spanning(Seat.LeastAlong + 8.0f, FieldSeat.MostAcross + 4.0f, Seat.SpanAlong() - 16.0f,
                                      Seat.MostAcross - 26.0f - (FieldSeat.MostAcross + 4.0f));
    if (Surface.PointerWithin(Body))
        ScrollAcross -= ImGui::GetIO().MouseWheel * 32.0f;

    const bool RetentionStanding = RetentionRun[0] != '\0';
    PresentedCount = 0u;
    for (std::uint32_t Ordinal = 0u; Ordinal < RowCount; ++Ordinal)
        PresentRow(Surface, Body, Rows[Ordinal], 0u, RetentionStanding, ArrivingDepot);

    const float ContentExtent = static_cast<float>(PresentedCount) * 32.0f;
    const float Ceiling = ContentExtent > Body.SpanAcross() ? ContentExtent - Body.SpanAcross() : 0.0f;
    if (ScrollAcross < 0.0f)    ScrollAcross = 0.0f;
    if (ScrollAcross > Ceiling) ScrollAcross = Ceiling;

    // ④ Foot — the seated counts, verbatim from the reference's directory foot.
    const PlaneExtent Foot = Spanning(Seat.LeastAlong, Seat.MostAcross - 26.0f, Seat.SpanAlong(), 26.0f);
    Surface.Ground(Foot, Sheet.SunkenGround, 0.0f);
    Surface.Rule(Foot.LeastAlong, Foot.LeastAcross, Foot.SpanAlong(), 1.0f, Sheet.HairEdge);

    const float FootAcross = CentredAcross(Foot, Surface.RunExtent(10.0f));
    float FootAlong = Foot.LeastAlong + 10.0f;
    const char* const FootFigures[2] = { "1", "4" };
    const char* const FootRuns[2]    = { "groups", "bodies" };
    for (std::uint32_t Ordinal = 0u; Ordinal < 2u; ++Ordinal)
    {
        Surface.TextRun(FootAlong, FootAcross, FootFigures[Ordinal], Sheet.InkPrimary, 10.0f);
        FootAlong += Surface.MeasureRun(FootFigures[Ordinal], 10.0f) + 5.0f;
        Surface.TextRun(FootAlong, FootAcross, FootRuns[Ordinal], Sheet.InkMuted, 10.0f);
        FootAlong += Surface.MeasureRun(FootRuns[Ordinal], 10.0f) + 7.0f;
        if (Ordinal == 0u)
        {
            Surface.Medallion(FootAlong - 3.0f, Foot.LeastAcross + 13.0f, 1.0f, Sheet.FieldUnit);
        }
    }
}

void OutlinerPanel::PresentRow(RecordingSurface& Surface, const PlaneExtent& Body, const OutlinerRowDeclaration& Row,
                               std::uint32_t Depth, bool RetentionStanding, const IconDepot& RowDepot)
{
    if (RetentionStanding && !Retained(Row))
        return;

    WorkspaceInk Sheet;
    const float RowExtent   = 32.0f;
    const float IndentAlong = 8.0f + Depth * 15.0f;
    const float RowAcross   = Body.LeastAcross + static_cast<float>(PresentedCount) * RowExtent - ScrollAcross;
    ++PresentedCount;

    if (RowAcross + RowExtent <= Body.LeastAcross || RowAcross >= Body.MostAcross)
        return;   // 📝 fully outside the body — nothing to present this tick

    const bool Branch = Row.EnclosureCount > 0u;
    const bool Open   = Branch && (Row.Expanded == nullptr || *Row.Expanded || RetentionStanding);
    const bool Taken  = TokenTaken(Row.Identity);
    const double Dimming = Row.Hidden != nullptr && *Row.Hidden ? 0.5 : 1.0;

    const PlaneExtent RowSeat = Spanning(Body.LeastAlong + IndentAlong, RowAcross, Body.SpanAlong() - IndentAlong - 7.0f, RowExtent);

    // ① Row interaction — one seat for take (additive under control) and inspect.
    ImGui::PushID(Row.Identity);
    ImGui::SetCursorScreenPos(ImVec2(RowSeat.LeastAlong, RowSeat.LeastAcross));
    ImGui::InvisibleButton("row", ImVec2(RowSeat.SpanAlong(), RowSeat.SpanAcross()));
    const bool RowClicked = ImGui::IsItemClicked();
    const bool RowDouble  = ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0);
    const bool RowRoused  = ImGui::IsItemHovered();

    if (Taken)
        Surface.Ground(RowSeat, Sheet.RailTaken, 6.0f);
    else if (RowRoused)
        Surface.Ground(RowSeat, Sheet.RowRoused, 6.0f);

    // ② Disclosure chevron.
    float Cursor = RowSeat.LeastAlong;
    bool TwistTaken = false;
    if (Branch)
    {
        const bool TwistRoused = ImGui::IsItemHovered() && Surface.PointerWithin(Spanning(RowSeat.LeastAlong - 4.0f, RowAcross, 24.0f, RowExtent));
        if (TwistRoused && ImGui::IsMouseClicked(0) && Row.Expanded != nullptr)
        {
            *Row.Expanded = !*Row.Expanded;
            TwistTaken = true;   // 📝 disclosure swallows the press; the row is not taken with it
        }
        Surface.Chevron(Cursor + 7.0f, RowAcross + RowExtent * 0.5f, 4.0f,
                        TwistRoused ? Sheet.InkPrimary : Sheet.InkFaint, Open);
    }
    Cursor += 15.0f + 8.0f;

    // ③ Classification seat — the dummy glyph, tinted by the classification hue.
    const InkOrdinate Tint = ClassificationTint(Row.Classification);
    const PlaneExtent GlyphSeat = Spanning(Cursor, CentredAcross(RowSeat, 18.0f), 18.0f, 18.0f);
    RowDepot.PresentGlyph(Surface, GlyphSeat, Translucent(Tint, Dimming));
    Cursor += 18.0f + 8.0f;

    // ④ Caption, enclosure count.
    const PlaneExtent EyeSeat = Spanning(RowSeat.MostAlong - 22.0f, CentredAcross(RowSeat, 20.0f), 20.0f, 20.0f);
    char CountFigures[12];
    std::snprintf(CountFigures, sizeof CountFigures, "%u", Row.EnclosureCount);
    const float CountExtent = Branch ? Surface.MeasureRun(CountFigures, 10.0f) + 8.0f : 0.0f;
    InkOrdinate CaptionInk = Taken ? Sheet.InkPrimary : Sheet.InkMuted;
    if (RowRoused && !Taken)
        CaptionInk = Sheet.InkPrimary;
    Surface.TextRunClipped(Cursor, CentredAcross(RowSeat, Surface.RunExtent(12.5f)), Row.Caption,
                           Translucent(CaptionInk, Dimming), 12.5f, EyeSeat.LeastAlong - Cursor - CountExtent - 4.0f);
    if (Branch)
        Surface.TextRun(EyeSeat.LeastAlong - CountExtent + 4.0f, CentredAcross(RowSeat, Surface.RunExtent(10.0f)),
                        CountFigures, Translucent(Sheet.InkFaint, Dimming), 10.0f);

    // ⑤ Presence eye — presented when hidden or roused.
    if (Row.Hidden != nullptr && (RowRoused || *Row.Hidden))
    {
        const bool EyeRoused = Surface.PointerWithin(EyeSeat);
        if (EyeRoused && ImGui::IsMouseClicked(0))
        {
            const bool Vacate = !(*Row.Hidden);
            struct Cascade { static void Apply(const OutlinerRowDeclaration& Target, bool Vacate) {
                if (Target.Hidden != nullptr) *Target.Hidden = Vacate;
                for (std::uint32_t Inner = 0u; Inner < Target.EnclosureCount; ++Inner)
                    Apply(Target.Enclosed[Inner], Vacate);
            } };
            Cascade::Apply(Row, Vacate);
        }
        Surface.EyeGlyph(EyeSeat.LeastAlong + 10.0f, EyeSeat.LeastAcross + 10.0f, 7.0f,
                         Translucent(EyeRoused ? Sheet.InkPrimary : Sheet.InkFaint, Dimming),
                         *Row.Hidden);
    }

    if (RowClicked && !TwistTaken)
    {
        // ①① Control gestures toggle the token additively; a plain press seats exactly one.
        const bool ControlHeld = ImGui::GetIO().KeyCtrl;
        bool Already = false;
        std::uint32_t AlreadyOrdinal = 0u;
        for (std::uint32_t Ordinal = 0u; Ordinal < TakenCount; ++Ordinal)
            if (std::strcmp(TakenIdentities[Ordinal], Row.Identity) == 0)
            {
                Already = true;
                AlreadyOrdinal = Ordinal;
                break;
            }
        if (ControlHeld && Already)
        {
            for (std::uint32_t Ordinal = AlreadyOrdinal; Ordinal + 1u < TakenCount; ++Ordinal)
                std::snprintf(TakenIdentities[Ordinal], sizeof TakenIdentities[Ordinal], "%s", TakenIdentities[Ordinal + 1u]);
            --TakenCount;
        }
        else if (ControlHeld && TakenCount < SelectionCapacity)
        {
            std::snprintf(TakenIdentities[TakenCount], sizeof TakenIdentities[TakenCount], "%s", Row.Identity);
            ++TakenCount;
        }
        else
        {
            SeatTaken(Row.Identity);
        }
    }
    if (RowDouble && !TwistTaken)
    {
        SeatTaken(Row.Identity);
        InspectRaised = true;
    }
    ImGui::PopID();

    // ⑥ The taken rail — the accent, at the row's leading edge.
    if (Taken)
        Surface.Ground(Spanning(RowSeat.LeastAlong + 1.0f, RowAcross + RowExtent * 0.5f - 7.5f, 3.0f, 15.0f), Sheet.Accent, 2.0f);

    if (Branch && Open)
        for (std::uint32_t Inner = 0u; Inner < Row.EnclosureCount; ++Inner)
            PresentRow(Surface, Body, Row.Enclosed[Inner], Depth + 1u, RetentionStanding, RowDepot);
}

}   // namespace Slate
