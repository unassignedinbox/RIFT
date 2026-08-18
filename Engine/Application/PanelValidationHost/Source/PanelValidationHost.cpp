//============================================================================================================================================
//                                                      PANELVALIDATIONHOST.CPP
//============================================================================================================================================
// 🧩 Records the texture-paint and CAD panels for direct visual comparison against their reference prototypes.

#include "Engine/SlateUI/Interface/CadPanel/Api/CadPanel.h"
#include "Engine/SlateUI/Interface/ControlPanel/Api/ControlPanel.h"
#include "Engine/SlateUI/Interface/IconDepot/Api/IconDepot.h"
#include "Engine/SlateUI/Interface/RasterCodec/Api/RasterCodec.h"
#include "Engine/SlateUI/Interface/RecordingSurface/Api/RecordingSurface.h"
#include "Engine/SlateUI/Interface/TexturePaintPanel/Api/TexturePaintPanel.h"
#include "Engine/SlateUI/Interface/ThemeSpecification/Api/ThemeSpecification.h"

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

constexpr std::uint32_t DisplayAlong  = 1600u;   // [px]
constexpr std::uint32_t DisplayAcross = 900u;    // [px]

constexpr float SheetMargin   = 40.0f;    // [px] - the sheet's outer margin
constexpr float SheetTitle    = 36.0f;    // [px] - the sheet's title strip
constexpr float CardGap       = 20.0f;    // [px] - between seated cards
constexpr float CardExtent    = 730.0f;   // [px] - each seated card's along span

/// 🧩 The browser forest the CAD seat borrows, seeded as the reference's data.js seeds it.
/// tag   internal
struct CadSeedStand
{
    bool OriginExpanded = true;    // [-] - the Origin enclosure
    bool SketchesExpanded = true;  // [-]
    bool BodiesExpanded = true;    // [-]
    bool OriginHidden = false;     // [-]
    bool FrontHidden = false;      // [-]
    bool OriginLocked = true;      // [-] - the datums lock

    BrowserRowDeclaration Planes[3];       // [-] - Front, Top, Right
    BrowserRowDeclaration Enclosures[3];   // [-] - Origin, Sketches, Bodies

    CadSeedStand()
    {
        Planes[0] = { "Front Plane", BrowserClassification::Plane, 0xEF4444u, "XY", nullptr, &FrontHidden, nullptr, nullptr, 0u };
        Planes[1] = { "Top Plane",   BrowserClassification::Plane, 0x22C55Eu, "XZ", nullptr, nullptr, nullptr, nullptr, 0u };
        Planes[2] = { "Right Plane", BrowserClassification::Plane, 0x3B82F6u, "YZ", nullptr, nullptr, nullptr, nullptr, 0u };

        Enclosures[0] = { "Origin",   BrowserClassification::Datums,    0u, "", &OriginExpanded,   &OriginHidden, &OriginLocked, Planes, 3u };
        Enclosures[1] = { "Sketches", BrowserClassification::Enclosure, 0u, "", &SketchesExpanded, nullptr, nullptr, nullptr, 0u };
        Enclosures[2] = { "Bodies",   BrowserClassification::Enclosure, 0u, "", &BodiesExpanded,   nullptr, nullptr, nullptr, 0u };
    }
};

/// 🧩 Seats the four mock layers, verbatim from the reference's mockLayers.
/// tag   internal
void SeatLayers(LayerOrdinates (&Layers)[6])
{
    static const char* const EdgeWearChannels[3] = { "Base Colour", "Roughness", "Metallic" };
    static const char* const DirtPassChannels[2] = { "Base Colour", "Roughness" };
    static const char* const ScratchesChannels[2] = { "Base Colour", "Bump" };
    static const char* const BaseMetalChannels[4] = { "Base Colour", "Roughness", "Metallic", "Bump" };

    std::snprintf(Layers[0].Name, sizeof Layers[0].Name, "Edge Wear");
    Layers[0].Content = 0u;  std::snprintf(Layers[0].Transfer, sizeof Layers[0].Transfer, "Multiply");
    Layers[0].Opacity = 78.0;  Layers[0].Shown = true;  Layers[0].PaintPacked = 0xF97316u;  Layers[0].TagPacked = 0xEAB308u;
    Layers[0].Channels = EdgeWearChannels;  Layers[0].ChannelCount = 3u;
    Layers[0].Mask.Enabled = true;  std::snprintf(Layers[0].Mask.Source, sizeof Layers[0].Mask.Source, "Generator");
    Layers[0].Mask.Strength = 92.0;  Layers[0].Mask.Invert = false;  Layers[0].Mask.Shown = true;

    std::snprintf(Layers[1].Name, sizeof Layers[1].Name, "Dirt Pass");
    Layers[1].Content = 1u;  std::snprintf(Layers[1].Transfer, sizeof Layers[1].Transfer, "Overlay");
    Layers[1].Opacity = 45.0;  Layers[1].Shown = true;  Layers[1].PaintPacked = 0x8B5CF6u;  Layers[1].TagPacked = 0xEC4899u;
    Layers[1].Channels = DirtPassChannels;  Layers[1].ChannelCount = 2u;
    Layers[1].Mask.Enabled = true;  std::snprintf(Layers[1].Mask.Source, sizeof Layers[1].Mask.Source, "Paint");
    Layers[1].Mask.Strength = 100.0;  Layers[1].Mask.Invert = true;  Layers[1].Mask.Shown = true;

    std::snprintf(Layers[2].Name, sizeof Layers[2].Name, "Scratches");
    Layers[2].Content = 0u;  std::snprintf(Layers[2].Transfer, sizeof Layers[2].Transfer, "Screen");
    Layers[2].Opacity = 60.0;  Layers[2].Shown = false;  Layers[2].PaintPacked = 0xF97316u;  Layers[2].TagPacked = 0x06B6D4u;
    Layers[2].Channels = ScratchesChannels;  Layers[2].ChannelCount = 2u;
    Layers[2].Mask.Enabled = false;

    std::snprintf(Layers[3].Name, sizeof Layers[3].Name, "Base Metal");
    Layers[3].Content = 1u;  std::snprintf(Layers[3].Transfer, sizeof Layers[3].Transfer, "Normal");
    Layers[3].Opacity = 100.0;  Layers[3].Shown = true;  Layers[3].PaintPacked = 0x8B5CF6u;  Layers[3].TagPacked = 0x3B82F6u;
    Layers[3].Channels = BaseMetalChannels;  Layers[3].ChannelCount = 4u;
    Layers[3].Mask.Enabled = false;
}

/// 🧩 One framed card seat on the validation sheet.
/// tag   internal
PlaneExtent PresentCard(RecordingSurface& Surface, float LeadingAlong, float TrailingExtent, const WorkspaceInk& Sheet,
                        const char* CaptionRun)
{
    const float CardAcross = static_cast<float>(DisplayAcross) - SheetMargin - SheetTitle - 20.0f;
    const PlaneExtent Card = Spanning(LeadingAlong, SheetMargin + SheetTitle + 20.0f, TrailingExtent, CardAcross);
    Surface.Ground(Card.Inset(0.0f, -6.0f), Partial(0x000000u, 0.35), 18.0f);
    Surface.Ground(Card, Sheet.StandingGround, 18.0f);
    Surface.Edge(Card, Sheet.HairEdgeStrong, 1.0f, 18.0f);
    Surface.TextRun(Card.LeastAlong + 18.0f, Card.LeastAcross - 26.0f, CaptionRun, Sheet.InkFaint, 11.0f);
    return Card.Inset(10.0f, 56.0f).Inset(-10.0f, -10.0f);
}

}   // namespace

//------------------------------------------------------------------------------------------------------------------------
//                                                          ENTRY
//------------------------------------------------------------------------------------------------------------------------

int main(int ArgumentCount, char** Arguments)
{
    const char* DumpPrefix = "Build/Shots/validation";
    for (int Ordinal = 1; Ordinal < ArgumentCount; ++Ordinal)
        if (std::strcmp(Arguments[Ordinal], "--prefix") == 0 && Ordinal + 1 < ArgumentCount)
            DumpPrefix = Arguments[++Ordinal];

    // ① Context, default typeface at three crisp sizes, atlas seated against the codec.
    ImGui::CreateContext();
    ImGuiIO& VendorIO = ImGui::GetIO();
    VendorIO.DisplaySize = ImVec2(static_cast<float>(DisplayAlong), static_cast<float>(DisplayAcross));

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
    Colours[ImGuiCol_PopupBg]    = ImVec4(0.055f, 0.055f, 0.055f, 0.98f);
    Colours[ImGuiCol_Border]     = ImVec4(1.0f, 1.0f, 1.0f, 0.10f);
    Colours[ImGuiCol_FrameBg]    = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

    void* AtlasIdentity = reinterpret_cast<void*>(static_cast<std::uintptr_t>(0x1u));
    RasterCodec Codec;
    if (!Codec.SeatAtlas(AtlasIdentity).ContentPresent())
    {
        std::fprintf(stderr, "PanelValidationHost: the atlas refused to seat\n");
        return 1;
    }

    IconDepot Depot;
    Depot.Construct();
    Codec.SeatPicture(PictureDeclaration{ Depot.GlyphIdentity(), IconDepot::GlyphExtent, IconDepot::GlyphExtent,
                                          Depot.PictureOrdinates() });

    // ② The seated ordinates — mock layers, channel sheet, mask sheet, the CAD seed.
    LayerOrdinates Layers[6];
    SeatLayers(Layers);

    ChannelOrdinates Channels;
    SeatChannelOrdinates(Channels);

    MaskOrdinates MaskSheet;

    CadSeedStand CadSeed;
    const CadComposition Composition = { "Part01", "Part01", 0u, 0u };

    LayerStackPanel StackPanel;
    ChannelPropertyPanel ChannelPanel;
    MaskPropertyPanel MaskPanel;
    CadWorkspacePanel CadPanel;

    // ③ The scripted states — one dump each.
    struct ValidationState
    {
        const char* ShotRun;     // [-] - the dump name
        std::uint32_t Seat;      // [-] - 0 texture layers, 1 texture mask, 2 CAD
        std::uint32_t ActiveLayer;   // [-] - taken layer ordinal
        bool         ActiveTargetMask;   // [-]
        bool         LayerExpanded;      // [-]
    };
    const ValidationState States[3] =
    {
        { "texturepaint-layers",  0u, 0u, false, true  },
        { "texturepaint-mask",    1u, 0u, true,  false },
        { "cad-workspace",        2u, 0u, false, false },
    };

    for (const ValidationState& State : States)
    {
        for (int Warm = 0; Warm < 3; ++Warm)
        {
            ImGui::NewFrame();

            ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
            ImGui::SetNextWindowSize(ImVec2(static_cast<float>(DisplayAlong), static_cast<float>(DisplayAcross)));
            ImGui::Begin("RIFT \u2014 Panel Validation", nullptr,
                         ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollbar);

            RecordingSurface Surface;
            if (Surface.Adopt(RecordingSurface::ShellLayer::Beneath).ContentPresent())
            {
                WorkspaceInk Sheet;
                const PlaneExtent Desk = Spanning(0.0f, 0.0f, static_cast<float>(DisplayAlong), static_cast<float>(DisplayAcross));
                Surface.Ground(Desk, Sheet.DeskGround, 0.0f);

                // ① The sheet's title strip.
                const char* TitleRun = State.Seat == 2u ? "RIFT \u2014 Panel Validation \xC2\xB7 CAD Workspace (References/Cad \xC2\xB7 transcribed)"
                    : "RIFT \u2014 Panel Validation \xC2\xB7 Texture Paint (References/remix-remix-global-ui/TexturePaint.tsx \xC2\xB7 transcribed)";
                Surface.TextRun(SheetMargin, 8.0f, TitleRun, Sheet.InkMuted, 12.5f);

                if (State.Seat == 0u || State.Seat == 1u)
                {
                    LayerOrdinates SeatedLayers[6];
                    SeatLayers(SeatedLayers);
                    SeatedLayers[State.ActiveLayer].Expanded = State.LayerExpanded;

                    StackPanel.ActiveLayer = State.ActiveLayer;
                    StackPanel.ActiveTargetMask = State.ActiveTargetMask;

                    const PlaneExtent StackSeat = PresentCard(Surface, SheetMargin, CardExtent, Sheet,
                                                              "LayersPane \xC2\xB7 Suzanne");
                    StackPanel.Advance(Surface, StackSeat, SeatedLayers, 4u, Depot);

                    const PlaneExtent InspectorSeat = PresentCard(Surface, SheetMargin + CardExtent + CardGap, CardExtent, Sheet,
                                                                  State.Seat == 0u ? "ChannelPropertyPanel \xC2\xB7 Brushed Copper"
                                                                                  : "MaskPropertyPanel \xC2\xB7 Edge Wear");
                    if (State.Seat == 0u)
                        ChannelPanel.Advance(Surface, InspectorSeat, Channels, Depot);
                    else
                        MaskPanel.Advance(Surface, InspectorSeat, MaskSheet, Depot);
                }
                else
                {
                    CadPanel.Advance(Surface, Desk, CadSeed.Enclosures, 3u, Composition, Depot);
                }

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
                std::printf("PanelValidationHost: %s seated\n", DumpPath);
            }
        }
    }

    ImGui::DestroyContext();
    return 0;
}
