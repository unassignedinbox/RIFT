//============================================================================================================================================
//                                                           OUTLINERHOST.CPP
//============================================================================================================================================
// 🧩 The headless standalone outliner — scripted states, software raster, marked raw dumps for the proof encoder.

#include "Engine/Application/OutlinerHost/Api/WorldEditorSeat.h"
#include "Engine/SlateUI/Interface/IconDepot/Api/IconDepot.h"
#include "Engine/SlateUI/Interface/OutlinerPanel/Api/OutlinerPanel.h"
#include "Engine/SlateUI/Interface/RasterCodec/Api/RasterCodec.h"
#include "Engine/SlateUI/Interface/RecordingSurface/Api/RecordingSurface.h"

#include "imgui.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

//------------------------------------------------------------------------------------------------------------------------
//                                                          FIGURES
//------------------------------------------------------------------------------------------------------------------------

namespace
{

constexpr std::uint32_t DisplayAlong  = 1500u;   // [px]
constexpr std::uint32_t DisplayAcross = 860u;    // [px]

/// 🧩 One scripted host state a proof shot seats.
/// tag   internal
struct HostState
{
    const char*  ShotRun;         // [-] - the dump name
    const char*  TakenIdentity;   // [-] - the outliner's taken row
    const char*  RetentionRun;    // [-] - the retention run seated in the field
    bool         SlideOpen;       // [-] - whether the inspector slide presents
};

}   // namespace

//------------------------------------------------------------------------------------------------------------------------
//                                              THE SHARED CONTEXT CONSTRUCTION
//------------------------------------------------------------------------------------------------------------------------

namespace Rift
{

/// 🧩 Constructs the context, the default typeface at three crisp sizes, and the styled window chrome.
/// tag   internal
void ConstructInterfaceContext()
{
    ImGui::CreateContext();
    ImGuiIO& VendorIO = ImGui::GetIO();

    ImFontConfig BodyConfig;    BodyConfig.SizePixels    = 13.0f;
    ImFontConfig SmallConfig;   SmallConfig.SizePixels   = 11.0f;
    ImFontConfig CaptionConfig; CaptionConfig.SizePixels = 10.0f;
    VendorIO.Fonts->AddFontDefaultVector(&BodyConfig);
    VendorIO.Fonts->AddFontDefaultVector(&SmallConfig);
    VendorIO.Fonts->AddFontDefaultVector(&CaptionConfig);
    VendorIO.Fonts->Build();

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
    Colours[ImGuiCol_PopupBg]    = ImVec4(0.063f, 0.063f, 0.071f, 0.98f);
    Colours[ImGuiCol_Border]     = ImVec4(1.0f, 1.0f, 1.0f, 0.10f);
    Colours[ImGuiCol_FrameBg]    = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
}

}   // namespace Rift

//------------------------------------------------------------------------------------------------------------------------
//                                                          ENTRY
//------------------------------------------------------------------------------------------------------------------------

int main(int ArgumentCount, char** Arguments)
{
    const char* DumpPrefix = "Build/Shots/outliner";
    for (int Ordinal = 1; Ordinal < ArgumentCount; ++Ordinal)
        if (std::strcmp(Arguments[Ordinal], "--prefix") == 0 && Ordinal + 1 < ArgumentCount)
            DumpPrefix = Arguments[++Ordinal];

    using namespace Slate;
    using namespace Rift;

    ConstructInterfaceContext();
    ImGuiIO& VendorIO = ImGui::GetIO();
    VendorIO.DisplaySize = ImVec2(static_cast<float>(DisplayAlong), static_cast<float>(DisplayAcross));

    void* AtlasIdentity = reinterpret_cast<void*>(static_cast<std::uintptr_t>(0x1u));
    RasterCodec Codec;
    if (!Codec.SeatAtlas(AtlasIdentity).ContentPresent())
    {
        std::fprintf(stderr, "OutlinerHost: the atlas refused to seat\n");
        return 1;
    }

    IconDepot Depot;
    Depot.Construct();
    Codec.SeatPicture(PictureDeclaration{ Depot.GlyphIdentity(), IconDepot::GlyphExtent, IconDepot::GlyphExtent,
                                          Depot.PictureOrdinates() });

    OutlinerPanel Outliner;
    Outliner.Construct(Depot);
    EntryInspectorPanel Inspector;

    SeedStand Stand;
    ForestStand Forest;
    AssembleForest(Stand, Forest);

    const HostState States[3] =
    {
        { "world-editor",  "g_03", "",        false },
        { "outliner-run",  "g_03", "light",   false },
        { "inspector",     "g_03", "",        true  },
    };

    for (const HostState& State : States)
    {
        for (int Warm = 0; Warm < 3; ++Warm)
        {
            ImGui::NewFrame();

            ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
            ImGui::SetNextWindowSize(ImVec2(static_cast<float>(DisplayAlong), static_cast<float>(DisplayAcross)));
            ImGui::Begin("RIFT \u2014 World Outliner", nullptr,
                         ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollbar);

            RecordingSurface Surface;
            if (Surface.Adopt(RecordingSurface::ShellLayer::Beneath).ContentPresent())
            {
                std::snprintf(Outliner.TakenIdentity, sizeof Outliner.TakenIdentity, "%s", State.TakenIdentity);
                std::snprintf(Outliner.RetentionRun, sizeof Outliner.RetentionRun, "%s", State.RetentionRun);
                PresentWorldEditorSeat(Surface, Spanning(0.0f, 0.0f, static_cast<float>(DisplayAlong), static_cast<float>(DisplayAcross)),
                                        Outliner, Inspector, Depot, Stand, Forest, State.SlideOpen);
                Surface.Seal();
            }

            ImGui::End();
            ImGui::Render();

            if (Warm == 2)
            {
                PixelSpace Extent{ DisplayAlong, DisplayAcross, {} };
                Codec.Rasterize(ImGui::GetDrawData(), Extent);
                char DumpPath[256];
                std::snprintf(DumpPath, sizeof DumpPath, "%s-%s.rgba", DumpPrefix, State.ShotRun);
                Codec.WriteRawDump(Extent, DumpPath);
                std::printf("OutlinerHost: %s seated\n", DumpPath);
            }
        }
    }

    ImGui::DestroyContext();
    return 0;
}
