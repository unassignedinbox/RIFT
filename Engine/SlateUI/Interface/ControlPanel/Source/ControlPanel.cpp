//============================================================================================================================================
//                                                           CONTROLPANEL.CPP
//============================================================================================================================================
// 🧩 The reference control panel's widgets — capsules, switches, segments, dropdowns, sliders, vectors, colours, paths — presented on the recording seam.

#include "Engine/SlateUI/Interface/ControlPanel/Api/ControlPanel.h"

#include "imgui.h"

#include <cmath>
#include <cstdio>
#include <cstring>

namespace Slate
{

//------------------------------------------------------------------------------------------------------------------------
//                                                      SHEET GATHERING
//------------------------------------------------------------------------------------------------------------------------

ControlSheet ControlSheetFromWorkspace(const WorkspaceInk& Sheet)
{
    ControlSheet Gathered;
    Gathered.FieldSunken     = Sheet.FieldSunken;
    Gathered.FieldUnit       = Sheet.FieldUnit;
    Gathered.FieldFocus      = Sheet.FieldFocus;
    Gathered.TrackGround     = Sheet.TrackGround;
    Gathered.TrackFill       = Sheet.TrackFill;
    Gathered.KnobInk         = Sheet.KnobInk;
    Gathered.TileGround      = Sheet.TileGround;
    Gathered.TileRoused      = Sheet.TileRoused;
    Gathered.TileTaken       = Sheet.RailTaken;
    Gathered.HairEdge        = Sheet.HairEdge;
    Gathered.HairEdgeStrong  = Sheet.HairEdgeStrong;
    Gathered.InkPrimary      = Sheet.InkPrimary;
    Gathered.InkMuted        = Sheet.InkMuted;
    Gathered.InkFaint        = Sheet.InkFaint;
    Gathered.Accent          = Sheet.Accent;
    Gathered.OnAccent        = InkOrdinate{ 27u, 27u, 30u, 255u };
    Gathered.RowRoused       = Sheet.RowRoused;
    Gathered.RowExtent       = Sheet.RowExtent;
    return Gathered;
}

ControlSheet ControlSheetFromChannel(const ChannelInk& Sheet)
{
    ControlSheet Gathered;
    Gathered.FieldSunken     = Sheet.FieldSunken;
    Gathered.FieldUnit       = Sheet.FieldUnit;
    Gathered.FieldFocus      = Sheet.TileRoused;
    Gathered.TrackGround     = Sheet.TrackGround;
    Gathered.TrackFill       = Sheet.TrackFill;
    Gathered.KnobInk         = Sheet.KnobInk;
    Gathered.TileGround      = Sheet.TileGround;
    Gathered.TileRoused      = Sheet.TileRoused;
    Gathered.TileTaken       = Sheet.TileTaken;
    Gathered.HairEdge        = Sheet.HairEdge;
    Gathered.HairEdgeStrong  = Sheet.HairEdgeStrong;
    Gathered.InkPrimary      = Sheet.InkPrimary;
    Gathered.InkMuted        = Sheet.InkMuted;
    Gathered.InkFaint        = Sheet.InkFaint;
    Gathered.Accent          = Sheet.Accent;
    Gathered.OnAccent        = Sheet.OnAccent;
    Gathered.RowRoused       = Sheet.RowRoused;
    Gathered.RowExtent       = 32.0f;
    return Gathered;
}

ControlSheet ControlSheetFromCad(const CadInk& Sheet)
{
    ControlSheet Gathered;
    Gathered.FieldSunken     = Sheet.DeskGround;
    Gathered.FieldUnit       = Sheet.PanelLifted;
    Gathered.FieldFocus      = Sheet.RowRoused;
    Gathered.TrackGround     = Sheet.PanelLifted;
    Gathered.TrackFill       = Covering(0x5A5A5Au);
    Gathered.KnobInk         = Covering(0xFFFFFFu);
    Gathered.TileGround      = Sheet.PanelLifted;
    Gathered.TileRoused      = Sheet.RowRoused;
    Gathered.TileTaken       = Sheet.RowTakenGround;
    Gathered.HairEdge        = Sheet.HairEdge;
    Gathered.HairEdgeStrong  = Sheet.RowTakenEdge;
    Gathered.InkPrimary      = Sheet.InkPrimary;
    Gathered.InkMuted        = Sheet.InkMuted;
    Gathered.InkFaint        = Sheet.InkFaint;
    Gathered.Accent          = Sheet.Accent;
    Gathered.OnAccent        = InkOrdinate{ 0u, 0u, 0u, 255u };
    Gathered.RowRoused       = Sheet.RowRoused;
    Gathered.RowExtent       = 32.0f;
    return Gathered;
}

namespace
{

/// 🧩 Presents one numeral capsule — black ground, right-aligned numeral, unit segment.
/// tag   internal
void PresentCapsule(RecordingSurface& Surface, const PlaneExtent& Seat, const char* NumeralRun, const char* UnitRun,
                    const ControlSheet& Sheet, bool Focused)
{
    Surface.Ground(Seat, Sheet.FieldSunken, 12.0f);
    const float UnitAlong = 30.0f;
    const PlaneExtent UnitSeat = Spanning(Seat.MostAlong - UnitAlong, Seat.LeastAcross, UnitAlong, Seat.SpanAcross());
    // ① The trailing unit segment rounds both trailing corners — one pass per corner.
    Surface.Ground(UnitSeat, Focused ? Sheet.FieldFocus : Sheet.FieldUnit, 12.0f, CornerSelection::UpperTrailing);
    Surface.Ground(UnitSeat, Focused ? Sheet.FieldFocus : Sheet.FieldUnit, 12.0f, CornerSelection::LowerTrailing);
    Surface.Ground(Spanning(UnitSeat.LeastAlong, UnitSeat.LeastAcross, UnitSeat.SpanAlong() * 0.5f, UnitSeat.SpanAcross()),
                   Focused ? Sheet.FieldFocus : Sheet.FieldUnit, 0.0f);

    const float NumeralSize = 12.5f;
    const float NumeralExtent = Surface.MeasureRun(NumeralRun, NumeralSize);
    Surface.TextRun(Seat.MostAlong - UnitAlong - NumeralExtent - 8.0f,
                    CentredAcross(Seat, Surface.RunExtent(NumeralSize)), NumeralRun, Sheet.InkPrimary, NumeralSize);
    const float UnitSize = 11.0f;
    const float UnitExtent = Surface.MeasureRun(UnitRun, UnitSize);
    Surface.TextRun(UnitSeat.LeastAlong + (UnitSeat.SpanAlong() - UnitExtent) * 0.5f,
                    CentredAcross(UnitSeat, Surface.RunExtent(UnitSize)), UnitRun, Sheet.InkMuted, UnitSize);
}

/// 🧩 Presents one track with fill and knob at the declared fraction.
/// tag   internal
void PresentTrack(RecordingSurface& Surface, const PlaneExtent& Track, double Fraction, const ControlSheet& Sheet, bool Roused)
{
    Surface.Ground(Track, Sheet.TrackGround, Track.SpanAcross() * 0.5f);
    Surface.Ground(Spanning(Track.LeastAlong, Track.LeastAcross, Track.SpanAlong() * static_cast<float>(Fraction), Track.SpanAcross()),
                   Sheet.TrackFill, Track.SpanAcross() * 0.5f);
    const float KnobExtent = Track.SpanAcross() + 2.0f;
    const float KnobAlong  = Track.LeastAlong + Track.SpanAlong() * static_cast<float>(Fraction);
    const float KnobAcross = Track.LeastAcross + Track.SpanAcross() * 0.5f;
    const InkOrdinate KnobRing = Roused ? Sheet.Accent : InkOrdinate{ 0u, 0u, 0u, 89u };
    Surface.Medallion(KnobAlong, KnobAcross, KnobExtent * 0.5f + 3.0f, KnobRing);
    Surface.Medallion(KnobAlong, KnobAcross, KnobExtent * 0.5f, Sheet.KnobInk);
}

/// 🧩 Declares one invisible interaction seat and reports its occupancy.
/// out   Clicked  [-]  the primary pressed edge inside the seat this tick
/// tag   internal
bool PresentSeat(const PlaneExtent& Seat, const char* PushIdentity, bool& Held, bool& Roused)
{
    ImGui::PushID(PushIdentity);
    ImGui::SetCursorScreenPos(ImVec2(Seat.LeastAlong, Seat.LeastAcross));
    ImGui::InvisibleButton("seat", ImVec2(Seat.SpanAlong(), Seat.SpanAcross()));
    const bool Clicked = ImGui::IsItemClicked();
    Held               = ImGui::IsItemActive();
    Roused             = ImGui::IsItemHovered();
    ImGui::PopID();
    return Clicked;
}

/// 🧩 Formats one numeral at the declared figure count.
/// tag   internal
void FormatNumeral(char (&Seat)[32], double Amount, std::uint32_t Figures)
{
    if (Figures == 0u)
        std::snprintf(Seat, sizeof Seat, "%.0f", Amount);
    else if (Figures == 1u)
        std::snprintf(Seat, sizeof Seat, "%.1f", Amount);
    else
        std::snprintf(Seat, sizeof Seat, "%.2f", Amount);
}

}   // namespace

//------------------------------------------------------------------------------------------------------------------------
//                                                   THE RETENTION FIELD
//------------------------------------------------------------------------------------------------------------------------

bool PresentRetentionField(RecordingSurface& Surface, const PlaneExtent& Seat, char* Run, std::uint32_t RunCapacity,
                           const char* Placeholder, const InkOrdinate& FieldGround, const InkOrdinate& FieldEdge,
                           const InkOrdinate& RunInk, const InkOrdinate& VacantInk)
{
    Surface.Ground(Seat, FieldGround, 6.0f);
    Surface.Edge(Seat, FieldEdge, 1.0f, 6.0f);

    Surface.SearchGlyph(Seat.LeastAlong + 14.0f, Seat.LeastAcross + Seat.SpanAcross() * 0.5f, 6.0f, VacantInk);

    // ① The entry itself — transparent over the drawn pill, default typeface.
    ImGui::PushID("retention");
    ImGui::SetCursorScreenPos(ImVec2(Seat.LeastAlong + 28.0f, Seat.LeastAcross + (Seat.SpanAcross() - 19.0f) * 0.5f));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, IM_COL32(0, 0, 0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 3.0f));
    const bool Vacant = Run[0] == '\0';
    if (Vacant)
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(VacantInk.Red, VacantInk.Green, VacantInk.Blue, VacantInk.Opacity));
    ImGui::SetNextItemWidth(Seat.MostAlong - Seat.LeastAlong - 38.0f);
    const char* VacatedRun = Vacant ? Placeholder : nullptr;
    const bool Held = ImGui::InputTextWithHint("##entry", VacatedRun, Run, RunCapacity, ImGuiInputTextFlags_None);
    if (Vacant)
        ImGui::PopStyleColor();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);
    ImGui::PopID();

    if (!Vacant && ImGui::IsItemFocused())
        Surface.Edge(Seat, FieldEdge, 1.0f, 6.0f);
    return ImGui::IsItemFocused() || (Held && ImGui::IsItemActive());
}

//------------------------------------------------------------------------------------------------------------------------
//                                                    THE SWITCH ROW
//------------------------------------------------------------------------------------------------------------------------

void PresentSwitchRow(RecordingSurface& Surface, const PlaneExtent& Row, const ControlRowDeclaration& Declared,
                      bool& Taken, const ControlSheet& Sheet, const char* PushIdentity)
{
    Surface.TextRun(Row.LeastAlong, CentredAcross(Row, Surface.RunExtent(Declared.CaptionSize)),
                    Declared.Caption, Sheet.InkMuted, Declared.CaptionSize);

    const PlaneExtent Track = Spanning(Row.MostAlong - 50.0f, CentredAcross(Row, 32.0f), 50.0f, 32.0f);
    bool Held = false, Roused = false;
    if (PresentSeat(Track, PushIdentity, Held, Roused) && Held)
        Taken = !Taken;

    Surface.Ground(Track, Taken ? Sheet.TrackFill : Sheet.FieldSunken, 16.0f);
    const float NubAcross = Track.SpanAcross() - 8.0f;
    const float NubAlong  = Taken ? Track.MostAlong - 4.0f - NubAcross : Track.LeastAlong + 4.0f;
    Surface.Medallion(NubAlong + NubAcross * 0.5f, Track.LeastAcross + Track.SpanAcross() * 0.5f, NubAcross * 0.5f,
                      InkOrdinate{ 255u, 255u, 255u, 255u });
}

//------------------------------------------------------------------------------------------------------------------------
//                                                   THE SEGMENT ROW
//------------------------------------------------------------------------------------------------------------------------

void PresentSegmentRow(RecordingSurface& Surface, const PlaneExtent& Row, const ControlRowDeclaration& Declared,
                       const char* const* Captions, std::uint32_t CaptionCount, std::uint32_t& Taken,
                       const ControlSheet& Sheet, const char* PushIdentity)
{
    Surface.TextRun(Row.LeastAlong, CentredAcross(Row, Surface.RunExtent(Declared.CaptionSize)),
                    Declared.Caption, Sheet.InkMuted, Declared.CaptionSize);

    float Leading = Row.LeastAlong + Declared.CaptionExtent + 10.0f;
    for (std::uint32_t Ordinal = 0u; Ordinal < CaptionCount; ++Ordinal)
    {
        const float Size  = 12.0f;
        const float Width = Surface.MeasureRun(Captions[Ordinal], Size) + 24.0f;
        const PlaneExtent Pill = Spanning(Leading, CentredAcross(Row, 32.0f), Width, 32.0f);

        bool Held = false, Roused = false;
        char Identity[48];
        std::snprintf(Identity, sizeof Identity, "%s.%u", PushIdentity, Ordinal);
        if (PresentSeat(Pill, Identity, Held, Roused))
            Taken = Ordinal;

        Surface.Ground(Pill, Taken == Ordinal ? Sheet.KnobInk : Sheet.TileGround, 9.0f);
        Surface.TextRun(Surface.CentredAlong(Pill, Captions[Ordinal], Size), CentredAcross(Pill, Surface.RunExtent(Size)),
                        Captions[Ordinal], Taken == Ordinal ? Sheet.OnAccent : Sheet.InkMuted, Size);
        Leading += Width + 6.0f;
    }
}

//------------------------------------------------------------------------------------------------------------------------
//                                                  THE DROPDOWN ROW
//------------------------------------------------------------------------------------------------------------------------

void PresentDropdownRow(RecordingSurface& Surface, const PlaneExtent& Row, const ControlRowDeclaration& Declared,
                        const char* const* Captions, std::uint32_t CaptionCount, std::uint32_t& Taken,
                        const ControlSheet& Sheet, const char* PushIdentity)
{
    Surface.TextRun(Row.LeastAlong, CentredAcross(Row, Surface.RunExtent(Declared.CaptionSize)),
                    Declared.Caption, Sheet.InkMuted, Declared.CaptionSize);

    const PlaneExtent Head = Spanning(Row.LeastAlong + Declared.CaptionExtent + 10.0f,
                                      CentredAcross(Row, 26.0f), (Row.MostAlong - Row.LeastAlong) - Declared.CaptionExtent - 10.0f, 26.0f);
    bool Held = false, Roused = false;
    const bool Clicked = PresentSeat(Head, PushIdentity, Held, Roused);

    // ① Pill head — leading corners rounded, one pass per corner.
    Surface.Ground(Head, Sheet.FieldSunken, 13.0f, CornerSelection::UpperLeading);
    Surface.Ground(Head, Sheet.FieldSunken, 13.0f, CornerSelection::LowerLeading);
    Surface.Ground(Spanning(Head.LeastAlong + 13.0f, Head.LeastAcross, Head.SpanAlong() - 26.0f, Head.SpanAcross()),
                   Sheet.FieldSunken, 0.0f);
    const PlaneExtent CaretSeat = Spanning(Head.MostAlong - 26.0f, Head.LeastAcross, 26.0f, Head.SpanAcross());
    Surface.Ground(CaretSeat, Roused ? Sheet.FieldFocus : Sheet.FieldUnit, 13.0f, CornerSelection::UpperTrailing);
    Surface.Ground(CaretSeat, Roused ? Sheet.FieldFocus : Sheet.FieldUnit, 13.0f, CornerSelection::LowerTrailing);
    Surface.Ground(Spanning(CaretSeat.LeastAlong, CaretSeat.LeastAcross, CaretSeat.SpanAlong() * 0.5f, CaretSeat.SpanAcross()),
                   Roused ? Sheet.FieldFocus : Sheet.FieldUnit, 0.0f);

    const float Size = 11.0f;
    Surface.TextRunClipped(Head.LeastAlong + 11.0f, CentredAcross(Head, Surface.RunExtent(Size)),
                           Captions[Taken < CaptionCount ? Taken : 0u], Sheet.InkPrimary, Size, Head.SpanAlong() - 44.0f);
    Surface.Chevron(CaretSeat.LeastAlong + CaretSeat.SpanAlong() * 0.5f, CaretSeat.LeastAcross + CaretSeat.SpanAcross() * 0.5f - 1.0f,
                    3.0f, Sheet.InkMuted, true);

    ImGui::PushID(PushIdentity);
    if (Clicked)
        ImGui::OpenPopup("list");
    if (ImGui::BeginPopup("list"))
    {
        ImGui::PushStyleColor(ImGuiCol_PopupBg, IM_COL32(Sheet.FieldSunken.Red, Sheet.FieldSunken.Green, Sheet.FieldSunken.Blue, 250));
        for (std::uint32_t Ordinal = 0u; Ordinal < CaptionCount; ++Ordinal)
        {
            if (ImGui::Selectable(Captions[Ordinal], Ordinal == Taken, ImGuiSelectableFlags_None, ImVec2(0.0f, 22.0f)))
                Taken = Ordinal;
            if (Ordinal == Taken)
            {
                const ImVec2 After = ImGui::GetItemRectMin();
                Surface.Ground(Spanning(After.x - 4.0f, After.y, 3.0f, 22.0f), Sheet.Accent, 1.0f);
            }
        }
        ImGui::PopStyleColor();
        ImGui::EndPopup();
    }
    ImGui::PopID();
}

//------------------------------------------------------------------------------------------------------------------------
//                                                   THE VALUE SLIDER ROW
//------------------------------------------------------------------------------------------------------------------------

void PresentSliderRow(RecordingSurface& Surface, const PlaneExtent& Row, const ControlRowDeclaration& Declared,
                      const SliderDeclaration& Range, double& Amount, const ControlSheet& Sheet, const char* PushIdentity)
{
    Surface.TextRun(Row.LeastAlong, CentredAcross(Row, Surface.RunExtent(Declared.CaptionSize)),
                    Declared.Caption, Sheet.InkMuted, Declared.CaptionSize);

    const double Span = Range.Maximum - Range.Minimum;
    double Fraction = Span > 0.0 ? (Amount - Range.Minimum) / Span : 0.0;
    if (Fraction < 0.0) Fraction = 0.0;
    if (Fraction > 1.0) Fraction = 1.0;

    const PlaneExtent Capsule = Spanning(Row.LeastAlong + Declared.CaptionExtent + 10.0f, CentredAcross(Row, 32.0f),
                                         Range.NumeralExtent, 32.0f);
    const PlaneExtent Track   = Spanning(Capsule.MostAlong + 7.0f, CentredAcross(Row, 19.0f),
                                         Row.MostAlong - Capsule.MostAlong - 7.0f, 19.0f);

    bool Held = false, Roused = false;
    bool TrackRoused = false;
    if (PresentSeat(Track, PushIdentity, Held, TrackRoused) || (Held && TrackRoused))
    {
        if (ImGui::IsItemActive())
        {
            const ImGuiIO& VendorIO = ImGui::GetIO();
            const float Local = (VendorIO.MousePos.x - Track.LeastAlong) / Track.SpanAlong();
            const float Confined = Local < 0.0f ? 0.0f : (Local > 1.0f ? 1.0f : Local);
            Amount = Range.Minimum + Confined * Span;
            Fraction = Confined;
        }
    }
    bool CapsuleHeld = false, CapsuleRoused = false;
    if (PresentSeat(Capsule, PushIdentity, CapsuleHeld, CapsuleRoused))
    {
        // ① Capsule drag steps the amount, the reference's ScalarEntry gesture.
        ImGuiIO& VendorIO = ImGui::GetIO();
        Amount += VendorIO.MouseDelta.x * (Range.Figures == 0u ? 1.0 : 0.01);
        if (Amount < Range.Minimum) Amount = Range.Minimum;
        if (Amount > Range.Maximum) Amount = Range.Maximum;
        Fraction = Span > 0.0 ? (Amount - Range.Minimum) / Span : 0.0;
    }

    char Numeral[32];
    FormatNumeral(Numeral, Amount, Range.Figures);
    PresentCapsule(Surface, Capsule, Numeral, Range.Unit, Sheet, CapsuleRoused);
    PresentTrack(Surface, Track, Fraction, Sheet, TrackRoused);
}

//------------------------------------------------------------------------------------------------------------------------
//                                                    THE VECTOR ROW
//------------------------------------------------------------------------------------------------------------------------

void PresentVectorRow(RecordingSurface& Surface, const PlaneExtent& Row, const ControlRowDeclaration& Declared,
                      double Ordinates[3], double Step, const ControlSheet& Sheet, const char* PushIdentity)
{
    Surface.TextRun(Row.LeastAlong, CentredAcross(Row, Surface.RunExtent(Declared.CaptionSize)),
                    Declared.Caption, Sheet.InkMuted, Declared.CaptionSize);

    static const char* const Axes[3] = { "X", "Y", "Z" };
    const float SpanAvailable = (Row.MostAlong - Row.LeastAlong) - Declared.CaptionExtent - 10.0f;
    const float CapsuleExtent = (SpanAvailable - 2.0f * 4.0f) / 3.0f;

    for (std::uint32_t Axis = 0u; Axis < 3u; ++Axis)
    {
        const PlaneExtent Capsule = Spanning(Row.LeastAlong + Declared.CaptionExtent + 10.0f + Axis * (CapsuleExtent + 4.0f),
                                             CentredAcross(Row, 32.0f), CapsuleExtent, 32.0f);
        bool Held = false, Roused = false;
        char Identity[48];
        std::snprintf(Identity, sizeof Identity, "%s.axis%u", PushIdentity, Axis);
        if (PresentSeat(Capsule, Identity, Held, Roused) && ImGui::IsItemActive())
        {
            ImGuiIO& VendorIO = ImGui::GetIO();
            Ordinates[Axis] += VendorIO.MouseDelta.x * Step;
        }

        Surface.Ground(Capsule, Sheet.FieldSunken, 12.0f);
        const PlaneExtent AxisSeat = Spanning(Capsule.LeastAlong, Capsule.LeastAcross, 24.0f, Capsule.SpanAcross());
        Surface.Ground(AxisSeat, Sheet.FieldUnit, 12.0f, CornerSelection::UpperLeading);
        Surface.Ground(AxisSeat, Sheet.FieldUnit, 0.0f);
        Surface.TextRun(Surface.CentredAlong(AxisSeat, Axes[Axis], 11.0f), CentredAcross(AxisSeat, Surface.RunExtent(11.0f)),
                        Axes[Axis], Sheet.InkMuted, 11.0f);

        char Numeral[32];
        FormatNumeral(Numeral, Ordinates[Axis], 2u);
        const float Size = 12.0f;
        Surface.TextRun(Capsule.MostAlong - Surface.MeasureRun(Numeral, Size) - 8.0f,
                        CentredAcross(Capsule, Surface.RunExtent(Size)), Numeral, Sheet.InkPrimary, Size);
    }
}

//------------------------------------------------------------------------------------------------------------------------
//                                                    THE COLOUR ROW
//------------------------------------------------------------------------------------------------------------------------

void PresentColourRow(RecordingSurface& Surface, const PlaneExtent& Row, const ControlRowDeclaration& Declared,
                      std::uint8_t Ordinates[4], bool& SeatedOpen, const ControlSheet& Sheet, const char* PushIdentity)
{
    Surface.TextRun(Row.LeastAlong, CentredAcross(Row, Surface.RunExtent(Declared.CaptionSize)),
                    Declared.Caption, Sheet.InkMuted, Declared.CaptionSize);

    const PlaneExtent Pill = Spanning(Row.LeastAlong + Declared.CaptionExtent + 10.0f, CentredAcross(Row, 36.0f),
                                      Row.MostAlong - Row.LeastAlong - Declared.CaptionExtent - 10.0f, 36.0f);
    bool Held = false, Roused = false;
    if (PresentSeat(Pill, PushIdentity, Held, Roused))
        SeatedOpen = !SeatedOpen;

    Surface.Ground(Pill, Sheet.FieldSunken, 18.0f);
    const InkOrdinate Swatch = InkOrdinate{ Ordinates[0], Ordinates[1], Ordinates[2], Ordinates[3] };
    Surface.Medallion(Pill.LeastAlong + 20.0f, Pill.LeastAcross + Pill.SpanAcross() * 0.5f, 9.0f, Swatch);

    char OrdinalRun[64];
    const double Alpha = Ordinates[3] / 255.0;
    std::snprintf(OrdinalRun, sizeof OrdinalRun, "%u, %u, %u  %.2f", Ordinates[0], Ordinates[1], Ordinates[2], Alpha);
    Surface.TextRun(Pill.LeastAlong + 38.0f, CentredAcross(Pill, Surface.RunExtent(12.0f)), OrdinalRun, Sheet.InkPrimary, 12.0f);

    const PlaneExtent CaretSeat = Spanning(Pill.MostAlong - 40.0f, Pill.LeastAcross, 40.0f, Pill.SpanAcross());
    Surface.Ground(CaretSeat, Sheet.FieldUnit, 18.0f, CornerSelection::UpperTrailing);
    Surface.Ground(CaretSeat, Sheet.FieldUnit, 0.0f);
    Surface.Chevron(CaretSeat.LeastAlong + 20.0f, CaretSeat.LeastAcross + CaretSeat.SpanAcross() * 0.5f - 1.0f,
                    3.0f, Sheet.InkMuted, SeatedOpen ? false : true);

    if (!SeatedOpen)
        return;

    // ① The picker — saturation-value field, hue rail, alpha rail, hexadecimal run.
    const PlaneExtent Picker = Spanning(Pill.LeastAlong, Pill.MostAcross + 6.0f, Pill.SpanAlong(), 120.0f + 12.0f + 12.0f + 34.0f);
    Surface.Ground(Picker, Sheet.TileGround, 12.0f);
    const PlaneExtent Field = Spanning(Picker.LeastAlong + 12.0f, Picker.LeastAcross + 12.0f, Picker.SpanAlong() - 24.0f, 120.0f);
    Surface.Ground(Field, Swatch, 8.0f);
    Surface.ScrimAlong(Field, InkOrdinate{ 255u, 255u, 255u, 255u }, InkOrdinate{ Ordinates[0], Ordinates[1], Ordinates[2], 255u });
    Surface.Scrim(Field, InkOrdinate{ 0u, 0u, 0u, 0u }, InkOrdinate{ 0u, 0u, 0u, 255u });
    Surface.Medallion(Field.LeastAlong + Field.SpanAlong() * 0.5f, Field.LeastAcross + Field.SpanAcross() * 0.5f, 7.0f,
                      InkOrdinate{ 255u, 255u, 255u, 255u });

    const PlaneExtent HueRail = Spanning(Field.LeastAlong, Field.MostAcross + 8.0f, Field.SpanAlong(), 12.0f);
    Surface.Ground(HueRail, Covering(0xFF0000u), 6.0f);
    Surface.Ground(Spanning(HueRail.LeastAlong + HueRail.SpanAlong() * 0.17f, HueRail.LeastAcross, HueRail.SpanAlong() * 0.17f, 12.0f), Covering(0xFFFF00u), 0.0f);
    Surface.Ground(Spanning(HueRail.LeastAlong + HueRail.SpanAlong() * 0.34f, HueRail.LeastAcross, HueRail.SpanAlong() * 0.17f, 12.0f), Covering(0x00FF00u), 0.0f);
    Surface.Ground(Spanning(HueRail.LeastAlong + HueRail.SpanAlong() * 0.51f, HueRail.LeastAcross, HueRail.SpanAlong() * 0.16f, 12.0f), Covering(0x00FFFFu), 0.0f);
    Surface.Ground(Spanning(HueRail.LeastAlong + HueRail.SpanAlong() * 0.67f, HueRail.LeastAcross, HueRail.SpanAlong() * 0.17f, 12.0f), Covering(0x0000FFu), 0.0f);
    Surface.Ground(Spanning(HueRail.LeastAlong + HueRail.SpanAlong() * 0.84f, HueRail.LeastAcross, HueRail.SpanAlong() * 0.16f, 12.0f), Covering(0xFF00FFu), 0.0f);
    Surface.Medallion(HueRail.LeastAlong + HueRail.SpanAlong() * 0.5f, HueRail.LeastAcross + 6.0f, 8.0f, Sheet.KnobInk);

    const PlaneExtent AlphaRail = Spanning(Field.LeastAlong, HueRail.MostAcross + 8.0f, Field.SpanAlong(), 12.0f);
    Surface.Ground(AlphaRail, Sheet.FieldUnit, 6.0f);
    Surface.ScrimAlong(AlphaRail, InkOrdinate{ Ordinates[0], Ordinates[1], Ordinates[2], 0u },
                       InkOrdinate{ Ordinates[0], Ordinates[1], Ordinates[2], 255u });
    Surface.Medallion(AlphaRail.LeastAlong + AlphaRail.SpanAlong() * static_cast<float>(Alpha), AlphaRail.LeastAcross + 6.0f, 8.0f, Sheet.KnobInk);

    char HexRun[16];
    std::snprintf(HexRun, sizeof HexRun, "#%02X%02X%02X", Ordinates[0], Ordinates[1], Ordinates[2]);
    Surface.TextRun(Picker.LeastAlong + 12.0f, AlphaRail.MostAcross + 8.0f, HexRun, Sheet.InkPrimary, 11.0f);
}

//------------------------------------------------------------------------------------------------------------------------
//                                                     THE TEXT ROW
//------------------------------------------------------------------------------------------------------------------------

void PresentTextRow(RecordingSurface& Surface, const PlaneExtent& Row, const ControlRowDeclaration& Declared,
                    char* Run, std::uint32_t RunCapacity, const ControlSheet& Sheet, const char* PushIdentity)
{
    Surface.TextRun(Row.LeastAlong, CentredAcross(Row, Surface.RunExtent(Declared.CaptionSize)),
                    Declared.Caption, Sheet.InkMuted, Declared.CaptionSize);

    const PlaneExtent Value = Spanning(Row.LeastAlong + Declared.CaptionExtent + 10.0f, CentredAcross(Row, 32.0f),
                                       Row.MostAlong - Row.LeastAlong - Declared.CaptionExtent - 10.0f - 40.0f, 32.0f);
    Surface.Ground(Value, Sheet.FieldSunken, 16.0f);

    ImGui::PushID(PushIdentity);
    ImGui::SetCursorScreenPos(ImVec2(Value.LeastAlong + 14.0f, Value.LeastAcross + (Value.SpanAcross() - 19.0f) * 0.5f));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, IM_COL32(0, 0, 0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 3.0f));
    ImGui::SetNextItemWidth(Value.SpanAlong() - 28.0f);
    ImGui::InputText("##run", Run, RunCapacity, ImGuiInputTextFlags_None);
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);
    ImGui::PopID();

    const PlaneExtent Browse = Spanning(Row.MostAlong - 32.0f, CentredAcross(Row, 32.0f), 32.0f, 32.0f);
    bool Held = false, Roused = false;
    PresentSeat(Browse, PushIdentity, Held, Roused);
    Surface.Medallion(Browse.LeastAlong + 16.0f, Browse.LeastAcross + 16.0f, 16.0f, Roused ? Sheet.FieldFocus : Sheet.FieldUnit);
    Surface.TextRun(Surface.CentredAlong(Browse, "...", 16.0f), CentredAcross(Browse, Surface.RunExtent(16.0f)), "...", Sheet.InkPrimary, 16.0f);
}

}   // namespace Slate
