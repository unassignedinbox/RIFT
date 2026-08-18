//============================================================================================================================================
//                                                           OUTLINERHOST.CPP
//============================================================================================================================================
// 🧩 The standalone scene directory — the CAD panel's outliner alone on the desk, no editor, no viewport, no lattice.

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

using namespace Slate;

constexpr std::uint32_t DisplayAlong  = 400u;    // [px]
constexpr std::uint32_t DisplayAcross = 760u;    // [px]

constexpr float DirectoryAlong  = 350.0f;   // [px] - the reference's directory column
constexpr float SheetMargin     = 25.0f;    // [px] - the desk margin

/// 🧩 The seed forest, verbatim from the reference's initialStore — Bracket_Rev4.
/// note  🔴 Self-referential — never returned by value; the caller owns one and AssembleForest wires it.
/// tag   internal
struct ForestStand
{
    OutlinerRowDeclaration Root[1];          // [-] - r001 Part
    OutlinerRowDeclaration Sketches[2];      // [-] - r003, r004
    OutlinerRowDeclaration Bracket[3];       // [-] - r007, r008, r009
    OutlinerRowDeclaration Bodies[3];        // [-] - r006, r010, r011
    OutlinerRowDeclaration Enclosed[2];      // [-] - r002 Sketches, r005 Bodies
};

/// 🧩 The host-owned disclosures and presences the forest borrows.
/// tag   internal
struct SeedStand
{
    bool ExpandedRoot     = true;    // [-] - r001
    bool ExpandedSketches = true;    // [-] - r002
    bool ExpandedBodies   = true;    // [-] - r005
    bool ExpandedBracket  = true;    // [-] - r006
    bool HiddenSketches   = false;   // [-] - r002
    bool HiddenBasePlate  = false;   // [-] - r003
};

/// 🧩 Wires the forest against the stand's disclosures and presences.
/// tag   internal
void AssembleForest(SeedStand& Stand, ForestStand& Forest)
{
    Forest.Sketches[0] = { "SK_BasePlate", "r003", DirectoryClassification::Sketch,   nullptr, &Stand.HiddenBasePlate, nullptr, 0u };
    Forest.Sketches[1] = { "SK_BoltHoles", "r004", DirectoryClassification::Sketch,   nullptr, nullptr, nullptr, 0u };

    Forest.Bracket[0] = { "SOL_Plate",   "r007", DirectoryClassification::Solid,    nullptr, nullptr, nullptr, 0u };
    Forest.Bracket[1] = { "SOL_Boss",    "r008", DirectoryClassification::Cylinder, nullptr, nullptr, nullptr, 0u };
    Forest.Bracket[2] = { "SOL_Rib",     "r009", DirectoryClassification::Solid,    nullptr, nullptr, nullptr, 0u };

    Forest.Bodies[0] = { "BODY_Bracket", "r006", DirectoryClassification::Enclosure, &Stand.ExpandedBracket, nullptr, Forest.Bracket, 3u };
    Forest.Bodies[1] = { "SOL_Housing",  "r010", DirectoryClassification::Solid,     nullptr, nullptr, nullptr, 0u };
    Forest.Bodies[2] = { "SOL_Dome",     "r011", DirectoryClassification::Sphere,    nullptr, nullptr, nullptr, 0u };

    Forest.Enclosed[0] = { "Sketches", "r002", DirectoryClassification::Enclosure, &Stand.ExpandedSketches, &Stand.HiddenSketches, Forest.Sketches, 2u };
    Forest.Enclosed[1] = { "Bodies",   "r005", DirectoryClassification::Enclosure, &Stand.ExpandedBodies,   nullptr,               Forest.Bodies,   3u };

    Forest.Root[0] = { "Part", "r001", DirectoryClassification::Scene, &Stand.ExpandedRoot, nullptr, Forest.Enclosed, 2u };
}

/// 🧩 One scripted host state a proof shot seats.
/// tag   internal
struct HostState
{
    const char*  ShotRun;              // [-] - the dump name
    const char*  TakenRuns[3];         // [-] - the taken tokens, additive past one
    const char*  RetentionRun;         // [-] - the retention run seated in the field
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

    OutlinerPanel Directory;
    SeedStand Stand;
    ForestStand Forest;
    AssembleForest(Stand, Forest);

    const HostState States[3] =
    {
        { "directory",   { "r007", nullptr, nullptr }, ""   },
        { "multiselect", { "r007", "r010", "r011" },   ""   },
        { "filter",      { "r003", nullptr, nullptr }, "sk" },
    };

    for (const HostState& State : States)
    {
        for (int Warm = 0; Warm < 3; ++Warm)
        {
            VendorIO.MousePos = ImVec2(-1.0e5f, -1.0e5f);
            ImGui::NewFrame();

            ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
            ImGui::SetNextWindowSize(ImVec2(static_cast<float>(DisplayAlong), static_cast<float>(DisplayAcross)));
            ImGui::Begin("RIFT \u2014 Directory", nullptr,
                         ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollbar);

            RecordingSurface Surface;
            if (Surface.Adopt(RecordingSurface::ShellLayer::Beneath).ContentPresent())
            {
                // ① The desk ground, then the directory alone.
                WorkspaceInk Sheet;
                Surface.Ground(Spanning(0.0f, 0.0f, static_cast<float>(DisplayAlong), static_cast<float>(DisplayAcross)),
                               Sheet.DeskGround, 0.0f);

                Directory.TakenCount = 0u;
                for (std::uint32_t Ordinal = 0u; Ordinal < 3u && State.TakenRuns[Ordinal] != nullptr; ++Ordinal)
                {
                    std::snprintf(Directory.TakenIdentities[Directory.TakenCount], sizeof Directory.TakenIdentities[0],
                                  "%s", State.TakenRuns[Ordinal]);
                    ++Directory.TakenCount;
                }
                std::snprintf(Directory.RetentionRun, sizeof Directory.RetentionRun, "%s", State.RetentionRun);

                Directory.Advance(Surface, Spanning(SheetMargin, SheetMargin, DirectoryAlong,
                                                    static_cast<float>(DisplayAcross) - SheetMargin * 2.0f),
                                  Forest.Root, 1u, OutlinerComposition{ "Directory", "Bracket_Rev4" }, Depot);
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
