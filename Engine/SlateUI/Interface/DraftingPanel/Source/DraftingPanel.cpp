//============================================================================================================================================
//                                                         DRAFTINGPANEL.CPP
//============================================================================================================================================
// 🧩 The drafting seat — directory on the left, the Properties & Actions bar and the metadata pane on the right.

#include "SlateUI/Interface/DraftingPanel/Api/DraftingPanel.h"
#include "SlateUI/Interface/ControlPanel/Api/ControlPanel.h"

#include "imgui.h"

#include <cstdio>
#include <cstring>

namespace Slate
{
namespace Reference
{

namespace
{

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

}   // namespace

void DraftingPanel::Advance(PanelExchange& Surface, const PlaneExtent& Seat, OutlinerPanel& Directory,
                            const OutlinerRowDeclaration* Rows, std::uint32_t RowCount,
                            const OutlinerRowDeclaration* Inspected, ProfileOrdinates& Profile, const IconDepot& Depot)
{
    WorkspaceInk Sheet;
    AdvanceRaised = false;

    // ① The directory column — the standalone outliner, reused as the scene director.
    const PlaneExtent DirectorySeat = Spanning(Seat.LeastAlong, Seat.LeastAcross, 350.0f, Seat.SpanAcross());
    Directory.Advance(Surface, DirectorySeat, Rows, RowCount, OutlinerComposition{ "Directory", "Bracket_Rev4" }, Depot);

    // ② The Properties & Actions bar.
    const PlaneExtent RightColumn = Spanning(DirectorySeat.MostAlong, Seat.LeastAcross,
                                             Seat.SpanAlong() - 350.0f, Seat.SpanAcross());
    Surface.Ground(RightColumn, Sheet.SunkenGround, 0.0f);
    const PlaneExtent Bar = Spanning(RightColumn.LeastAlong, RightColumn.LeastAcross, RightColumn.SpanAlong(), 46.0f);
    Surface.Rule(Bar.LeastAlong, Bar.MostAcross - 1.0f, Bar.SpanAlong(), 1.0f, Sheet.HairEdge);

    bool BarRoused = false;
    PresentSeat(Bar, "drafting.bar", BarRoused);
    const PlaneExtent Tile = Spanning(Bar.LeastAlong + 10.0f, CentredAcross(Bar, 24.0f), 24.0f, 24.0f);
    Surface.Ground(Tile, Covering(0x000000u), 6.0f);
    Depot.PresentGlyph(Surface, Tile.Inset(5.0f, 5.0f), InkOrdinate{ 255u, 255u, 255u, 255u });
    Surface.TextRun(Tile.MostAlong + 10.0f, CentredAcross(Bar, Surface.RunExtent(12.5f)), "Properties & Actions",
                    Sheet.InkPrimary, 12.5f);
    Surface.Chevron(Bar.MostAlong - 18.0f, Bar.LeastAcross + 23.0f, 4.0f, Sheet.InkMuted, false);

    // ③ The metadata pane beneath the bar.
    PresentMetadata(Surface, Spanning(RightColumn.LeastAlong, Bar.MostAcross, RightColumn.SpanAlong(),
                                      RightColumn.MostAcross - Bar.MostAcross), Inspected, Profile, Depot);
}

void DraftingPanel::PresentMetadata(PanelExchange& Surface, const PlaneExtent& Seat, const OutlinerRowDeclaration* Declared,
                                    const ProfileOrdinates& Profile, const IconDepot& Depot)
{
    WorkspaceInk Sheet;

    if (Declared == nullptr)
    {
        // ① The overview — target tile, caption pair.
        const float CentreAlong = Seat.LeastAlong + Seat.SpanAlong() * 0.5f;
        const PlaneExtent Tile = Spanning(CentreAlong - 24.0f, Seat.LeastAcross + 48.0f, 48.0f, 48.0f);
        Surface.Ground(Tile, Sheet.TileGround, 12.0f);
        Surface.Edge(Tile, Sheet.HairEdge, 1.0f, 12.0f);
        Depot.PresentGlyph(Surface, Tile.Inset(12.0f, 12.0f), Sheet.InkMuted);
        Surface.TextRun(Surface.CentredAlong(Seat, "Properties Overview", 12.5f), Tile.MostAcross + 16.0f,
                        "Properties Overview", Sheet.InkPrimary, 12.5f);
        const char* VacantRun = "Select a record in the directory to view its details.";
        Surface.TextRunClipped(Surface.CentredAlong(Seat, VacantRun, 11.0f), Tile.MostAcross + 34.0f, VacantRun,
                               Sheet.InkFaint, 11.0f, 220.0f);
        return;
    }

    const InkOrdinate Hue = ClassificationTint(Declared->Classification);
    const PlaneExtent Body = Spanning(Seat.LeastAlong + 12.0f, Seat.LeastAcross + 12.0f, Seat.SpanAlong() - 24.0f, 0.0f);
    float CursorAcross = Body.LeastAcross;

    // ② Hero — black tile with the white glyph, name, classification label.
    const PlaneExtent Hero = Spanning(Body.LeastAlong, CursorAcross, Body.SpanAlong(), 54.0f);
    Surface.Ground(Hero, Sheet.TileGround, 12.0f);
    Surface.Edge(Hero, Sheet.HairEdge, 1.0f, 12.0f);
    const PlaneExtent HeroTile = Spanning(Hero.LeastAlong + 10.0f, CentredAcross(Hero, 34.0f), 34.0f, 34.0f);
    Surface.Ground(HeroTile, Covering(0x000000u), 8.0f);
    Depot.PresentGlyph(Surface, HeroTile.Inset(5.0f, 5.0f), InkOrdinate{ 255u, 255u, 255u, 255u });
    Surface.TextRunClipped(HeroTile.MostAlong + 10.0f, Hero.LeastAcross + 10.0f, Declared->Caption, Sheet.InkPrimary, 13.0f,
                           Hero.SpanAlong() - 70.0f);
    Surface.TextRun(HeroTile.MostAlong + 10.0f, Hero.LeastAcross + 28.0f, ClassificationLabel(Declared->Classification), Hue, 10.5f);
    CursorAcross += 66.0f;

    // ③ The stats — token, presence, nesting, transform, and the shape's own figures.
    const auto StatRow = [&](const char* CaptionRun, const char* ValueRun)
    {
        Surface.TextRun(Body.LeastAlong, CursorAcross + 7.0f, CaptionRun, Sheet.InkMuted, 11.5f);
        Surface.TextRun(Body.MostAlong - Surface.MeasureRun(ValueRun, 11.5f), CursorAcross + 7.0f, ValueRun,
                        Covering(0xF2F2F4u), 11.5f);
        Surface.Rule(Body.LeastAlong, CursorAcross + 28.0f, Body.SpanAlong(), 1.0f, Sheet.HairEdge);
        CursorAcross += 28.0f;
    };

    StatRow("Token", Declared->Identity);
    StatRow("Visible", Declared->Hidden != nullptr && *Declared->Hidden ? "hidden" : "shown");
    if (Declared->EnclosureCount > 0u)
    {
        char NestedRun[24];
        std::snprintf(NestedRun, sizeof NestedRun, "%u records", Declared->EnclosureCount);
        StatRow("Nested", NestedRun);
    }
    char PositionRun[48];
    std::snprintf(PositionRun, sizeof PositionRun, "[%.1f, %.1f, %.1f]", Profile.Position[0], Profile.Position[1], Profile.Position[2]);
    StatRow("Position", PositionRun);
    if (Declared->Classification == DirectoryClassification::Cylinder ||
        Declared->Classification == DirectoryClassification::Sphere)
    {
        char RadiusRun[24];
        std::snprintf(RadiusRun, sizeof RadiusRun, "%.2f mm", Profile.Radius);
        StatRow("Radius", RadiusRun);
    }
    if (Declared->Classification == DirectoryClassification::Cylinder)
    {
        char HeightRun[24];
        std::snprintf(HeightRun, sizeof HeightRun, "%.2f mm", Profile.Height);
        StatRow("Height", HeightRun);
    }
    if (Declared->Classification == DirectoryClassification::Sketch)
    {
        char CurvesRun[16];
        std::snprintf(CurvesRun, sizeof CurvesRun, "%.0f", Profile.CurveTally);
        StatRow("Curves", CurvesRun);
        StatRow("Status", Profile.FullyConstrained ? "Constrained" : "Unconstrained");
    }
    if (Declared->Classification == DirectoryClassification::Solid)
    {
        char DepthRun[24];
        std::snprintf(DepthRun, sizeof DepthRun, "%.2f mm", Profile.ExtrudeDepth);
        StatRow("Depth", DepthRun);
    }

    // ④ The albedo row — mono ordinates beside the circle.
    char AlbedoRun[48];
    std::snprintf(AlbedoRun, sizeof AlbedoRun, "%u, %u, %u", Profile.Albedo[0], Profile.Albedo[1], Profile.Albedo[2]);
    Surface.TextRun(Body.LeastAlong, CursorAcross + 7.0f, "Albedo", Sheet.InkMuted, 11.5f);
    Surface.TextRun(Body.MostAlong - Surface.MeasureRun(AlbedoRun, 11.5f) - 26.0f, CursorAcross + 7.0f, AlbedoRun,
                    Covering(0xF2F2F4u), 11.5f);
    Surface.Medallion(Body.MostAlong - 10.0f, CursorAcross + 14.0f, 8.0f,
                      InkOrdinate{ Profile.Albedo[0], Profile.Albedo[1], Profile.Albedo[2], Profile.Albedo[3] });
    Surface.Ring(Body.MostAlong - 10.0f, CursorAcross + 14.0f, 8.0f, 1.0f, Sheet.HairEdgeStrong);
    Surface.Rule(Body.LeastAlong, CursorAcross + 28.0f, Body.SpanAlong(), 1.0f, Sheet.HairEdge);
    CursorAcross += 40.0f;

    // ⑤ The advance action — accent ground, the glyph, the caption, the Tab pill.
    const PlaneExtent Advance = Spanning(Body.LeastAlong, CursorAcross, Body.SpanAlong(), 32.0f);
    bool AdvanceRoused = false;
    if (PresentSeat(Advance, "metadata.advance", AdvanceRoused))
        AdvanceRaised = true;
    Surface.Ground(Advance, Partial(0x5B8CFFu, 0.13), 8.0f);
    Surface.Edge(Advance, Sheet.Accent, 1.0f, 8.0f);
    Depot.PresentGlyphCentred(Surface, Surface.CentredAlong(Advance, "Properties & History", 11.5f) - 12.0f,
                              Advance.LeastAcross + 16.0f, 14.0f, Sheet.InkPrimary);
    Surface.TextRun(Surface.CentredAlong(Advance, "Properties & History", 11.5f), CentredAcross(Advance, Surface.RunExtent(11.5f)),
                    "Properties & History", Sheet.InkPrimary, 11.5f);
    const float TabExtent = Surface.MeasureRun("Tab", 10.0f) + 16.0f;
    const PlaneExtent TabPill = Spanning(Surface.CentredAlong(Advance, "Properties & History", 11.5f) +
                                         Surface.MeasureRun("Properties & History", 11.5f) * 0.5f + 10.0f,
                                         CentredAcross(Advance, 18.0f), TabExtent, 18.0f);
    Surface.Ground(TabPill, Sheet.SunkenGround, 9.0f);
    Surface.TextRun(Surface.CentredAlong(TabPill, "Tab", 10.0f), CentredAcross(TabPill, Surface.RunExtent(10.0f)),
                    "Tab", Sheet.InkMuted, 10.0f);
    CursorAcross += 44.0f;

    // ⑥ The action list — new record, rename, duplicate, presence, delete.
    CursorAcross += 12.0f;
    Surface.TextRun(Body.LeastAlong + 4.0f, CursorAcross, "ACTIONS", Sheet.InkFaint, 10.0f);
    CursorAcross += 22.0f;

    const struct { const char* CaptionRun; const char* HintRun; bool Danger; } Actions[5] =
    {
        { "New record", "",     false },
        { "Rename",     "F2",   false },
        { "Duplicate",  "Ctrl D", false },
        { "Hide",       "H",    false },
        { "Delete",     "Del",  true  },
    };
    for (std::uint32_t Ordinal = 0u; Ordinal < 5u; ++Ordinal)
    {
        const PlaneExtent Action = Spanning(Body.LeastAlong, CursorAcross, Body.SpanAlong(), 29.0f);
        bool ActionRoused = false;
        PresentSeat(Action, "metadata.action", ActionRoused);
        if (ActionRoused)
            Surface.Ground(Action, Sheet.TileRoused, 8.0f);
        const InkOrdinate GlyphInk = Actions[Ordinal].Danger ? Covering(0xFF6B6Bu) : Sheet.InkMuted;
        Depot.PresentGlyphCentred(Surface, Action.LeastAlong + 12.0f, Action.LeastAcross + 14.5f, 14.0f, GlyphInk);
        Surface.TextRun(Action.LeastAlong + 26.0f, CentredAcross(Action, Surface.RunExtent(11.5f)), Actions[Ordinal].CaptionRun,
                        Actions[Ordinal].Danger ? Covering(0xFF6B6Bu) : Sheet.InkPrimary, 11.5f);
        if (Actions[Ordinal].HintRun[0] != '\0')
            Surface.TextRun(Action.MostAlong - Surface.MeasureRun(Actions[Ordinal].HintRun, 10.0f),
                            CentredAcross(Action, Surface.RunExtent(10.0f)), Actions[Ordinal].HintRun, Sheet.InkFaint, 10.0f);
        CursorAcross += 29.0f;
        if (Ordinal == 0u)
        {
            Surface.Rule(Body.LeastAlong + 8.0f, CursorAcross - 3.0f, Body.SpanAlong() - 16.0f, 1.0f, Sheet.HairEdge);
            CursorAcross += 3.0f;
        }
    }

    // ⑦ The foot — hue square, label, token.
    const PlaneExtent Foot = Spanning(Seat.LeastAlong, Seat.MostAcross - 26.0f, Seat.SpanAlong(), 26.0f);
    Surface.Ground(Foot, Sheet.SunkenGround, 0.0f);
    Surface.Rule(Foot.LeastAlong, Foot.LeastAcross, Foot.SpanAlong(), 1.0f, Sheet.HairEdge);
    Surface.Ground(Spanning(Foot.LeastAlong + 10.0f, CentredAcross(Foot, 8.0f) + 4.0f, 8.0f, 8.0f), Hue, 2.0f);
    Surface.TextRun(Foot.LeastAlong + 26.0f, CentredAcross(Foot, Surface.RunExtent(10.0f)),
                    ClassificationLabel(Declared->Classification), Sheet.InkMuted, 10.0f);
    Surface.TextRun(Foot.MostAlong - Surface.MeasureRun(Declared->Identity, 10.0f) - 10.0f,
                    CentredAcross(Foot, Surface.RunExtent(10.0f)), Declared->Identity, Sheet.InkMuted, 10.0f);
}

}   // namespace Reference
}   // namespace Slate
