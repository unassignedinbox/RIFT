//============================================================================================================================================
//                                                           OUTLINERHOST.CPP
//============================================================================================================================================
// 🧩 The standalone world-editor seat — top bar, options menu, viewport, docked inspector — around the general-purpose outliner.

#include "Engine/SlateUI/Interface/ControlPanel/Api/ControlPanel.h"
#include "Engine/SlateUI/Interface/IconDepot/Api/IconDepot.h"
#include "Engine/SlateUI/Interface/OutlinerPanel/Api/OutlinerPanel.h"
#include "Engine/SlateUI/Interface/RasterCodec/Api/RasterCodec.h"
#include "Engine/SlateUI/Interface/RecordingSurface/Api/RecordingSurface.h"
#include "Engine/SlateUI/Interface/ThemeSpecification/Api/ThemeSpecification.h"

#include "imgui.h"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>

//------------------------------------------------------------------------------------------------------------------------
//                                                          FIGURES
//------------------------------------------------------------------------------------------------------------------------

namespace
{

using namespace Slate;

constexpr std::uint32_t DisplayAlong  = 1500u;   // [px]
constexpr std::uint32_t DisplayAcross = 860u;    // [px]

constexpr float TopBarAcross      = 36.0f;   // [px] - the reference's top bar
constexpr float OptionsAlong      = 220.0f;   // [px] - the options menu column
constexpr float InspectorAlong    = 700.0f;   // [px] - the docked inspector
constexpr float OutlinerAlong     = 350.0f;   // [px] - the outliner column inside the inspector

//------------------------------------------------------------------------------------------------------------------------
//                                                    THE SEED FOREST
//------------------------------------------------------------------------------------------------------------------------

/// 🧩 Every host-owned ordinate and disclosure the seed forest carries, seated at the reference's values.
/// tag   internal
struct SeedStand
{
    bool ExpandedLighting   = true;    // [-] - g_02
    bool ExpandedEnvironment = true;   // [-] - g_07
    bool ExpandedSystems     = true;   // [-] - g_11
    bool HiddenLighting      = false;   // [-] - g_02
    bool HiddenSun           = false;   // [-] - g_03

    EntryOrdinates Sun;                       // [-] - g_03, defaults seated (100000 lm, 255/240/220, shadows)
    EntryOrdinates Atmosphere;                // [-] - g_04
    EntryOrdinates PlayerStart;               // [-] - g_05
    EntryOrdinates MainCamera;                // [-] - g_06
    EntryOrdinates BuildingA;                 // [-] - g_08
    EntryOrdinates BuildingB;                 // [-] - g_09
    EntryOrdinates FireHydrant;               // [-] - g_10
    EntryOrdinates GameManager;               // [-] - g_12
    EntryOrdinates CityNoise;                 // [-] - g_13
    EntryOrdinates DustMotes;                 // [-] - g_14

    SeedStand()
    {
        Atmosphere.Intensity = 10000u;
        Atmosphere.LightColour[0] = 180u;  Atmosphere.LightColour[1] = 200u;  Atmosphere.LightColour[2] = 255u;
        Atmosphere.CastShadows = false;

        MainCamera.FieldOfView = 90.0;  MainCamera.NearClip = 0.1;  MainCamera.FarClip = 10000.0;

        BuildingB.Position[0] = 500.0;  BuildingB.Position[2] = 200.0;  BuildingB.Rotation[1] = 90.0;
        FireHydrant.Position[0] = 120.0;  FireHydrant.Position[2] = 45.0;

        CityNoise.Volume = 0.8;  CityNoise.Looping = true;  CityNoise.Spatial = false;
        DustMotes.EmitRate = 50.0;  DustMotes.LifeTime = 5.0;  DustMotes.Looping = true;
    }
};

/// 🧩 The assembled seed forest, borrowed from one seed stand, verbatim from initialGameGraph.
/// note  🔴 Self-referential — the enclosing rows point at the sibling arrays of the same stand — so a
///       ForestStand is never returned by value; the caller owns one and AssembleForest wires it.
/// tag   internal
struct ForestStand
{
    OutlinerRowDeclaration Root[1];         // [-] - g_01
    OutlinerRowDeclaration Lighting[2];     // [-] - g_03, g_04
    OutlinerRowDeclaration Environment[3];  // [-] - g_08..g_10
    OutlinerRowDeclaration Systems[3];      // [-] - g_12..g_14
    OutlinerRowDeclaration Enclosures[3];   // [-] - g_02, g_07, g_11
    OutlinerRowDeclaration Standing[2];     // [-] - g_05, g_06
    OutlinerRowDeclaration Enclosed[5];     // [-] - the root's direct forest, in reference order
};

/// 🧩 Wires the forest against the stand's disclosures, presences and enclosed rows.
/// pre   Forest outlives every tick that presents it
/// tag   internal
void AssembleForest(SeedStand& Stand, ForestStand& Forest)
{
    Forest.Lighting[0]  = { "Directional Light (Sun)", "g_03", OutlinerClassification::Light, nullptr, &Stand.HiddenSun, nullptr, 0u };
    Forest.Lighting[1]  = { "Sky Atmosphere",          "g_04", OutlinerClassification::Light, nullptr, nullptr, nullptr, 0u };

    Forest.Environment[0] = { "Building_A_Prefab",       "g_08", OutlinerClassification::Actor, nullptr, nullptr, nullptr, 0u };
    Forest.Environment[1] = { "Building_B_Prefab",       "g_09", OutlinerClassification::Actor, nullptr, nullptr, nullptr, 0u };
    Forest.Environment[2] = { "Street_Prop_FireHydrant", "g_10", OutlinerClassification::Actor, nullptr, nullptr, nullptr, 0u };

    Forest.Systems[0] = { "GameManager",        "g_12", OutlinerClassification::Script,   nullptr, nullptr, nullptr, 0u };
    Forest.Systems[1] = { "Ambient_City_Noise", "g_13", OutlinerClassification::Audio,    nullptr, nullptr, nullptr, 0u };
    Forest.Systems[2] = { "Dust_Motes_VFX",     "g_14", OutlinerClassification::Particle, nullptr, nullptr, nullptr, 0u };

    Forest.Enclosures[0] = { "Lighting",    "g_02", OutlinerClassification::Enclosure, &Stand.ExpandedLighting,    &Stand.HiddenLighting, Forest.Lighting,    2u };
    Forest.Enclosures[1] = { "Environment", "g_07", OutlinerClassification::Enclosure, &Stand.ExpandedEnvironment, nullptr,               Forest.Environment, 3u };
    Forest.Enclosures[2] = { "Systems",     "g_11", OutlinerClassification::Enclosure, &Stand.ExpandedSystems,     nullptr,               Forest.Systems,     3u };

    Forest.Standing[0] = { "Player_Start", "g_05", OutlinerClassification::Trigger, nullptr, nullptr, nullptr, 0u };
    Forest.Standing[1] = { "Main Camera",  "g_06", OutlinerClassification::Camera,  nullptr, nullptr, nullptr, 0u };

    // ① The root's direct forest, in the reference's order — three enclosures with two standing rows between.
    Forest.Enclosed[0] = Forest.Enclosures[0];
    Forest.Enclosed[1] = Forest.Standing[0];
    Forest.Enclosed[2] = Forest.Standing[1];
    Forest.Enclosed[3] = Forest.Enclosures[1];
    Forest.Enclosed[4] = Forest.Enclosures[2];

    Forest.Root[0] = { "Level_01_City", "g_01", OutlinerClassification::Level, nullptr, nullptr, Forest.Enclosed, 5u };
}

/// 🧩 Finds the row declaration carrying the identity, through the whole forest.
/// cost  🚩
/// tag   internal
const OutlinerRowDeclaration* FindRow(const OutlinerRowDeclaration* Rows, std::uint32_t RowCount, const char* Identity)
{
    for (std::uint32_t Ordinal = 0u; Ordinal < RowCount; ++Ordinal)
    {
        if (std::strcmp(Rows[Ordinal].Identity, Identity) == 0)
            return &Rows[Ordinal];
        const OutlinerRowDeclaration* Enclosed = FindRow(Rows[Ordinal].Enclosed, Rows[Ordinal].EnclosureCount, Identity);
        if (Enclosed != nullptr)
            return Enclosed;
    }
    return nullptr;
}

/// 🧩 The ordinates a row identity inspects.
/// cost  ✔️
/// tag   internal
EntryOrdinates* FindOrdinates(SeedStand& Stand, const char* Identity)
{
    if (std::strcmp(Identity, "g_04") == 0) return &Stand.Atmosphere;
    if (std::strcmp(Identity, "g_05") == 0) return &Stand.PlayerStart;
    if (std::strcmp(Identity, "g_06") == 0) return &Stand.MainCamera;
    if (std::strcmp(Identity, "g_08") == 0) return &Stand.BuildingA;
    if (std::strcmp(Identity, "g_09") == 0) return &Stand.BuildingB;
    if (std::strcmp(Identity, "g_10") == 0) return &Stand.FireHydrant;
    if (std::strcmp(Identity, "g_12") == 0) return &Stand.GameManager;
    if (std::strcmp(Identity, "g_13") == 0) return &Stand.CityNoise;
    if (std::strcmp(Identity, "g_14") == 0) return &Stand.DustMotes;
    return &Stand.Sun;
}

}   // namespace

//------------------------------------------------------------------------------------------------------------------------
//                                                    THE HOST PRESENTATION
//------------------------------------------------------------------------------------------------------------------------

namespace
{

/// 🧩 One scripted host state a proof shot seats.
/// tag   internal
struct HostState
{
    const char*  ShotRun;          // [-] - the dump name
    const char*  TakenIdentity;    // [-] - the outliner's taken row
    const char*  RetentionRun;     // [-] - the retention run seated in the field
    bool         SlideOpen;        // [-] - whether the inspector slide presents
};

/// 🧩 Presents the whole seat — top bar, options, viewport, inspector — one tick.
/// tag   internal
void PresentSeat(RecordingSurface& Surface, const PlaneExtent& Desk, OutlinerPanel& Outliner,
                 EntryInspectorPanel& Inspector, const IconDepot& Depot, SeedStand& Stand,
                 ForestStand& Forest, bool SlideOpen)
{
    WorkspaceInk Sheet;

    // ① Top bar.
    const PlaneExtent TopBar = Spanning(Desk.LeastAlong, Desk.LeastAcross, Desk.SpanAlong(), TopBarAcross);
    Surface.Ground(TopBar, Sheet.SunkenGround, 0.0f);
    Surface.Rule(TopBar.LeastAlong, TopBar.MostAcross - 1.0f, TopBar.SpanAlong(), 1.0f, Sheet.HairEdge);
    Depot.PresentGlyphCentred(Surface, TopBar.LeastAlong + 23.0f, TopBar.LeastAcross + TopBarAcross * 0.5f, 18.0f, Sheet.Accent);
    Surface.TextRun(TopBar.LeastAlong + 42.0f, CentredAcross(TopBar, Surface.RunExtent(12.5f)), "World Editor", Sheet.InkPrimary, 12.5f);
    Surface.TextRun(TopBar.LeastAlong + 42.0f + Surface.MeasureRun("World Editor", 12.5f) + 12.0f,
                    CentredAcross(TopBar, Surface.RunExtent(11.0f)), "Level_01_City.map", Sheet.InkFaint, 11.0f);

    const float HintSize = 10.5f;
    const float HintExtent = Surface.MeasureRun("Tab", 10.5f) + Surface.MeasureRun("  summon inspector", HintSize);
    const PlaneExtent HintPill = Spanning(TopBar.MostAlong - HintExtent - 40.0f, CentredAcross(TopBar, 26.0f), HintExtent + 30.0f, 26.0f);
    Surface.Ground(HintPill, Sheet.StandingGround, 13.0f);
    Surface.Edge(HintPill, Sheet.HairEdge, 1.0f, 13.0f);
    Surface.TextRun(HintPill.LeastAlong + 12.0f, CentredAcross(HintPill, Surface.RunExtent(10.5f)), "Tab", Sheet.InkPrimary, 10.5f);
    Surface.TextRun(HintPill.LeastAlong + 12.0f + Surface.MeasureRun("Tab", 10.5f) + 7.0f,
                    CentredAcross(HintPill, Surface.RunExtent(HintSize)), "summon inspector", Sheet.InkMuted, HintSize);

    const PlaneExtent MainRow = Spanning(Desk.LeastAlong, TopBar.MostAcross, Desk.SpanAlong(), Desk.MostAcross - TopBar.MostAcross);

    // ② Options menu.
    const PlaneExtent Options = Spanning(MainRow.LeastAlong, MainRow.LeastAcross, OptionsAlong, MainRow.SpanAcross());
    Surface.Ground(Options, Sheet.SunkenGround, 0.0f);
    Surface.Rule(Options.MostAlong - 1.0f, Options.LeastAcross, 1.0f, Options.SpanAcross(), Sheet.HairEdgeStrong);
    Surface.TextRun(Options.LeastAlong + 16.0f, Options.LeastAcross + 14.0f, "Options", Sheet.InkPrimary, 12.5f);
    Surface.Rule(Options.LeastAlong, Options.LeastAcross + 46.0f, Options.SpanAlong(), 1.0f, Sheet.HairEdge);

    Surface.TextRun(Options.LeastAlong + 16.0f, Options.LeastAcross + 70.0f, "Dock Inspector", Sheet.InkMuted, 12.5f);
    static bool DockTaken = true;
    const ControlSheet Controls = ControlSheetFromWorkspace(Sheet);
    const ControlRowDeclaration DockRow = { "", 88.0f, 13.5f };
    PresentSwitchRow(Surface, Spanning(Options.LeastAlong + 116.0f, Options.LeastAcross + 62.0f, 50.0f, 32.0f),
                     DockRow, DockTaken, Controls, "options.dock");
    Surface.TextRunClipped(Options.LeastAlong + 16.0f, Options.LeastAcross + 104.0f,
                           "Inspector is docked to the right side of the screen.", Sheet.InkFaint, 11.0f, Options.SpanAlong() - 32.0f);

    Surface.TextRun(Options.LeastAlong + 16.0f, Options.LeastAcross + 146.0f, "Workspace Mode", Sheet.InkMuted, 12.5f);
    const char* const Modes[3] = { "Drafting", "Texture Paint", "Game Editor" };
    for (std::uint32_t Ordinal = 0u; Ordinal < 3u; ++Ordinal)
    {
        const bool Taken = Ordinal == 2u;
        const PlaneExtent Button = Spanning(Options.LeastAlong + 16.0f, Options.LeastAcross + 168.0f + Ordinal * 40.0f,
                                            Options.SpanAlong() - 32.0f, 32.0f);
        Surface.Ground(Button, Taken ? Sheet.AccentSoft : Sheet.TileGround, 6.0f);
        Surface.Edge(Button, Taken ? Sheet.Accent : Sheet.HairEdge, 1.0f, 6.0f);
        Surface.TextRun(Surface.CentredAlong(Button, Modes[Ordinal], 11.0f), CentredAcross(Button, Surface.RunExtent(11.0f)),
                        Modes[Ordinal], Taken ? Sheet.InkPrimary : Sheet.InkMuted, 11.0f);
    }

    // ③ Viewport — weave lattice, vignette, hint runs.
    const PlaneExtent Viewport = Spanning(Options.MostAlong, MainRow.LeastAcross,
                                          MainRow.SpanAlong() - OptionsAlong - InspectorAlong, MainRow.SpanAcross());
    Surface.Ground(Viewport, Sheet.DeskGround, 0.0f);

    for (float Across = 0.0f; Across < Viewport.SpanAcross(); Across += 28.0f)
        Surface.Stroke(Viewport.LeastAlong, Viewport.LeastAcross + Across, Viewport.MostAlong, Viewport.LeastAcross + Across, 1.0f,
                       Partial(0xFFFFFFu, 0.028));
    for (float Along = 0.0f; Along < Viewport.SpanAlong(); Along += 28.0f)
        Surface.Stroke(Viewport.LeastAlong + Along, Viewport.LeastAcross, Viewport.LeastAlong + Along, Viewport.MostAcross, 1.0f,
                       Partial(0xFFFFFFu, 0.028));
    for (float Across = 0.0f; Across < Viewport.SpanAcross(); Across += 140.0f)
        Surface.Stroke(Viewport.LeastAlong, Viewport.LeastAcross + Across, Viewport.MostAlong, Viewport.LeastAcross + Across, 1.0f,
                       Partial(0xFFFFFFu, 0.055));
    for (float Along = 0.0f; Along < Viewport.SpanAlong(); Along += 140.0f)
        Surface.Stroke(Viewport.LeastAlong + Along, Viewport.LeastAcross, Viewport.LeastAlong + Along, Viewport.MostAcross, 1.0f,
                       Partial(0xFFFFFFu, 0.055));

    const InkOrdinate VeilGround = Partial(0x000000u, 0.0);
    const InkOrdinate VeilEdge   = Partial(0x000000u, 0.55);
    Surface.Scrim(Spanning(Viewport.LeastAlong, Viewport.LeastAcross, Viewport.SpanAlong(), Viewport.SpanAcross() * 0.25f), VeilGround, VeilEdge);
    Surface.Scrim(Spanning(Viewport.LeastAlong, Viewport.MostAcross - Viewport.SpanAcross() * 0.25f, Viewport.SpanAlong(), Viewport.SpanAcross() * 0.25f), VeilEdge, VeilGround);
    Surface.ScrimAlong(Spanning(Viewport.LeastAlong, Viewport.LeastAcross, Viewport.SpanAlong() * 0.25f, Viewport.SpanAcross()), VeilGround, VeilEdge);
    Surface.ScrimAlong(Spanning(Viewport.MostAlong - Viewport.SpanAlong() * 0.25f, Viewport.LeastAcross, Viewport.SpanAlong() * 0.25f, Viewport.SpanAcross()), VeilEdge, VeilGround);

    const char* HintRun = SlideOpen ? "press Tab to slide through properties" : "press Tab to summon the outliner";
    Surface.TextRun(Surface.CentredAlong(Viewport, HintRun, 12.0f), Viewport.LeastAcross + Viewport.SpanAcross() * 0.45f,
                    HintRun, Sheet.InkFaint, 12.0f);

    const PlaneExtent HintsRail = Spanning(Viewport.LeastAlong, Viewport.MostAcross - 28.0f, Viewport.SpanAlong(), 28.0f);
    Surface.Scrim(HintsRail, VeilGround, Partial(0x000000u, 0.5));
    const char* const Hints[4] = { "Orbit LMB", "Pan MMB", "Zoom Wheel", "Inspector Tab" };
    float HintsAlong = HintsRail.LeastAlong + 13.0f;
    for (std::uint32_t Ordinal = 0u; Ordinal < 4u; ++Ordinal)
    {
        Surface.TextRun(HintsAlong, CentredAcross(HintsRail, Surface.RunExtent(10.5f)), Hints[Ordinal], Sheet.InkFaint, 10.5f);
        HintsAlong += Surface.MeasureRun(Hints[Ordinal], 10.5f) + 9.0f;
        if (Ordinal < 3u)
        {
            Surface.Medallion(HintsAlong, CentredAcross(HintsRail, 0.0f) + 14.0f, 1.0f, Sheet.FieldUnit);
            HintsAlong += 9.0f;
        }
    }

    // ④ Docked inspector — the two slides.
    const PlaneExtent InspectorSeat = Spanning(MainRow.MostAlong - InspectorAlong, MainRow.LeastAcross, InspectorAlong, MainRow.SpanAcross());
    Surface.Ground(InspectorSeat, Sheet.StandingGround, 0.0f);
    Surface.Rule(InspectorSeat.LeastAlong, InspectorSeat.LeastAcross, 1.0f, InspectorSeat.SpanAcross(), Sheet.HairEdgeStrong);

    const PlaneExtent OutlinerSeat = Spanning(InspectorSeat.LeastAlong, InspectorSeat.LeastAcross, OutlinerAlong, InspectorSeat.SpanAcross());
    const PlaneExtent PropertiesSeat = Spanning(InspectorSeat.LeastAlong, InspectorSeat.LeastAcross, InspectorAlong, InspectorSeat.SpanAcross());

    if (SlideOpen)
    {
        const OutlinerRowDeclaration* Declared = FindRow(Forest.Root, 1u, Outliner.TakenIdentity);
        EntryOrdinates* Ordinates = FindOrdinates(Stand, Declared != nullptr ? Declared->Identity : "g_03");
        Inspector.Advance(Surface, PropertiesSeat, Declared, *Ordinates, Depot);
    }
    else
    {
        Outliner.Advance(Surface, OutlinerSeat, Forest.Root, 1u,
                         OutlinerComposition{ "World Outliner", "Level_01_City" });

        const PlaneExtent VacantSeat = Spanning(OutlinerSeat.MostAlong, InspectorSeat.LeastAcross,
                                                InspectorAlong - OutlinerAlong, InspectorSeat.SpanAcross());
        Surface.Ground(VacantSeat, Sheet.SunkenGround, 0.0f);
        const char* VacantRun = "Select an entity in the Outliner and press Tab or double-click to view its properties in the Inspector slide.";
        Surface.TextRunClipped(Surface.CentredAlong(VacantSeat, VacantRun, 11.5f) < VacantSeat.LeastAlong + 20.0f ? VacantSeat.LeastAlong + 20.0f : Surface.CentredAlong(VacantSeat, VacantRun, 11.5f),
                                VacantSeat.LeastAcross + VacantSeat.SpanAcross() * 0.5f - 30.0f, VacantRun, Sheet.InkFaint, 11.5f,
                                VacantSeat.SpanAlong() - 40.0f);
    }
}

}   // namespace

//------------------------------------------------------------------------------------------------------------------------
//                                                          ENTRY
//------------------------------------------------------------------------------------------------------------------------

int main(int ArgumentCount, char** Arguments)
{
    const char* DumpPrefix = "Build/Shots/outliner";
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
    VendorStyle.ChildRounding     = 0.0f;
    VendorStyle.PopupRounding     = 9.0f;
    VendorStyle.PopupBorderSize   = 1.0f;
    VendorStyle.ScrollbarSize     = 0.0f;
    VendorStyle.GrabRounding      = 0.0f;
    ImVec4* Colours = VendorStyle.Colors;
    Colours[ImGuiCol_WindowBg]   = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    Colours[ImGuiCol_ChildBg]    = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    Colours[ImGuiCol_PopupBg]    = ImVec4(0.063f, 0.063f, 0.071f, 0.98f);
    Colours[ImGuiCol_Border]     = ImVec4(1.0f, 1.0f, 1.0f, 0.10f);
    Colours[ImGuiCol_FrameBg]    = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

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
                PresentSeat(Surface, Spanning(0.0f, 0.0f, static_cast<float>(DisplayAlong), static_cast<float>(DisplayAcross)),
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
