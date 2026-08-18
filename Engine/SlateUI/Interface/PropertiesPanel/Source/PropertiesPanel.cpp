//============================================================================================================================================
//                                                        PROPERTIESPANEL.CPP
//============================================================================================================================================
// 🧩 Record cards through ControlPanel widgets, the carousel, and the revision stack — Inspector.tsx on the recording seam.

#include "Engine/SlateUI/Interface/PropertiesPanel/Api/PropertiesPanel.h"
#include "Engine/SlateUI/Interface/ControlPanel/Api/ControlPanel.h"

#include "imgui.h"

#include <cctype>
#include <cstdio>
#include <cstring>

namespace Slate
{

//------------------------------------------------------------------------------------------------------------------------
//                                                     THE CATEGORY SET
//------------------------------------------------------------------------------------------------------------------------

const char* RevisionCategoryRun(RevisionCategory Category)
{
    switch (Category)
    {
        case RevisionCategory::Start:     return "Start";
        case RevisionCategory::Feature:   return "Feature";
        case RevisionCategory::Parameter: return "Params";
        case RevisionCategory::Sketch:    return "Sketch";
        case RevisionCategory::Relocate:  return "Relocate";
        case RevisionCategory::Grouping:  return "Group";
        case RevisionCategory::Create:    return "Create";
        case RevisionCategory::Edit:      return "Edit";
        case RevisionCategory::Drop:      return "Drop";
        case RevisionCategory::CategoryCount: break;
    }
    return "Edit";
}

InkOrdinate RevisionCategoryTint(RevisionCategory Category)
{
    switch (Category)
    {
        case RevisionCategory::Start:     return Covering(0x7EC8FFu);
        case RevisionCategory::Feature:   return Covering(0xFFB24Du);
        case RevisionCategory::Parameter: return Covering(0x4FD18Bu);
        case RevisionCategory::Sketch:    return Covering(0x37D6D6u);
        case RevisionCategory::Relocate:  return Covering(0x5B8CFFu);
        case RevisionCategory::Grouping:  return Covering(0xB98BFFu);
        case RevisionCategory::Create:    return Covering(0x7EC8FFu);
        case RevisionCategory::Edit:      return Covering(0xC99B6Au);
        case RevisionCategory::Drop:      return Covering(0xFF6B6Bu);
        case RevisionCategory::CategoryCount: break;
    }
    return Covering(0xC99B6Au);
}

void SeatProfile(ProfileOrdinates& Profile, const OutlinerRowDeclaration& Row)
{
    std::snprintf(Profile.Name, sizeof Profile.Name, "%s", Row.Caption);
    Profile.Visible = Row.Hidden == nullptr || !(*Row.Hidden);

    switch (Row.Classification)
    {
        case DirectoryClassification::Scene:
            Profile.Units = 1u;  Profile.ToleranceLinear = 0.01;  Profile.ToleranceAngular = 0.5;
            std::snprintf(Profile.DocumentPath, sizeof Profile.DocumentPath, "/Projects/Bracket_Rev4.wsdoc");
            break;
        case DirectoryClassification::Enclosure:
            Profile.NestedTally = static_cast<double>(Row.EnclosureCount);
            break;
        case DirectoryClassification::Sketch:
            Profile.PlaneChoice = 0u;  Profile.ConstraintTally = 12.0;  Profile.CurveTally = 8.0;
            Profile.FullyConstrained = false;  Profile.GridSnap = 0.5;
            break;
        case DirectoryClassification::Solid:
            Profile.ExtrudeDepth = 12.5;  Profile.DraftAngle = 0.0;  Profile.WallThickness = 2.5;  Profile.CappedEnds = true;
            break;
        case DirectoryClassification::Cylinder:
            Profile.Radius = 6.25;  Profile.Height = 18.0;  Profile.SegmentTally = 32.0;  Profile.CappedEnds = true;
            break;
        case DirectoryClassification::Sphere:
            Profile.Radius = 8.4;  Profile.SegmentTally = 48.0;  Profile.RingTally = 24.0;
            break;
        case DirectoryClassification::Cone:
            break;   // 📝 the prototype seats cone, revolve and loft through the shared defaults
        case DirectoryClassification::Revolve:
            Profile.SweepAngle = 360.0;  Profile.AxisChoice = 1u;  Profile.ProfileClosed = true;
            break;
        case DirectoryClassification::Loft:
            Profile.SectionTally = 3.0;  Profile.TangencyStart = 0.0;  Profile.TangencyEnd = 0.0;  Profile.Ruled = false;
            break;
        case DirectoryClassification::ClassificationCount:
            break;
    }
}

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

//------------------------------------------------------------------------------------------------------------------------
//                                                       THE INSPECTOR
//------------------------------------------------------------------------------------------------------------------------

namespace
{

/// 🧩 Counts the fields the classification's cards carry — the foot's figure, whatever page stands shown.
/// cost  ✔️
std::uint32_t CountFields(DirectoryClassification Classification)
{
    switch (Classification)
    {
        case DirectoryClassification::Scene:     return 3u + 3u;
        case DirectoryClassification::Enclosure: return 2u + 3u + 3u;
        case DirectoryClassification::Solid:     return 2u + 4u + 3u + 4u;
        case DirectoryClassification::Cylinder:  return 2u + 4u + 3u + 4u;
        case DirectoryClassification::Sphere:    return 2u + 3u + 3u + 4u;
        default:                                 return 2u + 3u + 4u;
    }
}

}   // namespace

void PropertiesPanel::Advance(RecordingSurface& Surface, const PlaneExtent& Seat, const OutlinerRowDeclaration* Declared,
                              ProfileOrdinates& Profile, const IconDepot& Depot,
                              const RevisionDeclaration* Revisions, std::uint32_t RevisionCount,
                              const OutlinerRowDeclaration* Forest, std::uint32_t ForestCount)
{
    WorkspaceInk Sheet;
    const ControlSheet Controls = ControlSheetFromWorkspace(Sheet);
    BackRaised = false;

    Surface.Ground(Seat, Sheet.SunkenGround, 0.0f);

    // ① Head — black tile, white glyph, name and classification label, the back action.
    const PlaneExtent Head = Spanning(Seat.LeastAlong, Seat.LeastAcross, Seat.SpanAlong(), 46.0f);
    Surface.Rule(Head.LeastAlong, Head.MostAcross - 1.0f, Head.SpanAlong(), 1.0f, Sheet.HairEdge);

    const PlaneExtent Tile = Spanning(Head.LeastAlong + 10.0f, CentredAcross(Head, 24.0f), 24.0f, 24.0f);
    Surface.Ground(Tile, Covering(0x000000u), 6.0f);
    Depot.PresentGlyph(Surface, Tile.Inset(4.0f, 4.0f), InkOrdinate{ 255u, 255u, 255u, 255u });

    if (Declared == nullptr)
    {
        Surface.TextRun(Tile.MostAlong + 10.0f, CentredAcross(Head, Surface.RunExtent(12.5f)), "Nothing selected", Sheet.InkPrimary, 12.5f);
    }
    else
    {
        const InkOrdinate Hue = ClassificationTint(Declared->Classification);
        Surface.TextRun(Tile.MostAlong + 10.0f, Head.LeastAcross + 8.0f, Declared->Caption, Sheet.InkPrimary, 12.5f);
        Surface.TextRun(Tile.MostAlong + 10.0f, Head.LeastAcross + 25.0f, ClassificationLabel(Declared->Classification), Hue, 10.0f);
    }

    const float BackExtent = Surface.MeasureRun("Back to scene directory", 11.0f) + 34.0f;
    const PlaneExtent BackSeat = Spanning(Head.MostAlong - BackExtent - 8.0f, CentredAcross(Head, 28.0f), BackExtent, 28.0f);
    bool BackRoused = false;
    if (PresentSeat(BackSeat, "inspector.back", BackRoused))
        BackRaised = true;
    if (BackRoused)
        Surface.Ground(BackSeat, Sheet.TileRoused, 6.0f);
    Surface.Chevron(BackSeat.LeastAlong + 10.0f, BackSeat.LeastAcross + 14.0f, 4.0f, Sheet.InkMuted, false);
    Surface.TextRun(BackSeat.LeastAlong + 19.0f, CentredAcross(BackSeat, Surface.RunExtent(11.0f)),
                    "Back to scene directory", Sheet.InkMuted, 11.0f);

    if (Declared == nullptr)
    {
        const char* VacantRun = "Select a record to inspect its properties.";
        Surface.TextRunClipped(Seat.LeastAlong + 24.0f, Head.MostAcross + 31.0f + 24.0f, VacantRun, Sheet.InkFaint, 11.5f,
                               Seat.SpanAlong() - 48.0f);
        return;
    }

    // ② The carousel — Properties | History, the taken one underlined by the accent.
    const PlaneExtent Carousel = Spanning(Seat.LeastAlong, Head.MostAcross, Seat.SpanAlong(), 31.0f);
    Surface.Ground(Carousel, Sheet.SunkenGround, 0.0f);
    Surface.Rule(Carousel.LeastAlong, Carousel.MostAcross - 1.0f, Carousel.SpanAlong(), 1.0f, Sheet.HairEdge);
    static const char* const CarouselCaptions[2] = { "Properties", "History" };
    for (std::uint32_t Ordinal = 0u; Ordinal < 2u; ++Ordinal)
    {
        const PlaneExtent Tab = Spanning(Carousel.LeastAlong + 6.0f + Ordinal * (Carousel.SpanAlong() - 12.0f) * 0.5f,
                                         Carousel.LeastAcross, (Carousel.SpanAlong() - 12.0f) * 0.5f, 31.0f);
        bool TabRoused = false;
        if (PresentSeat(Tab, Ordinal == 0u ? "carousel.properties" : "carousel.history", TabRoused))
            CarouselMode = Ordinal;
        Surface.TextRun(Surface.CentredAlong(Tab, CarouselCaptions[Ordinal], 11.5f), CentredAcross(Tab, Surface.RunExtent(11.5f)),
                        CarouselCaptions[Ordinal], CarouselMode == Ordinal ? Sheet.InkPrimary : Sheet.InkMuted, 11.5f);
        if (CarouselMode == Ordinal)
            Surface.Ground(Spanning(Tab.LeastAlong, Tab.MostAcross - 2.0f, Tab.SpanAlong(), 2.0f), Sheet.Accent, 0.0f);
    }

    // ③ The foot — hue square and the field count, whatever page stands shown.
    const PlaneExtent Foot = Spanning(Seat.LeastAlong, Seat.MostAcross - 26.0f, Seat.SpanAlong(), 26.0f);
    Surface.Ground(Foot, Sheet.SunkenGround, 0.0f);
    Surface.Rule(Foot.LeastAlong, Foot.LeastAcross, Foot.SpanAlong(), 1.0f, Sheet.HairEdge);
    const InkOrdinate FootHue = ClassificationTint(Declared->Classification);
    Surface.Ground(Spanning(Foot.LeastAlong + 10.0f, CentredAcross(Foot, 8.0f) + 4.0f, 8.0f, 8.0f), FootHue, 2.0f);

    char FieldsRun[24];
    std::snprintf(FieldsRun, sizeof FieldsRun, "%u fields", CountFields(Declared->Classification));
    Surface.TextRun(Foot.LeastAlong + 26.0f, CentredAcross(Foot, Surface.RunExtent(10.0f)), FieldsRun, Sheet.InkMuted, 10.0f);

    if (CarouselMode == 1u)
    {
        // ①① The history page — the revision stack of the declared row and everything it encloses.
        PresentHistory(Surface, Spanning(Seat.LeastAlong + 7.0f, Carousel.MostAcross, Seat.SpanAlong() - 11.0f,
                                         Foot.LeastAcross - Carousel.MostAcross),
                       Declared, Revisions, RevisionCount, Forest, ForestCount, Depot);
        return;
    }

    // ④ The cards, scrolled by wheel.
    const PlaneExtent Body = Spanning(Seat.LeastAlong + 7.0f, Carousel.MostAcross, Seat.SpanAlong() - 11.0f,
                                      Foot.LeastAcross - Carousel.MostAcross);
    if (Surface.PointerWithin(Body))
        ScrollAcross -= ImGui::GetIO().MouseWheel * 32.0f;

    bool CardFolded[4] = { false, false, false, false };
    float CursorAcross = Body.LeastAcross - ScrollAcross;

    const ControlRowDeclaration RowSpec = { "", 88.0f, 13.5f };

    const auto PresentCardHead = [&](const PlaneExtent& Card, const char* TitleRun, std::uint32_t Fields, bool& Folded, const char* Identity)
    {
        const PlaneExtent CardHead = Spanning(Card.LeastAlong, Card.LeastAcross, Card.SpanAlong(), 31.0f);
        bool HeadRoused = false;
        if (PresentSeat(CardHead, Identity, HeadRoused))
            Folded = !Folded;
        Surface.Ground(CardHead, Sheet.SunkenGround, 12.0f, CornerSelection::UpperLeading);
        Surface.Ground(CardHead, Sheet.SunkenGround, 12.0f, CornerSelection::UpperTrailing);
        Surface.Ground(Spanning(CardHead.LeastAlong + 12.0f, CardHead.LeastAcross, CardHead.SpanAlong() - 24.0f, CardHead.SpanAcross()),
                       Sheet.SunkenGround, 0.0f);
        if (!Folded)
            Surface.Rule(CardHead.LeastAlong, CardHead.MostAcross - 1.0f, CardHead.SpanAlong(), 1.0f, Sheet.HairEdge);
        Surface.Chevron(CardHead.LeastAlong + 10.0f, CardHead.LeastAcross + 15.5f, 3.5f, Sheet.InkFaint, !Folded);
        char TitleUpper[32];
        std::snprintf(TitleUpper, sizeof TitleUpper, "%s", TitleRun);
        for (char* Letter = TitleUpper; *Letter != '\0'; ++Letter)
            *Letter = static_cast<char>(std::toupper(static_cast<unsigned char>(*Letter)));
        Surface.TextRun(CardHead.LeastAlong + 24.0f, CentredAcross(CardHead, Surface.RunExtent(10.5f)), TitleUpper, Sheet.InkMuted, 10.5f);
        char CountFigures[8];
        std::snprintf(CountFigures, sizeof CountFigures, "%u", Fields);
        Surface.TextRun(CardHead.MostAlong - Surface.MeasureRun(CountFigures, 10.0f) - 10.0f,
                        CentredAcross(CardHead, Surface.RunExtent(10.0f)), CountFigures, Sheet.InkFaint, 10.0f);
    };

    const auto OpenCard = [&](float ExtentAcross, const char* TitleRun, std::uint32_t Fields, bool& Folded, const char* Identity) -> PlaneExtent
    {
        const PlaneExtent Card = Spanning(Body.LeastAlong, CursorAcross, Body.SpanAlong(), Folded ? 31.0f : ExtentAcross);
        Surface.Ground(Card, Covering(0x0A0A0Bu), 12.0f);
        Surface.Edge(Card, Sheet.HairEdge, 1.0f, 12.0f);
        PresentCardHead(Card, TitleRun, Fields, Folded, Identity);
        CursorAcross += Card.SpanAcross() + 6.0f;
        // 📝 the foot counts fields through CountFields; cards add nothing here
        return Card;
    };

    const auto CardBody = [&](const PlaneExtent& Card) -> PlaneExtent
    {
        return Spanning(Card.LeastAlong + 10.0f, Card.LeastAcross + 31.0f + 10.0f, Card.SpanAlong() - 20.0f, 0.0f);
    };

    const DirectoryClassification Classification = Declared->Classification;
    static const char* const UnitsCaptions[4] = { "Inches", "Millimetres", "Centimetres", "Metres" };
    static const char* const ShadingCaptions[3] = { "Smooth", "Faceted", "Flat" };
    static const char* const BooleanCaptions[3] = { "Union", "Subtract", "Intersect" };

    // ⑤ Record card — name, visibility, and the scene's units.
    {
        const std::uint32_t Fields = Classification == DirectoryClassification::Scene ? 3u : 2u;
        const PlaneExtent Card = OpenCard(31.0f + 10.0f + static_cast<float>(Fields) * 44.0f, "Record", Fields, CardFolded[0], "card.record");
        if (!CardFolded[0])
        {
            const PlaneExtent CardArea = CardBody(Card);
            float RowAcross = CardArea.LeastAcross;
            const ControlRowDeclaration NameRow = { "Name", 88.0f, 13.5f };
            PresentTextRow(Surface, Spanning(CardArea.LeastAlong, RowAcross, CardArea.SpanAlong(), 36.0f), NameRow,
                           Profile.Name, 32u, Controls, "record.name");
            RowAcross += 44.0f;
            const ControlRowDeclaration VisibleRow = { "Visible", 88.0f, 13.5f };
            PresentSwitchRow(Surface, Spanning(CardArea.LeastAlong, RowAcross, CardArea.SpanAlong(), 36.0f), VisibleRow,
                             Profile.Visible, Controls, "record.visible");
            RowAcross += 44.0f;
            if (Classification == DirectoryClassification::Scene)
            {
                const ControlRowDeclaration UnitsRow = { "Units", 88.0f, 13.5f };
                PresentDropdownRow(Surface, Spanning(CardArea.LeastAlong, RowAcross, CardArea.SpanAlong(), 36.0f), UnitsRow,
                                   UnitsCaptions, 4u, Profile.Units, Controls, "record.units");
            }
        }
    }

    const auto PresentTransformCard = [&](std::uint32_t Slot)
    {
        const PlaneExtent Card = OpenCard(31.0f + 10.0f + 3u * 44.0f + 10.0f, "Transform", 3u, CardFolded[Slot], "card.transform");
        if (CardFolded[Slot])
            return;
        const PlaneExtent CardArea = CardBody(Card);
        const ControlRowDeclaration PositionRow = { "Position", 88.0f, 13.5f };
        PresentVectorRow(Surface, Spanning(CardArea.LeastAlong, CardArea.LeastAcross, CardArea.SpanAlong(), 36.0f), PositionRow,
                         Profile.Position, 0.1, Controls, "transform.position");
        const ControlRowDeclaration RotationRow = { "Rotation", 88.0f, 13.5f };
        PresentVectorRow(Surface, Spanning(CardArea.LeastAlong, CardArea.LeastAcross + 44.0f, CardArea.SpanAlong(), 36.0f), RotationRow,
                         Profile.Rotation, 0.5, Controls, "transform.rotation");
        const ControlRowDeclaration ScaleRow = { "Scale", 88.0f, 13.5f };
        PresentVectorRow(Surface, Spanning(CardArea.LeastAlong, CardArea.LeastAcross + 88.0f, CardArea.SpanAlong(), 36.0f), ScaleRow,
                         Profile.Scale, 0.01, Controls, "transform.scale");
    };

    const auto PresentAppearanceCard = [&](std::uint32_t Slot)
    {
        const PlaneExtent Card = OpenCard(31.0f + 10.0f + 4u * 44.0f + 10.0f, "Appearance", 4u, CardFolded[Slot], "card.appearance");
        if (CardFolded[Slot])
            return;
        const PlaneExtent CardArea = CardBody(Card);
        float RowAcross = CardArea.LeastAcross;
        static bool ColourOpen = false;
        const ControlRowDeclaration AlbedoRow = { "Albedo", 88.0f, 13.5f };
        PresentColourRow(Surface, Spanning(CardArea.LeastAlong, RowAcross, CardArea.SpanAlong(), 36.0f), AlbedoRow,
                         Profile.Albedo, ColourOpen, Controls, "appearance.albedo");
        RowAcross += 44.0f;
        const ControlRowDeclaration RoughnessRow = { "Roughness", 88.0f, 13.5f };
        const SliderDeclaration RoughnessRange = { 0.0, 1.0, 2u, "\xC2\xB7", 78.0f };
        PresentSliderRow(Surface, Spanning(CardArea.LeastAlong, RowAcross, CardArea.SpanAlong(), 36.0f), RoughnessRow,
                         RoughnessRange, Profile.Roughness, Controls, "appearance.roughness");
        RowAcross += 44.0f;
        const ControlRowDeclaration MetalnessRow = { "Metalness", 88.0f, 13.5f };
        const SliderDeclaration MetalnessRange = { 0.0, 1.0, 2u, "\xC2\xB7", 78.0f };
        PresentSliderRow(Surface, Spanning(CardArea.LeastAlong, RowAcross, CardArea.SpanAlong(), 36.0f), MetalnessRow,
                         MetalnessRange, Profile.Metalness, Controls, "appearance.metalness");
        RowAcross += 44.0f;
        const ControlRowDeclaration ShadingRow = { "Shading", 88.0f, 13.5f };
        PresentDropdownRow(Surface, Spanning(CardArea.LeastAlong, RowAcross, CardArea.SpanAlong(), 36.0f), ShadingRow,
                           ShadingCaptions, 3u, Profile.ShadingMode, Controls, "appearance.shading");
    };

    switch (Classification)
    {
        case DirectoryClassification::Scene:
        {
            const PlaneExtent Card = OpenCard(31.0f + 10.0f + 3u * 44.0f + 10.0f, "Tolerance", 3u, CardFolded[1], "card.tolerance");
            if (!CardFolded[1])
            {
                const PlaneExtent CardArea = CardBody(Card);
                float RowAcross = CardArea.LeastAcross;
                const ControlRowDeclaration LinearRow = { "Linear", 88.0f, 13.5f };
                const SliderDeclaration LinearRange = { 0.0, 10.0, 3u, "mm", 78.0f };
                PresentScalarRow(Surface, Spanning(CardArea.LeastAlong, RowAcross, CardArea.SpanAlong(), 36.0f), LinearRow,
                                 LinearRange, 0.001, Profile.ToleranceLinear, Controls, "tolerance.linear");
                RowAcross += 44.0f;
                const ControlRowDeclaration AngularRow = { "Angular", 88.0f, 13.5f };
                const SliderDeclaration AngularRange = { 0.0, 10.0, 1u, "\xC2\xB0", 78.0f };
                PresentScalarRow(Surface, Spanning(CardArea.LeastAlong, RowAcross, CardArea.SpanAlong(), 36.0f), AngularRow,
                                 AngularRange, 0.1, Profile.ToleranceAngular, Controls, "tolerance.angular");
                RowAcross += 44.0f;
                const ControlRowDeclaration DocumentRow = { "Document", 88.0f, 13.5f };
                PresentTextRow(Surface, Spanning(CardArea.LeastAlong, RowAcross, CardArea.SpanAlong(), 36.0f), DocumentRow,
                               Profile.DocumentPath, 64u, Controls, "tolerance.document");
            }
            break;
        }
        case DirectoryClassification::Enclosure:
        {
            const PlaneExtent Card = OpenCard(31.0f + 10.0f + 3u * 44.0f + 10.0f, "Group", 3u, CardFolded[1], "card.group");
            if (!CardFolded[1])
            {
                const PlaneExtent CardArea = CardBody(Card);
                float RowAcross = CardArea.LeastAcross;
                const ControlRowDeclaration BooleanRow = { "Boolean", 88.0f, 13.5f };
                PresentSegmentRow(Surface, Spanning(CardArea.LeastAlong, RowAcross, CardArea.SpanAlong(), 36.0f), BooleanRow,
                                  BooleanCaptions, 3u, Profile.BooleanMode, Controls, "group.boolean");
                RowAcross += 44.0f;
                const ControlRowDeclaration SuppressRow = { "Suppress", 88.0f, 13.5f };
                PresentSwitchRow(Surface, Spanning(CardArea.LeastAlong, RowAcross, CardArea.SpanAlong(), 36.0f), SuppressRow,
                                 Profile.Suppressed, Controls, "group.suppress");
                RowAcross += 44.0f;
                const ControlRowDeclaration RecordsRow = { "Records", 88.0f, 13.5f };
                const SliderDeclaration RecordsRange = { 0.0, 64.0, 0u, "ct", 78.0f };
                PresentScalarRow(Surface, Spanning(CardArea.LeastAlong, RowAcross, CardArea.SpanAlong(), 36.0f), RecordsRow,
                                 RecordsRange, 1.0, Profile.NestedTally, Controls, "group.records");
            }
            PresentTransformCard(2u);
            break;
        }
        case DirectoryClassification::Solid:
        {
            const PlaneExtent Card = OpenCard(31.0f + 10.0f + 4u * 44.0f + 10.0f, "Extrusion", 4u, CardFolded[1], "card.extrusion");
            if (!CardFolded[1])
            {
                const PlaneExtent CardArea = CardBody(Card);
                float RowAcross = CardArea.LeastAcross;
                const ControlRowDeclaration DepthRow = { "Depth", 88.0f, 13.5f };
                const SliderDeclaration DepthRange = { 0.0, 100.0, 2u, "mm", 78.0f };
                PresentScalarRow(Surface, Spanning(CardArea.LeastAlong, RowAcross, CardArea.SpanAlong(), 36.0f), DepthRow,
                                 DepthRange, 0.1, Profile.ExtrudeDepth, Controls, "extrusion.depth");
                RowAcross += 44.0f;
                const ControlRowDeclaration DraftRow = { "Draft", 88.0f, 13.5f };
                const SliderDeclaration DraftRange = { -30.0, 30.0, 1u, "\xC2\xB0", 78.0f };
                PresentSliderRow(Surface, Spanning(CardArea.LeastAlong, RowAcross, CardArea.SpanAlong(), 36.0f), DraftRow,
                                 DraftRange, Profile.DraftAngle, Controls, "extrusion.draft");
                RowAcross += 44.0f;
                const ControlRowDeclaration WallRow = { "Wall", 88.0f, 13.5f };
                const SliderDeclaration WallRange = { 0.0, 20.0, 2u, "mm", 78.0f };
                PresentScalarRow(Surface, Spanning(CardArea.LeastAlong, RowAcross, CardArea.SpanAlong(), 36.0f), WallRow,
                                 WallRange, 0.1, Profile.WallThickness, Controls, "extrusion.wall");
                RowAcross += 44.0f;
                const ControlRowDeclaration CapRow = { "Cap ends", 88.0f, 13.5f };
                PresentSwitchRow(Surface, Spanning(CardArea.LeastAlong, RowAcross, CardArea.SpanAlong(), 36.0f), CapRow,
                                 Profile.CappedEnds, Controls, "extrusion.cap");
            }
            PresentTransformCard(2u);
            PresentAppearanceCard(3u);
            break;
        }
        case DirectoryClassification::Cylinder:
        {
            const PlaneExtent Card = OpenCard(31.0f + 10.0f + 4u * 44.0f + 10.0f, "Cylinder", 4u, CardFolded[1], "card.cylinder");
            if (!CardFolded[1])
            {
                const PlaneExtent CardArea = CardBody(Card);
                float RowAcross = CardArea.LeastAcross;
                const ControlRowDeclaration RadiusRow = { "Radius", 88.0f, 13.5f };
                const SliderDeclaration RadiusRange = { 0.0, 100.0, 2u, "mm", 78.0f };
                PresentScalarRow(Surface, Spanning(CardArea.LeastAlong, RowAcross, CardArea.SpanAlong(), 36.0f), RadiusRow,
                                 RadiusRange, 0.05, Profile.Radius, Controls, "cylinder.radius");
                RowAcross += 44.0f;
                const ControlRowDeclaration HeightRow = { "Height", 88.0f, 13.5f };
                const SliderDeclaration HeightRange = { 0.0, 100.0, 2u, "mm", 78.0f };
                PresentScalarRow(Surface, Spanning(CardArea.LeastAlong, RowAcross, CardArea.SpanAlong(), 36.0f), HeightRow,
                                 HeightRange, 0.1, Profile.Height, Controls, "cylinder.height");
                RowAcross += 44.0f;
                const ControlRowDeclaration SegmentsRow = { "Segments", 88.0f, 13.5f };
                const SliderDeclaration SegmentsRange = { 6.0, 128.0, 0u, "ct", 78.0f };
                PresentSliderRow(Surface, Spanning(CardArea.LeastAlong, RowAcross, CardArea.SpanAlong(), 36.0f), SegmentsRow,
                                 SegmentsRange, Profile.SegmentTally, Controls, "cylinder.segments");
                RowAcross += 44.0f;
                const ControlRowDeclaration CapRow = { "Cap ends", 88.0f, 13.5f };
                PresentSwitchRow(Surface, Spanning(CardArea.LeastAlong, RowAcross, CardArea.SpanAlong(), 36.0f), CapRow,
                                 Profile.CappedEnds, Controls, "cylinder.cap");
            }
            PresentTransformCard(2u);
            PresentAppearanceCard(3u);
            break;
        }
        case DirectoryClassification::Sphere:
        {
            const PlaneExtent Card = OpenCard(31.0f + 10.0f + 3u * 44.0f + 10.0f, "Sphere", 3u, CardFolded[1], "card.sphere");
            if (!CardFolded[1])
            {
                const PlaneExtent CardArea = CardBody(Card);
                float RowAcross = CardArea.LeastAcross;
                const ControlRowDeclaration RadiusRow = { "Radius", 88.0f, 13.5f };
                const SliderDeclaration RadiusRange = { 0.0, 100.0, 2u, "mm", 78.0f };
                PresentScalarRow(Surface, Spanning(CardArea.LeastAlong, RowAcross, CardArea.SpanAlong(), 36.0f), RadiusRow,
                                 RadiusRange, 0.05, Profile.Radius, Controls, "sphere.radius");
                RowAcross += 44.0f;
                const ControlRowDeclaration SegmentsRow = { "Segments", 88.0f, 13.5f };
                const SliderDeclaration SegmentsRange = { 6.0, 128.0, 0u, "ct", 78.0f };
                PresentSliderRow(Surface, Spanning(CardArea.LeastAlong, RowAcross, CardArea.SpanAlong(), 36.0f), SegmentsRow,
                                 SegmentsRange, Profile.SegmentTally, Controls, "sphere.segments");
                RowAcross += 44.0f;
                const ControlRowDeclaration RingsRow = { "Rings", 88.0f, 13.5f };
                const SliderDeclaration RingsRange = { 3.0, 64.0, 0u, "ct", 78.0f };
                PresentSliderRow(Surface, Spanning(CardArea.LeastAlong, RowAcross, CardArea.SpanAlong(), 36.0f), RingsRow,
                                 RingsRange, Profile.RingTally, Controls, "sphere.rings");
            }
            PresentTransformCard(2u);
            PresentAppearanceCard(3u);
            break;
        }
        default:
            PresentTransformCard(1u);
            PresentAppearanceCard(2u);
            break;
    }

}

//------------------------------------------------------------------------------------------------------------------------
//                                                       THE HISTORY
//------------------------------------------------------------------------------------------------------------------------

void PropertiesPanel::PresentHistory(RecordingSurface& Surface, const PlaneExtent& Seat, const OutlinerRowDeclaration* Declared,
                                     const RevisionDeclaration* Revisions, std::uint32_t RevisionCount,
                                     const OutlinerRowDeclaration* Forest, std::uint32_t ForestCount, const IconDepot& Depot)
{
    WorkspaceInk Sheet;
    if (Declared == nullptr)
    {
        Surface.TextRunClipped(Seat.LeastAlong + 24.0f, Seat.LeastAcross + 24.0f,
                               "No history events found for this selection or its children.", Sheet.InkFaint, 11.5f,
                               Seat.SpanAlong() - 48.0f);
        return;
    }

    // ① The declared row and everything it encloses, in forest order — the history's token walk.
    const OutlinerRowDeclaration* TokenWalk[16];
    std::uint32_t TokenCount = 0u;
    const auto Walk = [&](const OutlinerRowDeclaration* Rows, std::uint32_t Count, auto&& Recurse) -> void
    {
        for (std::uint32_t Ordinal = 0u; Ordinal < Count && TokenCount < 16u; ++Ordinal)
        {
            TokenWalk[TokenCount++] = &Rows[Ordinal];
            Recurse(Rows[Ordinal].Enclosed, Rows[Ordinal].EnclosureCount, Recurse);
        }
    };
    Walk(Declared, 1u, Walk);
    (void)Forest;
    (void)ForestCount;

    float CursorAcross = Seat.LeastAcross;
    for (std::uint32_t WalkOrdinal = 0u; WalkOrdinal < TokenCount; ++WalkOrdinal)
    {
        const OutlinerRowDeclaration* Record = TokenWalk[WalkOrdinal];
        std::uint32_t RecordRevisions = 0u;
        for (std::uint32_t Ordinal = 0u; Ordinal < RevisionCount; ++Ordinal)
            if (std::strcmp(Revisions[Ordinal].Identity, Record->Identity) == 0)
                ++RecordRevisions;
        if (RecordRevisions == 0u)
            continue;

        const InkOrdinate Hue = ClassificationTint(Record->Classification);

        // ② Group head — hue tile, name, classification chip, ops count, fold chevron.
        const PlaneExtent GroupHead = Spanning(Seat.LeastAlong, CursorAcross, Seat.SpanAlong(), 32.0f);
        bool GroupRoused = false;
        static bool GroupFolded[16] = {};
        char GroupIdentity[32];
        std::snprintf(GroupIdentity, sizeof GroupIdentity, "history.%s", Record->Identity);
        bool Folded = GroupFolded[WalkOrdinal];
        if (PresentSeat(GroupHead, GroupIdentity, GroupRoused))
            GroupFolded[WalkOrdinal] = !Folded;
        Folded = GroupFolded[WalkOrdinal];

        const PlaneExtent GroupTile = Spanning(GroupHead.LeastAlong + 12.0f, CentredAcross(GroupHead, 20.0f), 20.0f, 20.0f);
        Surface.Ground(GroupTile, Hue, 5.0f);
        Depot.PresentGlyph(Surface, GroupTile.Inset(4.0f, 4.0f), Covering(0x0A0A0Bu));
        Surface.TextRun(GroupTile.MostAlong + 10.0f, CentredAcross(GroupHead, Surface.RunExtent(12.5f)), Record->Caption,
                        Sheet.InkPrimary, 12.5f);

        const char* Label = ClassificationLabel(Record->Classification);
        const float LabelExtent = Surface.MeasureRun(Label, 10.0f) + 12.0f;
        const PlaneExtent Chip = Spanning(GroupTile.MostAlong + 10.0f + Surface.MeasureRun(Record->Caption, 12.5f) + 10.0f,
                                          CentredAcross(GroupHead, 18.0f), LabelExtent, 18.0f);
        Surface.Ground(Chip, Sheet.StandingGround, 4.0f);
        Surface.Edge(Chip, Sheet.HairEdge, 1.0f, 4.0f);
        Surface.TextRun(Surface.CentredAlong(Chip, Label, 10.0f), CentredAcross(Chip, Surface.RunExtent(10.0f)), Label,
                        Sheet.InkMuted, 10.0f);

        char OpsRun[16];
        std::snprintf(OpsRun, sizeof OpsRun, "%u ops", RecordRevisions);
        Surface.TextRun(GroupHead.MostAlong - Surface.MeasureRun(OpsRun, 10.0f) - 26.0f,
                        CentredAcross(GroupHead, Surface.RunExtent(10.0f)), OpsRun, Sheet.InkMuted, 10.0f);
        Surface.Chevron(GroupHead.MostAlong - 14.0f, GroupHead.LeastAcross + 16.0f, 3.5f, Sheet.InkFaint, !Folded);
        CursorAcross += 36.0f;

        if (Folded)
            continue;

        // ③ The revision stack — numbered bubbles on the hue spine.
        std::uint32_t Presented = 0u;
        for (std::uint32_t Ordinal = 0u; Ordinal < RevisionCount; ++Ordinal)
        {
            const RevisionDeclaration& Revision = Revisions[Ordinal];
            if (std::strcmp(Revision.Identity, Record->Identity) != 0)
                continue;

            const bool First = Presented == 0u;
            const bool Last  = Presented + 1u == RecordRevisions;
            const bool Expanded = Revision.Expanded != nullptr && *Revision.Expanded;

            const PlaneExtent Row = Spanning(Seat.LeastAlong + 7.0f, CursorAcross, Seat.SpanAlong() - 14.0f,
                                             Expanded ? 44.0f + 92.0f : 44.0f + 4.0f);

            // ① The bubble and the spine.
            Surface.Medallion(Row.LeastAlong + 12.5f, Row.LeastAcross + 19.0f, 12.5f, Hue);
            char OrdinalRun[8];
            std::snprintf(OrdinalRun, sizeof OrdinalRun, "%02u", Presented);
            Surface.TextRun(Row.LeastAlong + 12.5f - Surface.MeasureRun(OrdinalRun, 10.0f) * 0.5f, Row.LeastAcross + 13.0f,
                            OrdinalRun, InkOrdinate{ 255u, 255u, 255u, 255u }, 10.0f);
            const float SpineTop = Row.LeastAcross + (First ? 19.0f : 0.0f);
            const float SpineExtent = Last ? 19.0f : Row.SpanAcross();
            Surface.Ground(Spanning(Row.LeastAlong + 28.0f, SpineTop, 6.0f, SpineExtent), Hue, 4.0f);
            if (!First)
                Surface.Ground(Spanning(Row.LeastAlong + 28.0f, Row.LeastAcross, 6.0f, 19.0f), Hue, 0.0f);
            Surface.Medallion(Row.LeastAlong + 31.0f, Row.LeastAcross + 19.0f, 3.5f, InkOrdinate{ 255u, 255u, 255u, 255u });

            // ② The revision card.
            const PlaneExtent CardSeat = Spanning(Row.LeastAlong + 40.0f, Row.LeastAcross, Row.SpanAlong() - 40.0f, 44.0f);
            char RevisionIdentity[40];
            std::snprintf(RevisionIdentity, sizeof RevisionIdentity, "revision.%s.%u", Record->Identity, Ordinal);
            bool CardRoused = false;
            if (PresentSeat(CardSeat, RevisionIdentity, CardRoused) && Revision.Expanded != nullptr)
                *Revision.Expanded = !Expanded;

            if (Expanded)
            {
                Surface.Ground(CardSeat, Sheet.AccentSoft, 8.0f, CornerSelection::UpperLeading);
                Surface.Ground(CardSeat, Sheet.AccentSoft, 8.0f, CornerSelection::UpperTrailing);
                Surface.Edge(CardSeat, Sheet.Accent, 1.0f, 8.0f, CornerSelection::UpperLeading);
                Surface.Edge(CardSeat, Sheet.Accent, 1.0f, 8.0f, CornerSelection::UpperTrailing);
            }
            else
            {
                Surface.Ground(CardSeat, Sheet.TileGround, 8.0f);
                Surface.Edge(CardSeat, Sheet.HairEdge, 1.0f, 8.0f);
                if (CardRoused)
                    Surface.Ground(CardSeat, Sheet.TileRoused, 8.0f);
            }

            Surface.TextRunClipped(CardSeat.LeastAlong + 8.0f, CardSeat.LeastAcross + 7.0f, Revision.TitleRun,
                                   Covering(0xF2F2F4u), 12.5f, CardSeat.SpanAlong() - 70.0f);
            Depot.PresentGlyphCentred(Surface, CardSeat.LeastAlong + 14.0f, CardSeat.LeastAcross + 30.0f, 11.0f,
                                      RevisionCategoryTint(Revision.Category));
            Surface.TextRunClipped(CardSeat.LeastAlong + 22.0f, CardSeat.LeastAcross + 25.0f, Revision.SubtitleRun,
                                   Sheet.InkMuted, 10.0f, CardSeat.SpanAlong() - 90.0f);
            Surface.TextRun(CardSeat.MostAlong - Surface.MeasureRun(Revision.ClockRun, 10.0f) - 30.0f,
                            CentredAcross(CardSeat, Surface.RunExtent(10.0f)), Revision.ClockRun, Sheet.InkFaint, 10.0f);
            Surface.Chevron(CardSeat.MostAlong - 17.0f, CardSeat.LeastAcross + 22.0f, 3.5f, Sheet.InkMuted, Expanded);

            // ③ The fold — author and date, the comment seat, the edited value.
            if (Expanded)
            {
                const PlaneExtent Fold = Spanning(CardSeat.LeastAlong, CardSeat.MostAcross, CardSeat.SpanAlong(), 92.0f);
                Surface.Ground(Fold, Sheet.AccentSoft, 8.0f, CornerSelection::LowerLeading);
                Surface.Ground(Fold, Sheet.AccentSoft, 8.0f, CornerSelection::LowerTrailing);
                Surface.Edge(Fold, Sheet.Accent, 1.0f, 8.0f, CornerSelection::LowerLeading);
                Surface.Edge(Fold, Sheet.Accent, 1.0f, 8.0f, CornerSelection::LowerTrailing);
                Surface.Ground(Spanning(Fold.LeastAlong + 8.0f, Fold.LeastAcross, Fold.SpanAlong() - 16.0f, Fold.SpanAcross()),
                               Sheet.AccentSoft, 0.0f);

                char AuthorRun[64];
                std::snprintf(AuthorRun, sizeof AuthorRun, "By %s", Revision.AuthorRun[0] != '\0' ? Revision.AuthorRun : "System");
                Surface.TextRun(Fold.LeastAlong + 8.0f, Fold.LeastAcross + 7.0f, AuthorRun, Sheet.InkMuted, 10.0f);
                Surface.TextRun(Fold.MostAlong - Surface.MeasureRun(Revision.DateRun, 10.0f) - 8.0f, Fold.LeastAcross + 7.0f,
                                Revision.DateRun, Sheet.InkMuted, 10.0f);

                const PlaneExtent CommentSeat = Spanning(Fold.LeastAlong + 8.0f, Fold.LeastAcross + 24.0f, Fold.SpanAlong() - 16.0f, 34.0f);
                Surface.Ground(CommentSeat, Sheet.SunkenGround, 8.0f);
                Surface.Edge(CommentSeat, Sheet.HairEdge, 1.0f, 8.0f);
                Surface.TextRun(CommentSeat.LeastAlong + 8.0f, CommentSeat.LeastAcross + 5.0f, "COMMENT", Sheet.InkFaint, 10.0f);
                Surface.TextRunClipped(CommentSeat.LeastAlong + 8.0f, CommentSeat.LeastAcross + 19.0f, Revision.CommentRun,
                                       Covering(0xF2F2F4u), 11.5f, CommentSeat.SpanAlong() - 16.0f);

                if (Revision.EditRun[0] != '\0')
                {
                    const PlaneExtent ValueSeat = Spanning(Fold.LeastAlong + 8.0f, Fold.LeastAcross + 64.0f, Fold.SpanAlong() - 16.0f, 24.0f);
                    Surface.Ground(ValueSeat, Sheet.SunkenGround, 8.0f);
                    Surface.Edge(ValueSeat, Sheet.HairEdge, 1.0f, 8.0f);
                    Surface.TextRun(ValueSeat.LeastAlong + 8.0f, CentredAcross(ValueSeat, Surface.RunExtent(10.0f)), "Value",
                                    Sheet.InkMuted, 10.0f);
                    const PlaneExtent ValueField = Spanning(ValueSeat.LeastAlong + 52.0f, CentredAcross(ValueSeat, 20.0f),
                                                            ValueSeat.SpanAlong() - 60.0f, 20.0f);
                    Surface.Ground(ValueField, Sheet.StandingGround, 4.0f);
                    Surface.Edge(ValueField, Sheet.HairEdge, 1.0f, 4.0f);
                    Surface.TextRun(ValueField.LeastAlong + 8.0f, CentredAcross(ValueField, Surface.RunExtent(11.0f)),
                                    Revision.EditRun, Covering(0xF2F2F4u), 11.0f);
                }
            }

            CursorAcross += Row.SpanAcross() + 4.0f;
            ++Presented;
        }
        CursorAcross += 12.0f;
    }
}

}   // namespace Slate
