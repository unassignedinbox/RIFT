//============================================================================================================================================
//                                                            CADPANEL.CPP
//============================================================================================================================================
// 🧩 The Frontier CAD workspace — trapezoid tabs, the part header, browser dock, model stage and inspector dock.

#include "Engine/SlateUI/Interface/CadPanel/Api/CadPanel.h"
#include "Engine/SlateUI/Interface/ControlPanel/Api/ControlPanel.h"

#include "imgui.h"

#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>

namespace Slate
{

namespace
{

constexpr float TabBarAcross   = 38.0f;    // [px] - the tab strip
constexpr float HeaderAcross   = 46.0f;    // [px] - the part header
constexpr float DockAlong      = 322.0f;   // [px] - each dock column
constexpr float CarouselAcross = 36.0f;    // [px] - the dock carousel strip
constexpr float ConditionAcross = 30.0f;   // [px] - the stage condition strip

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

/// 🧩 Scales a tint's coverage — the hidden row dimming.
/// cost  ✔️
InkOrdinate TranslucentTint(const InkOrdinate& Ink, double Fraction)
{
    InkOrdinate Dimmed = Ink;
    Dimmed.Opacity = static_cast<std::uint8_t>(Ink.Opacity * Fraction + 0.5);
    return Dimmed;
}

/// 🧩 The classification tint a browser row carries.
/// cost  ✔️
InkOrdinate BrowserTint(BrowserClassification Classification)
{
    switch (Classification)
    {
        case BrowserClassification::Datums:    return Covering(0xEDEDEDu);
        case BrowserClassification::Plane:     return Covering(0x8A8A8Au);
        case BrowserClassification::Solid:     return Covering(0xEDEDEDu);
        case BrowserClassification::Surface:   return Covering(0x8A8A8Au);
        case BrowserClassification::Enclosure: break;
    }
    return Covering(0x8A8A8Au);
}

/// 🧩 The carousel — centred segments with the taken one underlined by the accent.
/// cost  ✔️
void PresentCarousel(RecordingSurface& Surface, const PlaneExtent& Seat, const char* const* Captions, std::uint32_t Count,
                     std::uint32_t& Taken, const char* PushIdentity)
{
    CadInk Sheet;
    Surface.Ground(Seat, Sheet.PanelGround, 0.0f);
    Surface.Rule(Seat.LeastAlong, Seat.MostAcross - 1.0f, Seat.SpanAlong(), 1.0f, Sheet.HairEdge);

    float CaptionExtents[3];
    float TotalExtent = 0.0f;
    for (std::uint32_t Ordinal = 0u; Ordinal < Count; ++Ordinal)
    {
        CaptionExtents[Ordinal] = Surface.MeasureRun(Captions[Ordinal], 11.5f) + 24.0f;
        TotalExtent += CaptionExtents[Ordinal] + 4.0f;
    }
    float Leading = Seat.LeastAlong + (Seat.SpanAlong() - TotalExtent + 4.0f) * 0.5f;

    for (std::uint32_t Ordinal = 0u; Ordinal < Count; ++Ordinal)
    {
        const PlaneExtent Segment = Spanning(Leading, Seat.LeastAcross + 4.0f, CaptionExtents[Ordinal], Seat.SpanAcross() - 9.0f);
        char Identity[48];
        std::snprintf(Identity, sizeof Identity, "%s.%u", PushIdentity, Ordinal);
        bool Roused = false;
        if (PresentSeat(Segment, Identity, Roused))
            Taken = Ordinal;
        Surface.TextRun(Surface.CentredAlong(Segment, Captions[Ordinal], 11.5f), CentredAcross(Segment, Surface.RunExtent(11.5f)),
                        Captions[Ordinal], Taken == Ordinal ? Sheet.InkPrimary : Sheet.InkMuted, 11.5f);
        if (Taken == Ordinal)
            Surface.Ground(Spanning(Surface.CentredAlong(Segment, Captions[Ordinal], 11.5f) - 4.0f, Seat.MostAcross - 2.0f,
                                    Surface.MeasureRun(Captions[Ordinal], 11.5f) + 8.0f, 2.0f), Sheet.Accent, 1.0f);
        Leading += CaptionExtents[Ordinal] + 4.0f;
    }
}

/// 🧩 The round icon action the header and bars seat.
/// cost  ✔️
void PresentRoundAction(RecordingSurface& Surface, const PlaneExtent& Seat, const IconDepot& Depot, const InkOrdinate& GlyphInk,
                        bool Taken, const char* PushIdentity)
{
    CadInk Sheet;
    bool Roused = false;
    PresentSeat(Seat, PushIdentity, Roused);
    if (Taken)
    {
        Surface.Medallion(Seat.LeastAlong + Seat.SpanAlong() * 0.5f, Seat.LeastAcross + Seat.SpanAcross() * 0.5f,
                          Seat.SpanAlong() * 0.5f, Sheet.CadSoft);
        Surface.Ring(Seat.LeastAlong + Seat.SpanAlong() * 0.5f, Seat.LeastAcross + Seat.SpanAcross() * 0.5f,
                     Seat.SpanAlong() * 0.5f - 1.0f, 1.0f, Partial(0xFFFFFFu, 0.35));
        Depot.PresentGlyph(Surface, Seat.Inset(8.0f, 8.0f), Sheet.InkPrimary);
    }
    else
    {
        if (Roused)
            Surface.Medallion(Seat.LeastAlong + Seat.SpanAlong() * 0.5f, Seat.LeastAcross + Seat.SpanAcross() * 0.5f,
                              Seat.SpanAlong() * 0.5f, Covering(0x1F1F1Fu));
        Depot.PresentGlyph(Surface, Seat.Inset(8.0f, 8.0f), Roused ? Sheet.InkPrimary : GlyphInk);
    }
}

}   // namespace

//------------------------------------------------------------------------------------------------------------------------
//                                                       THE WORKSPACE
//------------------------------------------------------------------------------------------------------------------------

void CadWorkspacePanel::Advance(RecordingSurface& Surface, const PlaneExtent& Seat, const BrowserRowDeclaration* Rows,
                                std::uint32_t RowCount, const CadComposition& Composition, const IconDepot& Depot)
{
    CadInk Sheet;
    Surface.Ground(Seat, Sheet.DeskGround, 0.0f);

    // ① Tab strip — one trapezoid tab, seated taken.
    const PlaneExtent TabBar = Spanning(Seat.LeastAlong, Seat.LeastAcross, Seat.SpanAlong(), TabBarAcross);
    Surface.Ground(TabBar, Sheet.TabGround, 0.0f);
    Surface.Rule(TabBar.LeastAlong, TabBar.MostAcross - 1.0f, TabBar.SpanAlong(), 1.0f, Sheet.HairEdge);

    const float TabPad = 8.0f;
    const float CaptionExtent = Surface.MeasureRun(Composition.TabCaption, 12.5f);
    const float TabExtent = CaptionExtent + TabPad * 2.0f + 8.0f + 19.0f + 14.0f;
    const PlaneExtent TabFill = Spanning(TabBar.LeastAlong + 14.0f + 20.0f, TabBar.LeastAcross, TabExtent - 40.0f, TabBarAcross);
    const PlaneExtent TabSeat  = Spanning(TabBar.LeastAlong + 14.0f, TabBar.LeastAcross, TabExtent, TabBarAcross);
    Surface.Ground(TabFill, Sheet.HeaderGround, 9.0f, CornerSelection::UpperLeading);
    Surface.Ground(TabFill, Sheet.HeaderGround, 9.0f, CornerSelection::UpperTrailing);
    // ①① The skewed shoulders — one rotated ground each.
    const PlaneExtent ShoulderLeading  = Spanning(TabSeat.LeastAlong, TabSeat.LeastAcross + 6.0f, 24.0f, TabBarAcross - 6.0f);
    const PlaneExtent ShoulderTrailing = Spanning(TabSeat.MostAlong - 24.0f, TabSeat.LeastAcross + 6.0f, 24.0f, TabBarAcross - 6.0f);
    Surface.Ground(ShoulderLeading, Sheet.HeaderGround, 8.0f, CornerSelection::UpperLeading);
    Surface.Ground(ShoulderTrailing, Sheet.HeaderGround, 8.0f, CornerSelection::UpperTrailing);
    Surface.Ground(Spanning(ShoulderLeading.LeastAlong + 8.0f, ShoulderTrailing.LeastAcross, TabExtent - 16.0f, TabBarAcross - 6.0f),
                   Sheet.HeaderGround, 0.0f);
    Surface.Medallion(TabSeat.LeastAlong + 18.0f, TabSeat.LeastAcross + TabBarAcross * 0.5f, 4.0f, Sheet.Accent);
    Surface.TextRun(TabSeat.LeastAlong + 30.0f, CentredAcross(TabSeat, Surface.RunExtent(12.5f)), Composition.TabCaption, Sheet.InkPrimary, 12.5f);
    Surface.CrossGlyph(TabSeat.MostAlong - 16.0f, TabSeat.LeastAcross + TabBarAcross * 0.5f, 3.5f, Sheet.InkFaint);

    const PlaneExtent TabAdd = Spanning(TabSeat.MostAlong + 8.0f, CentredAcross(TabBar, 28.0f), 28.0f, 28.0f);
    bool AddRoused = false;
    PresentSeat(TabAdd, "cad.tabadd", AddRoused);
    if (AddRoused)
        Surface.Medallion(TabAdd.LeastAlong + 14.0f, TabAdd.LeastAcross + 14.0f, 14.0f, Covering(0x1A1A1Cu));
    Surface.PlusGlyph(TabAdd.LeastAlong + 14.0f, TabAdd.LeastAcross + 14.0f, 4.5f, AddRoused ? Sheet.InkPrimary : Sheet.InkMuted);

    // ② Part header — brand, dock actions, the part caption and unit chip.
    const PlaneExtent Header = Spanning(Seat.LeastAlong, TabBar.MostAcross, Seat.SpanAlong(), HeaderAcross);
    Surface.Ground(Header, Sheet.HeaderGround, 0.0f);
    Surface.Rule(Header.LeastAlong, Header.MostAcross - 1.0f, Header.SpanAlong(), 1.0f, Sheet.HairEdge);
    Depot.PresentGlyphCentred(Surface, Header.LeastAlong + 20.0f, Header.LeastAcross + HeaderAcross * 0.5f, 20.0f, Sheet.Accent);
    Surface.TextRun(Header.LeastAlong + 36.0f, CentredAcross(Header, Surface.RunExtent(13.0f)), "Frontier CAD", Sheet.InkPrimary, 13.0f);
    Surface.Ground(Spanning(Header.LeastAlong + 134.0f, CentredAcross(Header, 22.0f), 1.0f, 22.0f), Sheet.HairEdge, 0.0f);

    const float CaptionFigures = Surface.MeasureRun(Composition.PartCaption, 12.0f);
    const float ChipExtent = Surface.MeasureRun("MM", 10.0f) + 16.0f;
    const float TitleExtent = CaptionFigures + 7.0f + ChipExtent;
    const float TitleAlong = Header.LeastAlong + (Header.SpanAlong() - TitleExtent) * 0.5f;
    Surface.TextRun(TitleAlong, CentredAcross(Header, Surface.RunExtent(12.0f)), Composition.PartCaption, Sheet.InkPrimary, 12.0f);
    const PlaneExtent ChipSeat = Spanning(TitleAlong + CaptionFigures + 7.0f, CentredAcross(Header, 20.0f), ChipExtent, 20.0f);
    Surface.Ground(ChipSeat, Sheet.CadSoft, 10.0f);
    Surface.Edge(ChipSeat, Sheet.CadEdge, 1.0f, 10.0f);
    Surface.TextRun(Surface.CentredAlong(ChipSeat, "MM", 10.0f), CentredAcross(ChipSeat, Surface.RunExtent(10.0f)), "MM", Sheet.Accent, 10.0f);

    PresentRoundAction(Surface, Spanning(Header.LeastAlong + 146.0f, CentredAcross(Header, 30.0f), 30.0f, 30.0f), Depot, Sheet.InkMuted,
                       false, "cad.dockleft");
    PresentRoundAction(Surface, Spanning(Header.MostAlong - 176.0f, CentredAcross(Header, 30.0f), 30.0f, 30.0f), Depot, Sheet.InkMuted,
                       false, "cad.dockright");

    // ③ The editor body.
    const PlaneExtent Body = Spanning(Seat.LeastAlong, Header.MostAcross, Seat.SpanAlong(), Seat.MostAcross - Header.MostAcross);
    float StageLeading = Body.LeastAlong;
    float StageExtent = Body.SpanAlong();
    if (LeftDockOpen)  { StageLeading += DockAlong;  StageExtent -= DockAlong; }
    if (RightDockOpen) { StageExtent -= DockAlong; }

    if (LeftDockOpen)
    {
        const PlaneExtent Dock = Spanning(Body.LeastAlong, Body.LeastAcross, DockAlong, Body.SpanAcross());
        Surface.Ground(Dock, Sheet.PanelGround, 0.0f);
        Surface.Rule(Dock.MostAlong - 1.0f, Dock.LeastAcross, 1.0f, Dock.SpanAcross(), Sheet.HairEdge);

        static const char* const LeftCaptions[2] = { "Browser", "Tools" };
        PresentCarousel(Surface, Spanning(Dock.LeastAlong, Dock.LeastAcross, Dock.SpanAlong(), CarouselAcross),
                        LeftCaptions, 2u, LeftCarousel, "cad.left");

        if (LeftCarousel == 0u)
        {
            // ①① Browser — filter field, the seeded forest, the count foot.
            const PlaneExtent FieldSeat = Spanning(Dock.LeastAlong + 10.0f, Dock.LeastAcross + CarouselAcross + 10.0f, Dock.SpanAlong() - 46.0f, 30.0f);
            char RetentionRun[48] = "";
            PresentRetentionField(Surface, FieldSeat, RetentionRun, 48u, "Filter outliner...", Sheet.PanelRaised, Sheet.HairEdge,
                                  Sheet.InkPrimary, Sheet.InkFaint);
            PresentRoundAction(Surface, Spanning(Dock.MostAlong - 38.0f, FieldSeat.LeastAcross, 30.0f, 30.0f), Depot, Sheet.InkMuted,
                               false, "cad.newsketch");

            const PlaneExtent BrowserBody = Spanning(Dock.LeastAlong + 8.0f, FieldSeat.MostAcross + 8.0f, Dock.SpanAlong() - 16.0f,
                                                     Dock.SpanAcross() - ConditionAcross - CarouselAcross - 56.0f);
            PresentedRows = 0u;
            float CursorAcross = BrowserBody.LeastAcross - BrowserScroll;
            for (std::uint32_t Ordinal = 0u; Ordinal < RowCount; ++Ordinal)
                PresentBrowserRow(Surface, BrowserBody, Rows[Ordinal], 0u, Depot, CursorAcross);

            const float ContentExtent = static_cast<float>(PresentedRows) * 28.0f;
            const float Ceiling = ContentExtent > BrowserBody.SpanAcross() ? ContentExtent - BrowserBody.SpanAcross() : 0.0f;
            if (BrowserScroll < 0.0f)      BrowserScroll = 0.0f;
            if (BrowserScroll > Ceiling)   BrowserScroll = Ceiling;

            const PlaneExtent DockFoot = Spanning(Dock.LeastAlong, Dock.MostAcross - ConditionAcross, Dock.SpanAlong(), ConditionAcross);
            Surface.Ground(DockFoot, Sheet.PanelRaised, 0.0f);
            Surface.Rule(DockFoot.LeastAlong, DockFoot.LeastAcross, DockFoot.SpanAlong(), 1.0f, Sheet.HairEdge);
            char BodyRun[24];
            std::snprintf(BodyRun, sizeof BodyRun, "%u body", Composition.BodyCount);
            char FeatureRun[24];
            std::snprintf(FeatureRun, sizeof FeatureRun, "%u features", Composition.FeatureCount);
            const float FootAcross = CentredAcross(DockFoot, Surface.RunExtent(11.0f));
            Surface.TextRun(DockFoot.LeastAlong + 12.0f, FootAcross, BodyRun, Sheet.InkMuted, 11.0f);
            Surface.Medallion(DockFoot.LeastAlong + 12.0f + Surface.MeasureRun(BodyRun, 11.0f) + 8.0f, DockFoot.LeastAcross + 15.0f, 1.0f, Sheet.HairEdge);
            Surface.TextRun(DockFoot.LeastAlong + 12.0f + Surface.MeasureRun(BodyRun, 11.0f) + 16.0f, FootAcross, FeatureRun, Sheet.InkMuted, 11.0f);
            Surface.Medallion(DockFoot.MostAlong - 52.0f, DockFoot.LeastAcross + 15.0f, 3.0f, Sheet.Green);
            Surface.TextRun(DockFoot.MostAlong - 44.0f, FootAcross, "Live", Sheet.InkMuted, 11.0f);
        }
        else
        {
            // ①① Tools — environment segments, search, the grouped tool lattice.
            const PlaneExtent EnvironmentSeat = Spanning(Dock.LeastAlong + 10.0f, Dock.LeastAcross + CarouselAcross + 12.0f, Dock.SpanAlong() - 20.0f, 30.0f);
            Surface.Ground(EnvironmentSeat, Sheet.PanelLifted, 8.0f);
            static const char* const Environments[3] = { "Sketch", "Part", "Assembly" };
            float EnvironmentLeading = EnvironmentSeat.LeastAlong + 8.0f;
            for (std::uint32_t Ordinal = 0u; Ordinal < 3u; ++Ordinal)
            {
                const float Extent = Surface.MeasureRun(Environments[Ordinal], 11.0f) + 20.0f;
                const PlaneExtent Segment = Spanning(EnvironmentLeading, EnvironmentSeat.LeastAcross + 3.0f, Extent, 24.0f);
                if (Ordinal == 1u)
                    Surface.Ground(Segment, Sheet.Accent, 6.0f);
                Surface.TextRun(Surface.CentredAlong(Segment, Environments[Ordinal], 11.0f), CentredAcross(Segment, Surface.RunExtent(11.0f)),
                                Environments[Ordinal], Ordinal == 1u ? Covering(0x000000u) : Sheet.InkMuted, 11.0f);
                EnvironmentLeading += Extent + 4.0f;
            }

            const PlaneExtent ToolSearch = Spanning(Dock.LeastAlong + 10.0f, EnvironmentSeat.MostAcross + 10.0f, Dock.SpanAlong() - 20.0f, 28.0f);
            char ToolRun[48] = "";
            PresentRetentionField(Surface, ToolSearch, ToolRun, 48u, "Search tools...", Sheet.PanelRaised, Sheet.HairEdge,
                                  Sheet.InkPrimary, Sheet.InkFaint);

            float GroupAcross = ToolSearch.MostAcross + 14.0f;
            const char* const Groups[3] = { "Draw", "Modify", "Measure" };
            const std::uint32_t GroupCounts[3] = { 4u, 4u, 3u };
            for (std::uint32_t GroupOrdinal = 0u; GroupOrdinal < 3u; ++GroupOrdinal)
            {
                Surface.TextRun(Dock.LeastAlong + 12.0f, GroupAcross, Groups[GroupOrdinal], Sheet.InkFaint, 10.0f);
                GroupAcross += 18.0f;
                float ToolAlong = Dock.LeastAlong + 12.0f;
                for (std::uint32_t ToolOrdinal = 0u; ToolOrdinal < GroupCounts[GroupOrdinal]; ++ToolOrdinal)
                {
                    const PlaneExtent ToolSeat = Spanning(ToolAlong, GroupAcross, 34.0f, 34.0f);
                    const bool Taken = GroupOrdinal == 0u && ToolOrdinal == 0u;
                    PresentRoundAction(Surface, ToolSeat, Depot, Sheet.InkMuted, Taken, "cad.tool");
                    ToolAlong += 38.0f;
                    if (ToolAlong > Dock.MostAlong - 46.0f)
                        break;
                }
                GroupAcross += 42.0f;
            }

            const PlaneExtent DockFoot = Spanning(Dock.LeastAlong, Dock.MostAcross - ConditionAcross, Dock.SpanAlong(), ConditionAcross);
            Surface.Ground(DockFoot, Sheet.PanelRaised, 0.0f);
            Surface.Rule(DockFoot.LeastAlong, DockFoot.LeastAcross, DockFoot.SpanAlong(), 1.0f, Sheet.HairEdge);
            Surface.TextRun(DockFoot.LeastAlong + 12.0f, CentredAcross(DockFoot, Surface.RunExtent(11.0f)), "Line", Sheet.InkMuted, 11.0f);
            Surface.Medallion(DockFoot.MostAlong - 52.0f, DockFoot.LeastAcross + 15.0f, 3.0f, Sheet.Green);
            Surface.TextRun(DockFoot.MostAlong - 44.0f, CentredAcross(DockFoot, Surface.RunExtent(11.0f)), "Ready", Sheet.InkMuted, 11.0f);
        }
    }

    if (RightDockOpen)
    {
        const PlaneExtent Dock = Spanning(Body.MostAlong - DockAlong, Body.LeastAcross, DockAlong, Body.SpanAcross());
        Surface.Ground(Dock, Sheet.PanelGround, 0.0f);
        Surface.Rule(Dock.LeastAlong, Dock.LeastAcross, 1.0f, Dock.SpanAcross(), Sheet.HairEdge);

        static const char* const RightCaptions[3] = { "Properties", "Features", "History" };
        PresentCarousel(Surface, Spanning(Dock.LeastAlong, Dock.LeastAcross, Dock.SpanAlong(), CarouselAcross),
                        RightCaptions, 3u, RightCarousel, "cad.right");

        if (RightCarousel == 0u)
        {
            // ①① Properties — head and the vacated seat.
            const PlaneExtent PaneHead = Spanning(Dock.LeastAlong, Dock.LeastAcross + CarouselAcross, Dock.SpanAlong(), 46.0f);
            const PlaneExtent Tile = Spanning(PaneHead.LeastAlong + 12.0f, CentredAcross(PaneHead, 26.0f), 26.0f, 26.0f);
            Surface.Ground(Tile, Sheet.PanelLifted, 7.0f);
            Depot.PresentGlyph(Surface, Tile.Inset(6.0f, 6.0f), Sheet.InkMuted);
            Surface.TextRun(Tile.MostAlong + 9.0f, PaneHead.LeastAcross + 8.0f, "Properties", Sheet.InkPrimary, 12.5f);
            Surface.Medallion(Tile.MostAlong + 15.0f, PaneHead.LeastAcross + 28.0f, 3.0f, Sheet.InkFaint);
            Surface.TextRun(Tile.MostAlong + 22.0f, PaneHead.LeastAcross + 22.0f, "No selection", Sheet.InkFaint, 10.0f);

            const PlaneExtent Vacant = Spanning(Dock.LeastAlong + 40.0f, PaneHead.MostAcross + 70.0f, Dock.SpanAlong() - 80.0f, 60.0f);
            Depot.PresentGlyphCentred(Surface, Vacant.LeastAlong + 30.0f, Vacant.LeastAcross, 60.0f, Sheet.InkFaint);
            Surface.TextRun(Surface.CentredAlong(Dock, "Nothing selected", 12.5f), Vacant.LeastAcross + 74.0f, "Nothing selected", Sheet.InkPrimary, 12.5f);
            const char* VacantRun = "Pick a feature in the timeline or an entity in the viewport to edit its parameters.";
            Surface.TextRunClipped(Dock.LeastAlong + 40.0f, Vacant.LeastAcross + 94.0f, VacantRun, Sheet.InkFaint, 10.5f, Dock.SpanAlong() - 80.0f);
        }
        else if (RightCarousel == 1u)
        {
            // ①① Features — the timeline head, the vacated list, the foot actions.
            const PlaneExtent PaneHead = Spanning(Dock.LeastAlong, Dock.LeastAcross + CarouselAcross, Dock.SpanAlong(), 46.0f);
            const PlaneExtent Tile = Spanning(PaneHead.LeastAlong + 12.0f, CentredAcross(PaneHead, 26.0f), 26.0f, 26.0f);
            Surface.Ground(Tile, Sheet.PanelLifted, 7.0f);
            Depot.PresentGlyph(Surface, Tile.Inset(6.0f, 6.0f), Sheet.InkMuted);
            Surface.TextRun(Tile.MostAlong + 9.0f, PaneHead.LeastAcross + 8.0f, Composition.PartCaption, Sheet.InkPrimary, 12.5f);
            char SubRun[32];
            std::snprintf(SubRun, sizeof SubRun, "%u features", Composition.FeatureCount);
            Surface.TextRun(Tile.MostAlong + 9.0f, PaneHead.LeastAcross + 23.0f, SubRun, Sheet.InkFaint, 10.0f);
            PresentRoundAction(Surface, Spanning(PaneHead.MostAlong - 40.0f, CentredAcross(PaneHead, 26.0f), 26.0f, 26.0f), Depot, Sheet.InkMuted,
                               false, "cad.featureadd");

            const char* VacantRun = "The timeline is empty. Add a feature to begin the part.";
            Surface.TextRunClipped(Dock.LeastAlong + 40.0f, PaneHead.MostAcross + 40.0f, VacantRun, Sheet.InkFaint, 10.5f, Dock.SpanAlong() - 80.0f);

            const PlaneExtent PaneFoot = Spanning(Dock.LeastAlong, Dock.MostAcross - ConditionAcross, Dock.SpanAlong(), ConditionAcross);
            Surface.Ground(PaneFoot, Sheet.PanelRaised, 0.0f);
            Surface.Rule(PaneFoot.LeastAlong, PaneFoot.LeastAcross, PaneFoot.SpanAlong(), 1.0f, Sheet.HairEdge);
            Surface.TextRun(PaneFoot.LeastAlong + 12.0f, CentredAcross(PaneFoot, Surface.RunExtent(11.0f)), "Collapse", Sheet.InkMuted, 11.0f);
            const PlaneExtent RollSeat = Spanning(PaneFoot.MostAlong - 96.0f, CentredAcross(PaneFoot, 22.0f), 86.0f, 22.0f);
            Surface.Ground(RollSeat, Sheet.Accent, 6.0f);
            Surface.TextRun(Surface.CentredAlong(RollSeat, "Roll to end", 11.0f), CentredAcross(RollSeat, Surface.RunExtent(11.0f)),
                            "Roll to end", Covering(0x000000u), 11.0f);
        }
        else
        {
            // ①① Revision list — head with undo and redo, the vacated list.
            const PlaneExtent PaneHead = Spanning(Dock.LeastAlong, Dock.LeastAcross + CarouselAcross, Dock.SpanAlong(), 40.0f);
            Surface.TextRun(PaneHead.LeastAlong + 12.0f, CentredAcross(PaneHead, Surface.RunExtent(12.5f)), "History", Sheet.InkPrimary, 12.5f);
            PresentRoundAction(Surface, Spanning(PaneHead.MostAlong - 78.0f, CentredAcross(PaneHead, 28.0f), 28.0f, 28.0f), Depot, Sheet.InkMuted,
                               false, "cad.undo");
            PresentRoundAction(Surface, Spanning(PaneHead.MostAlong - 42.0f, CentredAcross(PaneHead, 28.0f), 28.0f, 28.0f), Depot, Sheet.InkMuted,
                               false, "cad.redo");
            const char* VacantRun = "No revisions yet. Every edit lands here, scrubbable.";
            Surface.TextRunClipped(Dock.LeastAlong + 40.0f, PaneHead.MostAcross + 40.0f, VacantRun, Sheet.InkFaint, 10.5f, Dock.SpanAlong() - 80.0f);
        }
    }

    // ④ The stage.
    const PlaneExtent Stage = Spanning(StageLeading, Body.LeastAcross, StageExtent, Body.SpanAcross());
    PresentStage(Surface, Stage, Composition, Depot);
}

//------------------------------------------------------------------------------------------------------------------------
//                                                       THE BROWSER
//------------------------------------------------------------------------------------------------------------------------

void CadWorkspacePanel::PresentBrowserRow(RecordingSurface& Surface, const PlaneExtent& Body, const BrowserRowDeclaration& Row,
                                          std::uint32_t Depth, const IconDepot& Depot, float& CursorAcross)
{
    CadInk Sheet;
    const float RowExtent = 28.0f;
    const PlaneExtent RowSeat = Spanning(Body.LeastAlong, CursorAcross, Body.SpanAlong(), RowExtent);
    CursorAcross += RowExtent;
    ++PresentedRows;
    if (RowSeat.MostAcross <= Body.LeastAcross || RowSeat.LeastAcross >= Body.MostAcross)
    {
        // ① Rows off the body still advance the cursor through their forest.
        if (Row.EnclosureCount > 0u && (Row.Expanded == nullptr || *Row.Expanded))
            for (std::uint32_t Ordinal = 0u; Ordinal < Row.EnclosureCount; ++Ordinal)
                PresentBrowserRow(Surface, Body, Row.Enclosed[Ordinal], Depth + 1u, Depot, CursorAcross);
        return;
    }

    const bool Branch = Row.EnclosureCount > 0u;
    const bool Open   = Branch && (Row.Expanded == nullptr || *Row.Expanded);
    const bool Taken  = std::strcmp(TakenIdentity, Row.Caption) == 0;
    const double Dimming = Row.Hidden != nullptr && *Row.Hidden ? 0.45 : 1.0;

    const PlaneExtent RowGround = RowSeat.Inset(0.0f, 1.0f);
    if (Taken)
    {
        Surface.Ground(RowGround, Sheet.RowTakenGround, 6.0f);
        Surface.Edge(RowGround, Sheet.RowTakenEdge, 1.0f, 6.0f);
    }
    else if (Surface.PointerWithin(RowGround))
    {
        Surface.Ground(RowGround, Sheet.RowRoused, 6.0f);
    }

    char Identity[48];
    std::snprintf(Identity, sizeof Identity, "cad.row.%s", Row.Caption);
    ImGui::PushID(Identity);
    ImGui::SetCursorScreenPos(ImVec2(RowGround.LeastAlong, RowGround.LeastAcross));
    ImGui::InvisibleButton("row", ImVec2(RowGround.SpanAlong(), RowGround.SpanAcross()));
    const bool RowClicked = ImGui::IsItemClicked();
    ImGui::PopID();

    float Cursor = RowGround.LeastAlong + Depth * 14.0f;
    if (Branch)
    {
        const PlaneExtent TwistSeat = Spanning(Cursor, RowGround.LeastAcross, 18.0f, RowExtent);
        const bool TwistRoused = Surface.PointerWithin(TwistSeat);
        if (TwistRoused && ImGui::IsMouseClicked(0) && Row.Expanded != nullptr)
            *Row.Expanded = !*Row.Expanded;
        Surface.Chevron(TwistSeat.LeastAlong + 9.0f, RowGround.LeastAcross + RowExtent * 0.5f, 3.5f,
                        TwistRoused ? Sheet.InkPrimary : Sheet.InkFaint, Open);
    }
    Cursor += 18.0f;

    const InkOrdinate Tint = Row.Swatch != 0u ? Covering(Row.Swatch) : BrowserTint(Row.Classification);
    const PlaneExtent GlyphSeat = Spanning(Cursor, CentredAcross(RowGround, 16.0f), 16.0f, 16.0f);
    Depot.PresentGlyph(Surface, GlyphSeat, TranslucentTint(Tint, Dimming));
    Cursor += 20.0f;

    if (Row.Swatch != 0u)
    {
        Surface.Medallion(Cursor + 3.0f, RowGround.LeastAcross + RowExtent * 0.5f, 3.0f, Covering(Row.Swatch));
        Cursor += 8.0f;
    }

    const PlaneExtent ActionSeat = Spanning(RowGround.MostAlong - 24.0f, CentredAcross(RowGround, 20.0f), 20.0f, 20.0f);
    if (Row.AxisRun[0] != '\0')
        Surface.TextRun(ActionSeat.LeastAlong - Surface.MeasureRun(Row.AxisRun, 10.0f) - 8.0f,
                        CentredAcross(RowGround, Surface.RunExtent(10.0f)), Row.AxisRun, Sheet.InkFaint, 10.0f);
    else if (Branch)
    {
        char CountFigures[8];
        std::snprintf(CountFigures, sizeof CountFigures, "%u", Row.EnclosureCount);
        Surface.TextRun(ActionSeat.LeastAlong - Surface.MeasureRun(CountFigures, 10.0f) - 8.0f,
                        CentredAcross(RowGround, Surface.RunExtent(10.0f)), CountFigures, Sheet.InkFaint, 10.0f);
    }

    const bool EyeRelevant = Row.Hidden != nullptr && (Surface.PointerWithin(RowGround) || *Row.Hidden || Row.Locked != nullptr);
    if (EyeRelevant)
    {
        const bool EyeRoused = Surface.PointerWithin(ActionSeat);
        if (EyeRoused && ImGui::IsMouseClicked(0) && Row.Locked == nullptr)
            *Row.Hidden = !(*Row.Hidden);
        Surface.EyeGlyph(ActionSeat.LeastAlong + 10.0f, ActionSeat.LeastAcross + 10.0f, 7.0f,
                         EyeRoused ? Sheet.InkPrimary : Sheet.InkFaint, Row.Hidden != nullptr && *Row.Hidden);
    }

    Surface.TextRunClipped(Cursor + 2.0f, CentredAcross(RowGround, Surface.RunExtent(12.0f)), Row.Caption,
                           InkOrdinate{ static_cast<std::uint8_t>(237u * Dimming + 0.5), static_cast<std::uint8_t>(237u * Dimming + 0.5),
                                        static_cast<std::uint8_t>(237u * Dimming + 0.5), 255u },
                           12.0f, ActionSeat.LeastAlong - Cursor - 26.0f);

    if (RowClicked)
        std::snprintf(TakenIdentity, sizeof TakenIdentity, "%s", Row.Caption);

    if (Open)
        for (std::uint32_t Ordinal = 0u; Ordinal < Row.EnclosureCount; ++Ordinal)
            PresentBrowserRow(Surface, Body, Row.Enclosed[Ordinal], Depth + 1u, Depot, CursorAcross);
}

//------------------------------------------------------------------------------------------------------------------------
//                                                        THE STAGE
//------------------------------------------------------------------------------------------------------------------------

void CadWorkspacePanel::PresentStage(RecordingSurface& Surface, const PlaneExtent& Stage, const CadComposition& Composition,
                                     const IconDepot& Depot)
{
    CadInk Sheet;
    Surface.Ground(Stage, Sheet.StageGround, 0.0f);

    const float CentreAlong = Stage.LeastAlong + Stage.SpanAlong() * 0.5f;
    const float HorizonAcross = Stage.LeastAcross + Stage.SpanAcross() * 0.42f;

    // ① The dot lattice — a receding ground of dots below the horizon.
    for (int StepZ = 1; StepZ <= 9; ++StepZ)
    {
        const double Perspective = StepZ / 9.0;
        const float RowAcross = HorizonAcross + static_cast<float>(Perspective) * Stage.SpanAcross() * 0.55f;
        const float Spread = 40.0f + static_cast<float>(1.0 - Perspective) * 340.0f;
        const float Radius = 0.6f + static_cast<float>(Perspective) * 1.2f;
        const int Dots = static_cast<int>(Spread / 26.0f);
        const int DotCap = Dots > 11 ? 11 : Dots;
        for (int Dot = -DotCap; Dot <= DotCap; ++Dot)
        {
            const float DotAlong = CentreAlong + Dot * 26.0f;
            if (DotAlong < Stage.LeastAlong + 8.0f || DotAlong > Stage.MostAlong - 8.0f)
                continue;
            const double Fade = 1.0 - std::fabs(static_cast<double>(Dot)) / (DotCap + 1.0) * 0.4;
            Surface.Medallion(DotAlong, RowAcross, Radius, Partial(0xFFFFFFu, 0.14 * Fade * Perspective));
        }
    }

    // ② The origin triad — red, green, blue strokes from the origin.
    const float OriginAlong = CentreAlong;
    const float OriginAcross = Stage.LeastAcross + Stage.SpanAcross() * 0.55f;
    Surface.Stroke(OriginAlong, OriginAcross, OriginAlong - 52.0f, OriginAcross + 26.0f, 1.6f, Sheet.Red);
    Surface.Stroke(OriginAlong, OriginAcross, OriginAlong + 52.0f, OriginAcross + 26.0f, 1.6f, Sheet.Green);
    Surface.Stroke(OriginAlong, OriginAcross, OriginAlong, OriginAcross - 56.0f, 1.6f, Sheet.Blue);

    // ③ The part pill — dot, caption, environment run.
    const PlaneExtent PartPill = Spanning(CentreAlong - 92.0f, Stage.LeastAcross + 14.0f, 184.0f, 30.0f);
    Surface.Ground(PartPill, Partial(0x0E1014u, 0.85), 15.0f);
    Surface.Edge(PartPill, Sheet.HairEdge, 1.0f, 15.0f);
    Surface.Medallion(PartPill.LeastAlong + 20.0f, PartPill.LeastAcross + 15.0f, 4.0f, Sheet.Accent);
    Surface.TextRun(PartPill.LeastAlong + 32.0f, CentredAcross(PartPill, Surface.RunExtent(12.0f)), Composition.PartCaption, Sheet.InkPrimary, 12.0f);
    Surface.TextRun(PartPill.LeastAlong + 32.0f + Surface.MeasureRun(Composition.PartCaption, 12.0f) + 10.0f,
                    CentredAcross(PartPill, Surface.RunExtent(10.0f)), "MODEL", Sheet.InkMuted, 10.0f);

    // ④ The mode pill — OBJECT, seated taken.
    const PlaneExtent ModePill = Spanning(CentreAlong - 46.0f, PartPill.MostAcross + 10.0f, 92.0f, 24.0f);
    Surface.Ground(ModePill, Sheet.CadSoft, 12.0f);
    Surface.Edge(ModePill, Sheet.CadEdge, 1.0f, 12.0f);
    Surface.TextRun(Surface.CentredAlong(ModePill, "OBJECT", 10.0f), CentredAcross(ModePill, Surface.RunExtent(10.0f)), "OBJECT", Sheet.Accent, 10.0f);

    // ⑤ The display bar and the settings action, upper trailing.
    const PlaneExtent DisplayBar = Spanning(Stage.MostAlong - 168.0f, Stage.LeastAcross + 14.0f, 152.0f, 38.0f);
    Surface.Ground(DisplayBar, Partial(0x0E1014u, 0.80), 19.0f);
    Surface.Edge(DisplayBar, Sheet.HairEdge, 1.0f, 19.0f);
    for (std::uint32_t Ordinal = 0u; Ordinal < 4u; ++Ordinal)
        PresentRoundAction(Surface, Spanning(DisplayBar.LeastAlong + 4.0f + Ordinal * 36.0f, DisplayBar.LeastAcross + 4.0f, 30.0f, 30.0f),
                           Depot, Sheet.InkMuted, Ordinal == 0u, "cad.disp");
    PresentRoundAction(Surface, Spanning(Stage.MostAlong - 46.0f, DisplayBar.MostAcross + 8.0f, 32.0f, 32.0f), Depot, Sheet.InkMuted,
                       false, "cad.settings");

    // ⑥ The condition strip.
    const PlaneExtent ConditionStrip = Spanning(Stage.LeastAlong, Stage.MostAcross - ConditionAcross, Stage.SpanAlong(), ConditionAcross);
    Surface.Ground(ConditionStrip, Sheet.PanelRaised, 0.0f);
    Surface.Rule(ConditionStrip.LeastAlong, ConditionStrip.LeastAcross, ConditionStrip.SpanAlong(), 1.0f, Sheet.HairEdge);
    const float StripAcross = CentredAcross(ConditionStrip, Surface.RunExtent(11.0f));
    float StripAlong = ConditionStrip.LeastAlong + 14.0f;
    const char* const Stats[4] = { "Model", "Object", "Nothing selected", "mm \xC2\xB7 0.01 tol" };
    for (std::uint32_t Ordinal = 0u; Ordinal < 4u; ++Ordinal)
    {
        Surface.TextRun(StripAlong, StripAcross, Stats[Ordinal], Sheet.InkPrimary, 11.0f);
        StripAlong += Surface.MeasureRun(Stats[Ordinal], 11.0f) + 14.0f;
        Surface.Ground(Spanning(StripAlong - 7.0f, CentredAcross(ConditionStrip, 14.0f), 1.0f, 14.0f), Sheet.HairEdge, 0.0f);
    }
    Surface.TextRun(ConditionStrip.MostAlong - 130.0f, StripAcross, "Snap On", Sheet.InkPrimary, 11.0f);
    Surface.Medallion(ConditionStrip.MostAlong - 96.0f, ConditionStrip.LeastAcross + 15.0f, 1.0f, Sheet.HairEdge);
    Surface.Medallion(ConditionStrip.MostAlong - 80.0f, ConditionStrip.LeastAcross + 15.0f, 3.0f, Sheet.Green);
    Surface.TextRun(ConditionStrip.MostAlong - 72.0f, StripAcross, "Rebuilt", Sheet.InkMuted, 11.0f);
}

}   // namespace Slate
