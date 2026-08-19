//============================================================================================================================================
//                                                        RECORDINGSURFACE.CPP
//============================================================================================================================================
// 🧩 The one translation unit that addresses the vendored interface library — every recorded primitive lands in its draw list.

#include "SlateUI/Interface/PanelExchange/Api/PanelExchange.h"

#include "imgui.h"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>

namespace Slate
{
namespace Reference
{  
namespace
{

constexpr float _pi = 3.14159265358979323846f;   // [-] - circle constant

/// 🧩 Converts one ink into the vendored colour ordinate.
/// cost  ✔️
ImVec4 InkToVec4(const InkOrdinate& Ink)
{
    return ImVec4(Ink.Red / 255.0f, Ink.Green / 255.0f, Ink.Blue / 255.0f, Ink.Opacity / 255.0f);
}

/// 🧩 Converts the corner selection into the vendored rounding flags.
/// cost  ✔️
ImDrawFlags CornerFlags(CornerSelection Corners)
{
    switch (Corners)
    {
        case CornerSelection::None:          return ImDrawFlags_RoundCornersNone;
        case CornerSelection::Upper:         return ImDrawFlags_RoundCornersTop;
        case CornerSelection::Lower:         return ImDrawFlags_RoundCornersBottom;
        case CornerSelection::UpperLeading:  return ImDrawFlags_RoundCornersTopLeft;
        case CornerSelection::UpperTrailing: return ImDrawFlags_RoundCornersTopRight;
        case CornerSelection::LowerLeading:  return ImDrawFlags_RoundCornersBottomLeft;
        case CornerSelection::LowerTrailing: return ImDrawFlags_RoundCornersBottomRight;
        case CornerSelection::All:           break;
    }
    return ImDrawFlags_RoundCornersAll;
}

/// 🧩 Decodes one UTF-8 codepoint and advances the cursor.
/// out   Codepoint  [-]  the decoded codepoint; Cursor names the next byte
/// cost  ✔️
std::uint32_t DecodeCodepoint(const char* Run, std::int32_t& Cursor)
{
    const std::uint8_t Leading = static_cast<std::uint8_t>(Run[Cursor]);
    if ((Leading & 0x80u) == 0u)
    {
        Cursor += 1;
        return Leading;
    }
    if ((Leading & 0xE0u) == 0xC0u && Run[Cursor + 1] != '\0')
    {
        const std::uint32_t Codepoint = ((Leading & 0x1Fu) << 6) | (static_cast<std::uint8_t>(Run[Cursor + 1]) & 0x3Fu);
        Cursor += 2;
        return Codepoint;
    }
    if ((Leading & 0xF0u) == 0xE0u && Run[Cursor + 1] != '\0' && Run[Cursor + 2] != '\0')
    {
        const std::uint32_t Codepoint = ((Leading & 0x0Fu) << 12)
                                      | ((static_cast<std::uint8_t>(Run[Cursor + 1]) & 0x3Fu) << 6)
                                      |  (static_cast<std::uint8_t>(Run[Cursor + 2]) & 0x3Fu);
        Cursor += 3;
        return Codepoint;
    }
    Cursor += 1;
    return Leading;
}

/// 🧩 The drawn stand-in for a codepoint the default typeface does not seat.
/// out   true when the codepoint has a drawn stand-in; Advance carries its measured advance
/// cost  ✔️
bool DrawnStandIn(std::uint32_t Codepoint, float& Advance)
{
    switch (Codepoint)
    {
        case 0x00B7u:  Advance = 4.0f;  return true;   // · - middle dot
        case 0x00B0u:  Advance = 5.0f;  return true;   // ° - degree ring
        case 0x2014u:  Advance = 8.0f;  return true;   // — - em dash, drawn as a rule
        default:       return false;
    }
}

}   // namespace

//------------------------------------------------------------------------------------------------------------------------
//                                                       ADOPTION
//------------------------------------------------------------------------------------------------------------------------

Deliver<bool> PanelExchange::Adopt(ShellLayer Layer)
{
    ImGuiIO& VendorIO = ImGui::GetIO();
    if (VendorIO.Fonts == nullptr)
        return Deliver<bool>::Refuse({ RefusalReason::CapabilityAbsent, "no font atlas stands constructed" });

    Recording = (Layer == ShellLayer::Above) ? static_cast<void*>(ImGui::GetForegroundDrawList(ImGui::GetMainViewport()))
                                             : static_cast<void*>(ImGui::GetBackgroundDrawList(ImGui::GetMainViewport()));
    Standing  = true;

    Arrived.Along          = VendorIO.MousePos.x;
    Arrived.Across         = VendorIO.MousePos.y;
    Arrived.PrimaryDown    = VendorIO.MouseDown[0];
    Arrived.PrimaryPressed = ImGui::IsMouseClicked(0);
    Arrived.SecondaryDown  = VendorIO.MouseDown[1];

    return Deliver<bool>::Delivered(true);
}

void PanelExchange::Seal()
{
    Standing  = false;
    Recording = nullptr;
}

//------------------------------------------------------------------------------------------------------------------------
//                                                    GROUNDS AND EDGES
//------------------------------------------------------------------------------------------------------------------------

void PanelExchange::Ground(const PlaneExtent& Extent, const InkOrdinate& Ink, float Radius, CornerSelection Corners)
{
    if (!Standing)
        return;
    ImDrawList* List = static_cast<ImDrawList*>(Recording);
    List->AddRectFilled(ImVec2(Extent.LeastAlong, Extent.LeastAcross),
                        ImVec2(Extent.MostAlong, Extent.MostAcross),
                        ImGui::GetColorU32(InkToVec4(Ink)), Radius, CornerFlags(Corners));
}

void PanelExchange::Edge(const PlaneExtent& Extent, const InkOrdinate& Ink, float Thickness, float Radius, CornerSelection Corners)
{
    if (!Standing)
        return;
    ImDrawList* List = static_cast<ImDrawList*>(Recording);
    List->AddRect(ImVec2(Extent.LeastAlong + Thickness * 0.5f, Extent.LeastAcross + Thickness * 0.5f),
                  ImVec2(Extent.MostAlong - Thickness * 0.5f, Extent.MostAcross - Thickness * 0.5f),
                  ImGui::GetColorU32(InkToVec4(Ink)), Radius, CornerFlags(Corners), Thickness);
}

void PanelExchange::Scrim(const PlaneExtent& Extent, const InkOrdinate& Upper, const InkOrdinate& Lower)
{
    if (!Standing)
        return;
    ImDrawList* List = static_cast<ImDrawList*>(Recording);
    List->AddRectFilledMultiColor(ImVec2(Extent.LeastAlong, Extent.LeastAcross),
                                  ImVec2(Extent.MostAlong, Extent.MostAcross),
                                  ImGui::GetColorU32(InkToVec4(Upper)), ImGui::GetColorU32(InkToVec4(Upper)),
                                  ImGui::GetColorU32(InkToVec4(Lower)), ImGui::GetColorU32(InkToVec4(Lower)));
}

void PanelExchange::ScrimAlong(const PlaneExtent& Extent, const InkOrdinate& Leading, const InkOrdinate& Trailing)
{
    if (!Standing)
        return;
    ImDrawList* List = static_cast<ImDrawList*>(Recording);
    List->AddRectFilledMultiColor(ImVec2(Extent.LeastAlong, Extent.LeastAcross),
                                  ImVec2(Extent.MostAlong, Extent.MostAcross),
                                  ImGui::GetColorU32(InkToVec4(Leading)), ImGui::GetColorU32(InkToVec4(Trailing)),
                                  ImGui::GetColorU32(InkToVec4(Trailing)), ImGui::GetColorU32(InkToVec4(Leading)));
}

void PanelExchange::Medallion(float CentreAlong, float CentreAcross, float Radius, const InkOrdinate& Ink)
{
    if (!Standing)
        return;
    static_cast<ImDrawList*>(Recording)->AddCircleFilled(ImVec2(CentreAlong, CentreAcross), Radius,
                                                          ImGui::GetColorU32(InkToVec4(Ink)), 24);
}

void PanelExchange::Picture(const PlaneExtent& Extent, void* VendorIdentity, const InkOrdinate& Tint, float Radius, CornerSelection Corners)
{
    if (!Standing || VendorIdentity == nullptr)
        return;
    const ImTextureRef TexRef(static_cast<ImTextureID>(reinterpret_cast<std::uintptr_t>(VendorIdentity)));
    static_cast<ImDrawList*>(Recording)->AddImageRounded(TexRef,
                                                         ImVec2(Extent.LeastAlong, Extent.LeastAcross),
                                                         ImVec2(Extent.MostAlong, Extent.MostAcross),
                                                         ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f),
                                                         ImGui::GetColorU32(InkToVec4(Tint)), Radius, CornerFlags(Corners));
}

void PanelExchange::Ring(float CentreAlong, float CentreAcross, float Radius, float Thickness, const InkOrdinate& Ink)
{
    if (!Standing)
        return;
    static_cast<ImDrawList*>(Recording)->AddCircle(ImVec2(CentreAlong, CentreAcross), Radius,
                                                    ImGui::GetColorU32(InkToVec4(Ink)), 24, Thickness);
}

void PanelExchange::Stroke(float AlongA, float AcrossA, float AlongB, float AcrossB, float Thickness, const InkOrdinate& Ink)
{
    if (!Standing)
        return;
    static_cast<ImDrawList*>(Recording)->AddLine(ImVec2(AlongA, AcrossA), ImVec2(AlongB, AcrossB),
                                                  ImGui::GetColorU32(InkToVec4(Ink)), Thickness);
}

void PanelExchange::Rule(float Along, float Across, float ExtentAlong, float Thickness, const InkOrdinate& Ink)
{
    Stroke(Along, Across, Along + ExtentAlong, Across, Thickness, Ink);
}

//------------------------------------------------------------------------------------------------------------------------
//                                                       TEXT RUNS
//------------------------------------------------------------------------------------------------------------------------

namespace
{

/// 🧩 The vendored typeface whose crisp size lies nearest the requested presentation size.
/// cost  ✔️
ImFont* TypefaceFor(float Size)
{
    ImGuiIO&      VendorIO = ImGui::GetIO();
    ImFont*       Nearest  = VendorIO.Fonts->Fonts[0];
    float         NearestGap = 1.0e9f;
    for (ImFont* Standing : VendorIO.Fonts->Fonts)
    {
        const float Gap = (Size > Standing->LegacySize) ? Size - Standing->LegacySize : Standing->LegacySize - Size;
        if (Gap < NearestGap)
        {
            Nearest     = Standing;
            NearestGap  = Gap;
        }
    }
    return Nearest;
}

}   // namespace

void PanelExchange::TextRun(float Along, float Across, const char* Run, const InkOrdinate& Ink, float Size)
{
    if (!Standing || Run == nullptr || Run[0] == '\0')
        return;

    ImDrawList* List        = static_cast<ImDrawList*>(Recording);
    ImFont*     Face        = TypefaceFor(Size);
    const float PresentedSize = Face->LegacySize;   // 📝 the nearest baked size — lazy bakes never trigger
    const ImU32 Packed      = ImGui::GetColorU32(InkToVec4(Ink));
    float       Cursor      = Along;

    std::int32_t Scan = 0;
    while (Run[Scan] != '\0')
    {
        const std::int32_t SegmentStart = Scan;
        std::uint32_t      Codepoint    = 0;
        float              Advance      = 0.0f;
        while (Run[Scan] != '\0')
        {
            std::int32_t Probe = Scan;
            Codepoint = DecodeCodepoint(Run, Probe);
            if (DrawnStandIn(Codepoint, Advance))
                break;
            Scan = Probe;
        }

        if (Scan > SegmentStart)
        {
            const std::int32_t Count = Scan - SegmentStart;
            List->AddText(Face, PresentedSize, ImVec2(Cursor, Across), Packed, Run + SegmentStart, Run + SegmentStart + Count);
            Cursor += Face->CalcTextSizeA(PresentedSize, 1.0e9f, 0.0f, Run + SegmentStart, Run + SegmentStart + Count).x;
        }

        if (Run[Scan] == '\0')
            break;

        // ① The drawn stand-ins — middle dot, degree ring, em dash rule.
        const float Baseline = Across + PresentedSize * 0.78f;
        if (Codepoint == 0x00B7u)
            Medallion(Cursor + Advance * 0.5f, Baseline - PresentedSize * 0.30f, 1.1f, Ink);
        else if (Codepoint == 0x00B0u)
            Ring(Cursor + Advance * 0.5f, Baseline - PresentedSize * 0.62f, 1.4f, 1.0f, Ink);
        else if (Codepoint == 0x2014u)
            Rule(Cursor, Baseline - 1.0f, Advance, 1.0f, Ink);
        Cursor += Advance;

        Scan += (Codepoint >= 0x0800u) ? 3 : ((Codepoint >= 0x0080u) ? 2 : 1);
    }
}

void PanelExchange::TextRunClipped(float Along, float Across, const char* Run, const InkOrdinate& Ink, float Size, float ExtentAlong)
{
    if (!Standing || Run == nullptr)
        return;

    // ① Measure with stand-ins already folded in, then retreat whole stand-in boundaries until it fits.
    float Measured = MeasureRun(Run, Size);
    if (Measured <= ExtentAlong)
    {
        TextRun(Along, Across, Run, Ink, Size);
        return;
    }

    ImFont* Face = TypefaceFor(Size);
    const float PresentedSize = Face->LegacySize;
    char    Trimmed[256];
    std::int32_t Count = 0;
    while (Run[Count] != '\0' && Count < 255)
    {
        Trimmed[Count] = Run[Count];
        ++Count;
    }
    Trimmed[Count] = '\0';

    const float EllipsisExtent = Face->CalcTextSizeA(PresentedSize, 1.0e9f, 0.0f, "...").x;
    while (Count > 0 && MeasureRun(Trimmed, Size) + EllipsisExtent > ExtentAlong)
    {
        // ② Retreat past a whole UTF-8 codepoint each step.
        --Count;
        while (Count > 0 && (static_cast<std::uint8_t>(Trimmed[Count]) & 0xC0u) == 0x80u)
            --Count;
        Trimmed[Count] = '\0';
    }

    TextRun(Along, Across, Trimmed, Ink, Size);
    TextRun(Along + MeasureRun(Trimmed, Size), Across, "...", Ink, Size);
}

float PanelExchange::MeasureRun(const char* Run, float Size) const
{
    if (Run == nullptr)
        return 0.0f;

    ImFont* Face        = TypefaceFor(Size);
    const float PresentedSize = Face->LegacySize;
    float   Extent      = 0.0f;
    std::int32_t Scan = 0;
    while (Run[Scan] != '\0')
    {
        const std::int32_t SegmentStart = Scan;
        std::uint32_t      Codepoint    = 0;
        float              Advance      = 0.0f;
        while (Run[Scan] != '\0')
        {
            std::int32_t Probe = Scan;
            Codepoint = DecodeCodepoint(Run, Probe);
            if (DrawnStandIn(Codepoint, Advance))
                break;
            Scan = Probe;
        }

        if (Scan > SegmentStart)
            Extent += Face->CalcTextSizeA(PresentedSize, 1.0e9f, 0.0f, Run + SegmentStart, Run + SegmentStart + (Scan - SegmentStart)).x;

        if (Run[Scan] == '\0')
            break;

        Extent += Advance;
        Scan += (Codepoint >= 0x0800u) ? 3 : ((Codepoint >= 0x0080u) ? 2 : 1);
    }
    return Extent;
}

float PanelExchange::RunExtent(float Size) const
{
    return TypefaceFor(Size)->LegacySize;
}

float PanelExchange::CentredAlong(const PlaneExtent& Extent, const char* Run, float Size) const
{
    return Extent.LeastAlong + (Extent.SpanAlong() - MeasureRun(Run, Size)) * 0.5f;
}

//------------------------------------------------------------------------------------------------------------------------
//                                                    AFFORDANCE GLYPHS
//------------------------------------------------------------------------------------------------------------------------

void PanelExchange::Chevron(float CentreAlong, float CentreAcross, float Radius, const InkOrdinate& Ink, bool Down)
{
    // ① Down — the two strokes meet at the lower ordinate; right — they meet at the trailing ordinate.
    if (Down)
    {
        Stroke(CentreAlong - Radius, CentreAcross - Radius * 0.55f, CentreAlong, CentreAcross + Radius * 0.55f, 1.7f, Ink);
        Stroke(CentreAlong,          CentreAcross + Radius * 0.55f, CentreAlong + Radius, CentreAcross - Radius * 0.55f, 1.7f, Ink);
    }
    else
    {
        Stroke(CentreAlong - Radius * 0.55f, CentreAcross - Radius, CentreAlong + Radius * 0.55f, CentreAcross, 1.7f, Ink);
        Stroke(CentreAlong + Radius * 0.55f, CentreAcross,          CentreAlong - Radius * 0.55f, CentreAcross + Radius, 1.7f, Ink);
    }
}

void PanelExchange::EyeGlyph(float CentreAlong, float CentreAcross, float Radius, const InkOrdinate& Ink, bool Absent)
{
    if (!Standing)
        return;
    ImDrawList* List   = static_cast<ImDrawList*>(Recording);
    const ImU32 Packed = ImGui::GetColorU32(InkToVec4(Ink));

    const float Squash = 0.70f;
    const float LeadingEdge   = _pi * 1.05f;
    const float TrailingEdge  = _pi * 1.95f;

    if (Absent)
    {
        // ① One lowered lid plus the strike.
        List->PathClear();
        for (int Ordinal = 0; Ordinal <= 10; ++Ordinal)
        {
            const float θ = LeadingEdge + (TrailingEdge - LeadingEdge) * Ordinal / 10.0f;
            List->PathLineTo(ImVec2(CentreAlong + std::cos(θ) * Radius, CentreAcross + std::sin(θ) * Radius * Squash + Radius * 0.18f));
        }
        List->PathStroke(Packed, 0, 1.4f);
        Stroke(CentreAlong - Radius * 0.75f, CentreAcross - Radius * 0.35f, CentreAlong + Radius * 0.75f, CentreAcross + Radius * 0.60f, 1.4f, Ink);
        return;
    }

    // ① Upper lid.
    List->PathClear();
    for (int Ordinal = 0; Ordinal <= 12; ++Ordinal)
    {
        const float θ = _pi * 1.02f + _pi * 0.96f * Ordinal / 12.0f;
        List->PathLineTo(ImVec2(CentreAlong + std::cos(θ) * Radius, CentreAcross + std::sin(θ) * Radius * Squash));
    }
    List->PathStroke(Packed, 0, 1.4f);
    // ② Lower lid.
    List->PathClear();
    for (int Ordinal = 0; Ordinal <= 12; ++Ordinal)
    {
        const float θ = _pi * 0.98f - _pi * 0.96f * Ordinal / 12.0f;
        List->PathLineTo(ImVec2(CentreAlong + std::cos(θ) * Radius, CentreAcross + std::sin(θ) * Radius * Squash));
    }
    List->PathStroke(Packed, 0, 1.4f);
    // ③ Pupil.
    Medallion(CentreAlong, CentreAcross + Radius * 0.04f, Radius * 0.26f, Ink);
}

void PanelExchange::SearchGlyph(float CentreAlong, float CentreAcross, float Radius, const InkOrdinate& Ink)
{
    Ring(CentreAlong - Radius * 0.18f, CentreAcross - Radius * 0.18f, Radius * 0.62f, 1.5f, Ink);
    Stroke(CentreAlong + Radius * 0.34f, CentreAcross + Radius * 0.34f,
           CentreAlong + Radius * 0.78f, CentreAcross + Radius * 0.78f, 1.5f, Ink);
}

void PanelExchange::PlusGlyph(float CentreAlong, float CentreAcross, float Radius, const InkOrdinate& Ink)
{
    Stroke(CentreAlong - Radius, CentreAcross, CentreAlong + Radius, CentreAcross, 1.5f, Ink);
    Stroke(CentreAlong, CentreAcross - Radius, CentreAlong, CentreAcross + Radius, 1.5f, Ink);
}

void PanelExchange::CrossGlyph(float CentreAlong, float CentreAcross, float Radius, const InkOrdinate& Ink)
{
    Stroke(CentreAlong - Radius, CentreAcross - Radius, CentreAlong + Radius, CentreAcross + Radius, 1.5f, Ink);
    Stroke(CentreAlong + Radius, CentreAcross - Radius, CentreAlong - Radius, CentreAcross + Radius, 1.5f, Ink);
}

void PanelExchange::CheckGlyph(float CentreAlong, float CentreAcross, float Radius, const InkOrdinate& Ink)
{
    Stroke(CentreAlong - Radius, CentreAcross, CentreAlong - Radius * 0.25f, CentreAcross + Radius * 0.75f, 1.6f, Ink);
    Stroke(CentreAlong - Radius * 0.25f, CentreAcross + Radius * 0.75f, CentreAlong + Radius, CentreAcross - Radius * 0.6f, 1.6f, Ink);
}

void PanelExchange::TrashGlyph(float CentreAlong, float CentreAcross, float Radius, const InkOrdinate& Ink)
{
    const PlaneExtent Bin = Spanning(CentreAlong - Radius * 0.75f, CentreAcross - Radius * 0.45f, Radius * 1.5f, Radius * 1.35f);
    Edge(Bin, Ink, 1.3f, 1.5f, CornerSelection::Lower);
    Stroke(CentreAlong - Radius, CentreAcross - Radius * 0.55f, CentreAlong + Radius, CentreAcross - Radius * 0.55f, 1.3f, Ink);
    Stroke(CentreAlong - Radius * 0.25f, CentreAcross - Radius * 0.05f, CentreAlong - Radius * 0.25f, CentreAcross + Radius * 0.75f, 1.2f, Ink);
    Stroke(CentreAlong + Radius * 0.25f, CentreAcross - Radius * 0.05f, CentreAlong + Radius * 0.25f, CentreAcross + Radius * 0.75f, 1.2f, Ink);
}

}   // namespace Reference
}   // namespace Slate
