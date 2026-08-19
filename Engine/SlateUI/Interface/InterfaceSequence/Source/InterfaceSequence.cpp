//============================================================================================================================================
//                                                       INTERFACESEQUENCE.CPP
//============================================================================================================================================
// 🧩 The one place a headless host's vendored context is addressed — adoption, pointer, tick, dismissal.

#include "SlateUI/Interface/InterfaceSequence/Api/InterfaceSequence.h"

#include "imgui.h"

#if defined(_WIN32)
#include <windows.h>
#endif

#include <cstdio>

namespace Slate
{
namespace Reference
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

Deliver<bool> InterfaceSequence::OpenSeatWindow(double DisplayAlong, double DisplayAcross)
{
    if (ImGui::GetCurrentContext() == nullptr)
        return Deliver<bool>::Refuse({ RefusalReason::CapabilityAbsent, "no vendored context stands" });

    // ① One borderless window filling the display — the seat every panel's widgets stand in.
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(ImVec2(static_cast<float>(DisplayAlong), static_cast<float>(DisplayAcross)));
    ImGui::Begin("Slate \u2014 Reference Seat", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollbar |
                 ImGuiWindowFlags_NoSavedSettings);
    return Deliver<bool>::Delivered(true);
}

void InterfaceSequence::CloseSeatWindow()
{
    if (ImGui::GetCurrentContext() == nullptr)
        return;
    ImGui::End();
}

Deliver<bool> InterfaceSequence::OpenTick()
{
    ImGuiIO& VendorIO = ImGui::GetIO();
    ImGui::NewFrame();
    return OpenSeatWindow(VendorIO.DisplaySize.x, VendorIO.DisplaySize.y);
}

void* InterfaceSequence::SealTick()
{
    ImGui::End();
    ImGui::Render();
    return ImGui::GetDrawData();
}


void InterfaceSequence::SeatFaultReporter()
{
#if defined(_WIN32)
    ::SetUnhandledExceptionFilter([](EXCEPTION_POINTERS* Pointers) -> LONG
    {
        std::fprintf(stderr, "[fault] exception 0x%08lX at %p — the stage named above is where it landed\n",
                     static_cast<unsigned long>(Pointers->ExceptionRecord->ExceptionCode),
                     Pointers->ExceptionRecord->ExceptionAddress);
        std::fflush(stderr);
        return EXCEPTION_EXECUTE_HANDLER;
    });
#endif
}

void InterfaceSequence::NameStage(const char* StageRun)
{
    std::fprintf(stderr, "[stage] %s\n", StageRun);
    std::fflush(stderr);
}

void InterfaceSequence::Dismiss()
{
    ImGui::DestroyContext();
}

}   // namespace Reference
}   // namespace Slate
