//============================================================================================================================================
//                                                       INTERFACESEQUENCE.CPP
//============================================================================================================================================
// 🧩 The one place a headless host's vendored context is addressed — adoption, pointer, tick, dismissal.

#include "SlateUI/Interface/InterfaceSequence/Api/InterfaceSequence.h"

#include "imgui.h"

namespace Slate
{

Deliver<bool> InterfaceSequence::Adopt(double DisplayAlong, double DisplayAcross)
{
    ImGui::CreateContext();
    ImGuiIO& VendorIO = ImGui::GetIO();
    VendorIO.DisplaySize = ImVec2(static_cast<float>(DisplayAlong), static_cast<float>(DisplayAcross));

    // ① The default typeface at three crisp sizes — no font files, no custom atlas.
    ImFontConfig BodyConfig;    BodyConfig.SizePixels    = 13.0f;
    ImFontConfig SmallConfig;   SmallConfig.SizePixels   = 11.0f;
    ImFontConfig CaptionConfig; CaptionConfig.SizePixels = 10.0f;
    VendorIO.Fonts->AddFontDefaultVector(&BodyConfig);
    VendorIO.Fonts->AddFontDefaultVector(&SmallConfig);
    VendorIO.Fonts->AddFontDefaultVector(&CaptionConfig);
    VendorIO.Fonts->Build();

    // ② The borderless host chrome — every surface is recorded through the seam, so the window itself is transparent.
    ImGuiStyle& VendorStyle = ImGui::GetStyle();
    VendorStyle.WindowRounding    = 0.0f;
    VendorStyle.WindowPadding     = ImVec2(0.0f, 0.0f);
    VendorStyle.WindowBorderSize  = 0.0f;
    VendorStyle.PopupRounding     = 9.0f;
    VendorStyle.PopupBorderSize   = 1.0f;
    VendorStyle.ScrollbarSize     = 0.0f;
    ImVec4* Colours = VendorStyle.Colors;
    Colours[ImGuiCol_WindowBg]   = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    Colours[ImGuiCol_ChildBg]    = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    Colours[ImGuiCol_PopupBg]    = ImVec4(0.059f, 0.059f, 0.067f, 0.98f);
    Colours[ImGuiCol_Border]     = ImVec4(1.0f, 1.0f, 1.0f, 0.10f);
    Colours[ImGuiCol_FrameBg]    = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

    return Deliver<bool>::Delivered(true);
}

void InterfaceSequence::SeatPointer(float Along, float Across)
{
    ImGui::GetIO().MousePos = ImVec2(Along, Across);
}

void InterfaceSequence::SeatPrimaryPress()
{
    ImGui::GetIO().AddMouseButtonEvent(0, true);
}

Deliver<bool> InterfaceSequence::OpenTick()
{
    ImGuiIO& VendorIO = ImGui::GetIO();
    ImGui::NewFrame();

    // ① One borderless window filling the display — the seat every panel records against.
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(VendorIO.DisplaySize);
    ImGui::Begin("RIFT \u2014 Panel Seat", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollbar);
    return Deliver<bool>::Delivered(true);
}

void* InterfaceSequence::SealTick()
{
    ImGui::End();
    ImGui::Render();
    return ImGui::GetDrawData();
}

void InterfaceSequence::Dismiss()
{
    ImGui::DestroyContext();
}

}   // namespace Slate
