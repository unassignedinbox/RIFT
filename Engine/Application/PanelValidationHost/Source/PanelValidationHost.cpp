//============================================================================================================================================
//                                                      PANELVALIDATIONHOST.CPP
//============================================================================================================================================
// 🧩 Records the texture-paint panels and the CAD drafting panel for direct visual comparison against their references.

#include "Engine/SlateUI/Interface/ControlPanel/Api/ControlPanel.h"
#include "Engine/SlateUI/Interface/DraftingPanel/Api/DraftingPanel.h"
#include "Engine/SlateUI/Interface/IconDepot/Api/IconDepot.h"
#include "Engine/SlateUI/Interface/OutlinerPanel/Api/OutlinerPanel.h"
#include "Engine/SlateUI/Interface/PropertiesPanel/Api/PropertiesPanel.h"
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

constexpr float SheetMargin = 40.0f;    // [px] - the sheet's outer margin
constexpr float SheetTitle  = 36.0f;    // [px] - the sheet's title strip
constexpr float CardGap     = 20.0f;    // [px] - between seated cards

//------------------------------------------------------------------------------------------------------------------------
//                                              THE TEXTURE-PAINT SEED
//------------------------------------------------------------------------------------------------------------------------

/// 🧩 Seats the four mock layers, verbatim from the reference's mockLayers.
/// tag   internal
void SeatLayers(LayerOrdinates (&Layers)[6])
{
    static const char* const EdgeWearChannels[3]  = { "Base Colour", "Roughness", "Metallic" };
    static const char* const DirtPassChannels[2]  = { "Base Colour", "Roughness" };
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

//------------------------------------------------------------------------------------------------------------------------
//                                              THE CAD DRAFTING SEED
//------------------------------------------------------------------------------------------------------------------------

/// 🧩 The CAD drafting seed — the same forest the standalone directory seats, reused by the drafting panel.
/// tag   internal
struct DraftingStand
{
    OutlinerRowDeclaration Root[1];
    OutlinerRowDeclaration Sketches[2];
    OutlinerRowDeclaration Bracket[3];
    OutlinerRowDeclaration Bodies[3];
    OutlinerRowDeclaration Enclosed[2];

    bool ExpandedRoot     = true;
    bool ExpandedSketches = true;
    bool ExpandedBodies   = true;
    bool ExpandedBracket  = true;
    bool HiddenSketches   = false;
    bool HiddenBasePlate  = false;
    bool RevisionFoldOpen = true;

    /// 🧩 Wires the forest, verbatim from the reference's initialStore.
    /// tag   internal
    void Assemble()
    {
        Sketches[0] = { "SK_BasePlate", "r003", DirectoryClassification::Sketch,   nullptr, &HiddenBasePlate, nullptr, 0u };
        Sketches[1] = { "SK_BoltHoles", "r004", DirectoryClassification::Sketch,   nullptr, nullptr, nullptr, 0u };

        Bracket[0] = { "SOL_Plate",   "r007", DirectoryClassification::Solid,    nullptr, nullptr, nullptr, 0u };
        Bracket[1] = { "SOL_Boss",    "r008", DirectoryClassification::Cylinder, nullptr, nullptr, nullptr, 0u };
        Bracket[2] = { "SOL_Rib",     "r009", DirectoryClassification::Solid,    nullptr, nullptr, nullptr, 0u };

        Bodies[0] = { "BODY_Bracket", "r006", DirectoryClassification::Enclosure, &ExpandedBracket, nullptr, Bracket, 3u };
        Bodies[1] = { "SOL_Housing",  "r010", DirectoryClassification::Solid,     nullptr, nullptr, nullptr, 0u };
        Bodies[2] = { "SOL_Dome",     "r011", DirectoryClassification::Sphere,    nullptr, nullptr, nullptr, 0u };

        Enclosed[0] = { "Sketches", "r002", DirectoryClassification::Enclosure, &ExpandedSketches, &HiddenSketches, Sketches, 2u };
        Enclosed[1] = { "Bodies",   "r005", DirectoryClassification::Enclosure, &ExpandedBodies,   nullptr,               Bodies,   3u };

        Root[0] = { "Part", "r001", DirectoryClassification::Scene, &ExpandedRoot, nullptr, Enclosed, 2u };
    }

    /// 🧩 Finds the row carrying the identity, through the whole forest.
    /// cost  🚩
    /// tag   internal
    const OutlinerRowDeclaration* Find(const char* Identity) const
    {
        const OutlinerRowDeclaration* Stacks[4] = { Root, Enclosed, Bodies, Bracket };
        const std::uint32_t Counts[4] = { 1u, 2u, 3u, 3u };
        const OutlinerRowDeclaration* Extra[2] = { Sketches, nullptr };
        for (std::uint32_t StackOrdinal = 0u; StackOrdinal < 4u; ++StackOrdinal)
            for (std::uint32_t Ordinal = 0u; Ordinal < Counts[StackOrdinal]; ++Ordinal)
            {
                const OutlinerRowDeclaration& Row = Stacks[StackOrdinal][Ordinal];
                if (std::strcmp(Row.Identity, Identity) == 0)
                    return &Row;
                for (std::uint32_t Inner = 0u; Inner < Row.EnclosureCount; ++Inner)
                    if (std::strcmp(Row.Enclosed[Inner].Identity, Identity) == 0)
                        return &Row.Enclosed[Inner];
            }
        for (std::uint32_t Ordinal = 0u; Ordinal < 2u; ++Ordinal)
            if (Extra[Ordinal] != nullptr && std::strcmp(Extra[Ordinal]->Identity, Identity) == 0)
                return Extra[Ordinal];
        return nullptr;
    }
};

/// 🧩 The revision record, seeded as the reference's generateRevisions seats it.
/// tag   internal
struct RevisionStand
{
    static constexpr std::uint32_t RevisionCapacity = 32u;   // [-]

    bool Folds[RevisionCapacity] = {};   // [-] - host-owned revision folds
    RevisionDeclaration Revisions[RevisionCapacity];

    /// 🧩 Seats the revisions — every record's creation pair, plus the seeded edits the fold demonstrates.
    /// tag   internal
    void Assemble(DraftingStand& Stand)
    {
        static const char* const CreatedRuns[11] = {
            "Created Part", "Created Sketches", "Created SK_BasePlate", "Created SK_BoltHoles", "Created Bodies",
            "Created BODY_Bracket", "Created SOL_Plate", "Created SOL_Boss", "Created SOL_Rib",
            "Created SOL_Housing", "Created SOL_Dome"
        };
        static const char* const Tokens[11] = {
            "r001", "r002", "r003", "r004", "r005", "r006", "r007", "r008", "r009", "r010", "r011"
        };

        std::uint32_t Seated = 0u;
        for (std::uint32_t Ordinal = 0u; Ordinal < 11u && Seated + 2u < RevisionCapacity; ++Ordinal)
        {
            Revisions[Seated++] = { Tokens[Ordinal], RevisionCategory::Start,  CreatedRuns[Ordinal], "Initial state",
                                    "", "System", "", "09:14", "2026-08-17", &Folds[Seated] };
            Revisions[Seated++] = { Tokens[Ordinal], RevisionCategory::Create, "Added to scene", "Inserted at origin",
                                    "", "System", "", "09:19", "2026-08-17", &Folds[Seated] };
        }

        // ① The seeded edits — the boss cylinder's radius, the plate's draft, the sketch's constraints.
        Revisions[Seated++] = { "r008", RevisionCategory::Parameter, "Adjusted radius", "Radius 6.25 \xE2\x86\x92 6.75",
                                "Increased radius to match new constraints.", "Alex Chen", "6.75", "10:42", "2026-08-17",
                                &Stand.RevisionFoldOpen };
        Revisions[Seated++] = { "r008", RevisionCategory::Feature, "Draft angle applied", "Draft angle 3\xC2\xB0",
                                "Customer requested smoother finish.", "Sam Rivera", "3.0", "11:05", "2026-08-17",
                                &Folds[Seated] };
        Revisions[Seated++] = { "r003", RevisionCategory::Sketch, "Profile constrained", "12 constraints",
                                "Approximated spline from DXF import.", "Maria Rossi", "", "11:31", "2026-08-17",
                                &Folds[Seated] };
        Count = Seated;
    }

    std::uint32_t Count = 0u;   // [-] - seated revisions
};

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
    const char* ProofPrefix = "VisualProof/PanelValidationHost";
    bool PauseAtEnd = false;
    for (int Ordinal = 1; Ordinal < ArgumentCount; ++Ordinal)
    {
        if (std::strcmp(Arguments[Ordinal], "--prefix") == 0 && Ordinal + 1 < ArgumentCount)
            ProofPrefix = Arguments[++Ordinal];
        if (std::strcmp(Arguments[Ordinal], "--pause") == 0)
            PauseAtEnd = true;
    }

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
    if (!Depot.Construct().ContentPresent())
    {
        std::fprintf(stderr, "PanelValidationHost: the glyph depot refused to construct\n");
        return 1;
    }
    Codec.SeatPicture(PictureDeclaration{ Depot.GlyphIdentity(), IconDepot::GlyphExtent, IconDepot::GlyphExtent,
                                          Depot.PictureOrdinates() });

    // ② The seated ordinates.
    LayerOrdinates Layers[6];
    SeatLayers(Layers);
    ChannelOrdinates Channels;
    SeatChannelOrdinates(Channels);
    MaskOrdinates MaskSheet;

    DraftingStand Drafting;
    Drafting.Assemble();
    RevisionStand Revisions;
    Revisions.Assemble(Drafting);

    LayerStackPanel StackPanel;
    ChannelPropertyPanel ChannelPanel;
    MaskPropertyPanel MaskPanel;

    // ③ The scripted states — one dump each.
    struct ValidationState
    {
        const char*  ShotRun;           // [-] - the dump name
        std::uint32_t Seat;             // [-] - 0 texture layers, 1 texture mask, 2 texture reorder, 3 CAD properties, 4 CAD history
        std::uint32_t ActiveLayer;      // [-] - taken layer ordinal
        bool          ActiveTargetMask; // [-]
        bool          LayerExpanded;    // [-]
        bool          DragScripted;     // [-] - seat a live reorder drag under the pointer
    };
    const ValidationState States[5] =
    {
        { "texturepaint-layers",  0u, 0u, false, true,  false },
        { "texturepaint-mask",    1u, 0u, true,  false, false },
        { "texturepaint-reorder", 2u, 1u, false, false, true  },
        { "cad-properties",       3u, 0u, false, false, false },
        { "cad-history",          4u, 0u, false, false, false },
    };

    for (const ValidationState& State : States)
    {
        for (int Warm = 0; Warm < 3; ++Warm)
        {
            // ①① The scripted pointer: a press on the Dirt Pass zone that travels between cards.
            if (State.DragScripted)
            {
                const ImVec2 PressSeat(250.0f, 330.0f);
                const ImVec2 DropSeat(250.0f, 380.0f);
                if (Warm == 0)
                {
                    VendorIO.MousePos = PressSeat;
                    VendorIO.AddMouseButtonEvent(0, true);
                }
                else
                {
                    VendorIO.MousePos = DropSeat;
                }
            }
            else
            {
                VendorIO.MousePos = ImVec2(-1.0e5f, -1.0e5f);
                if (Warm == 0)
                    VendorIO.AddMouseButtonEvent(0, false);
            }

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

                const char* TitleRun = State.Seat >= 3u
                    ? "RIFT \u2014 Panel Validation \xC2\xB7 CAD drafting panel (remix-remix-global-ui \xC2\xB7 DirectoryPane + Inspector \xC2\xB7 transcribed)"
                    : "RIFT \u2014 Panel Validation \xC2\xB7 Texture Paint (remix-remix-global-ui/TexturePaint.tsx \xC2\xB7 transcribed)";
                Surface.TextRun(SheetMargin, 8.0f, TitleRun, Sheet.InkMuted, 12.5f);

                if (State.Seat <= 2u)
                {
                    // ②① The texture-paint seats.
                    LayerOrdinates SeatedLayers[6];
                    SeatLayers(SeatedLayers);
                    SeatedLayers[State.ActiveLayer].Expanded = State.LayerExpanded;
                    StackPanel.ActiveLayer = State.ActiveLayer;
                    StackPanel.ActiveTargetMask = State.ActiveTargetMask;

                    const PlaneExtent StackSeat = PresentCard(Surface, SheetMargin, 730.0f, Sheet, "LayersPane \xC2\xB7 Suzanne");
                    StackPanel.Advance(Surface, StackSeat, SeatedLayers, 4u, Depot);

                    const PlaneExtent InspectorSeat = PresentCard(Surface, SheetMargin + 730.0f + CardGap, 730.0f, Sheet,
                                                                  State.Seat == 1u ? "MaskPropertyPanel \xC2\xB7 Edge Wear"
                                                                                  : "ChannelPropertyPanel \xC2\xB7 Brushed Copper");
                    if (State.Seat == 1u)
                        MaskPanel.Advance(Surface, InspectorSeat, MaskSheet, Depot);
                    else if (State.Seat == 0u)
                        ChannelPanel.Advance(Surface, InspectorSeat, Channels, Depot);
                }
                else
                {
                    // ②① The CAD drafting seats — the scene directory beside the metadata pane, the carousel beside them.
                    const OutlinerRowDeclaration* Inspected = Drafting.Find("r008");
                    ProfileOrdinates Profile;
                    if (Inspected != nullptr)
                        SeatProfile(Profile, *Inspected);

                    OutlinerPanel Directory;
                    Directory.SeatTaken("r008");
                    DraftingPanel DraftingSeat;
                    const PlaneExtent DraftingCard = PresentCard(Surface, SheetMargin, 810.0f, Sheet,
                                                                 "DraftingPanel \xC2\xB7 DirectoryPane + MetadataPane");
                    DraftingSeat.Advance(Surface, DraftingCard, Directory, Drafting.Root, 1u, Inspected, Profile, Depot);

                    PropertiesPanel Properties;
                    Properties.CarouselMode = State.Seat == 4u ? 1u : 0u;
                    const PlaneExtent PropertiesCard = PresentCard(Surface, SheetMargin + 810.0f + CardGap, 690.0f, Sheet,
                                                                   State.Seat == 4u ? "Inspector \xC2\xB7 History"
                                                                                    : "Inspector \xC2\xB7 Properties");
                    Properties.Advance(Surface, PropertiesCard, Inspected, Profile, Depot,
                                       Revisions.Revisions, Revisions.Count, Drafting.Root, 1u);
                }

                Surface.Seal();
            }

            ImGui::End();
            ImGui::Render();

            if (Warm == 2)
            {
                PixelSpace Extent{ DisplayAlong, DisplayAcross, {} };
                Codec.Rasterize(ImGui::GetDrawData(), Extent);
                char ProofPath[256];
                std::snprintf(ProofPath, sizeof ProofPath, "%s/%s.png", ProofPrefix, State.ShotRun);
                const Deliver<bool> Written = Codec.WritePortableNetworkGraphic(Extent, ProofPath);
                char RawPath[256];
                std::snprintf(RawPath, sizeof RawPath, "Build/Shots/%s.rgba", State.ShotRun);
                Codec.WriteRawDump(Extent, RawPath);   // 📝 the raw dump feeds the repository's small encoder
                if (Written.ContentPresent())
                    std::printf("%s: %s seated\n", "PanelValidationHost", ProofPath);
                else
                    std::fprintf(stderr, "%s: %s refused — %s\n", "PanelValidationHost", ProofPath, Written.Declined().Run);
            }
        }
    }

    ImGui::DestroyContext();

    std::printf("PanelValidationHost: every proof seated under %s\n", ProofPrefix);
    if (PauseAtEnd)
    {
        std::printf("PanelValidationHost: press enter to close\n");
        std::fgetc(stdin);
    }
    return 0;
}
