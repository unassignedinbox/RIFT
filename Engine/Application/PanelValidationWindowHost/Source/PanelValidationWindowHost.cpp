//============================================================================================================================================
//                                                  PANELVALIDATIONWINDOWHOST.CPP
//============================================================================================================================================
// 🧩 The interactive validation seat — the texture-paint and CAD drafting panels in one live window, every widget real.

#include "Contract/DeliveryContract.h"
#include "SlateUI/Interface/InterfaceExchange/Api/InterfaceExchange.h"
#include "SlateUI/Interface/InterfaceExchange/Api/RecordingSurface.h"
#include "SlateVulkan/Device/HostLifecycle/Api/HostLifecycle.h"

#include "SlateUI/Interface/FieldPanel/Api/FieldPanel.h"
#include "SlateUI/Interface/DraftingPanel/Api/DraftingPanel.h"
#include "SlateUI/Interface/IconDepot/Api/IconDepot.h"
#include "SlateUI/Interface/InterfaceSequence/Api/InterfaceSequence.h"
#include "SlateUI/Interface/OutlinerPanel/Api/OutlinerPanel.h"
#include "SlateUI/Interface/PanelExchange/Api/PanelExchange.h"
#include "SlateUI/Interface/PropertiesPanel/Api/PropertiesPanel.h"
#include "SlateUI/Interface/ReferenceSpecification/Api/ReferenceSpecification.h"
#include "SlateUI/Interface/TexturePaintPanel/Api/TexturePaintPanel.h"

#include <cstdio>
#include <cstring>

//------------------------------------------------------------------------------------------------------------------------
//                                                          FIGURES
//------------------------------------------------------------------------------------------------------------------------

namespace
{

using namespace Slate;

constexpr std::uint32_t InitialWidth  = 1700u;   // [px] - three columns of panels
constexpr std::uint32_t InitialHeight = 950u;    // [px]

constexpr const char* WindowTitle = "Slate \u2014 Panel Validation (interactive)";
constexpr const char* HostName    = "PanelValidationWindowHost";

constexpr float DeskClearInk[4] = { 0.039f, 0.039f, 0.043f, 1.0f };   // [-] - #0a0a0b

constexpr float ModeStripAcross = 44.0f;    // [px] - the workspace-mode strip

//------------------------------------------------------------------------------------------------------------------------
//                                              THE TEXTURE-PAINT SEED
//------------------------------------------------------------------------------------------------------------------------

void SeatLayers(Slate::Reference::LayerOrdinates (&Layers)[6])
{
    using Slate::Reference::LayerOrdinates;

    static const char* const EdgeWearChannels[3]  = { "Base Colour", "Roughness", "Metallic" };
    static const char* const DirtPassChannels[2]  = { "Base Colour", "Roughness" };
    static const char* const ScratchesChannels[2] = { "Base Colour", "Bump" };
    static const char* const BaseMetalChannels[4] = { "Base Colour", "Roughness", "Metallic", "Bump" };

    Layers[0] = LayerOrdinates{};
    std::snprintf(Layers[0].Name, sizeof Layers[0].Name, "Edge Wear");
    Layers[0].Content = 0u;  std::snprintf(Layers[0].Transfer, sizeof Layers[0].Transfer, "Multiply");
    Layers[0].Opacity = 78.0;  Layers[0].Shown = true;  Layers[0].PaintPacked = 0xF97316u;  Layers[0].TagPacked = 0xEAB308u;
    Layers[0].Channels = EdgeWearChannels;  Layers[0].ChannelCount = 3u;
    Layers[0].Mask.Enabled = true;  std::snprintf(Layers[0].Mask.Source, sizeof Layers[0].Mask.Source, "Generator");
    Layers[0].Mask.Strength = 92.0;  Layers[0].Mask.Invert = false;  Layers[0].Mask.Shown = true;

    Layers[1] = LayerOrdinates{};
    std::snprintf(Layers[1].Name, sizeof Layers[1].Name, "Dirt Pass");
    Layers[1].Content = 1u;  std::snprintf(Layers[1].Transfer, sizeof Layers[1].Transfer, "Overlay");
    Layers[1].Opacity = 45.0;  Layers[1].Shown = true;  Layers[1].PaintPacked = 0x8B5CF6u;  Layers[1].TagPacked = 0xEC4899u;
    Layers[1].Channels = DirtPassChannels;  Layers[1].ChannelCount = 2u;
    Layers[1].Mask.Enabled = true;  std::snprintf(Layers[1].Mask.Source, sizeof Layers[1].Mask.Source, "Paint");
    Layers[1].Mask.Strength = 100.0;  Layers[1].Mask.Invert = true;  Layers[1].Mask.Shown = true;

    Layers[2] = LayerOrdinates{};
    std::snprintf(Layers[2].Name, sizeof Layers[2].Name, "Scratches");
    Layers[2].Content = 0u;  std::snprintf(Layers[2].Transfer, sizeof Layers[2].Transfer, "Screen");
    Layers[2].Opacity = 60.0;  Layers[2].Shown = false;  Layers[2].PaintPacked = 0xF97316u;  Layers[2].TagPacked = 0x06B6D4u;
    Layers[2].Channels = ScratchesChannels;  Layers[2].ChannelCount = 2u;

    Layers[3] = LayerOrdinates{};
    std::snprintf(Layers[3].Name, sizeof Layers[3].Name, "Base Metal");
    Layers[3].Content = 1u;  std::snprintf(Layers[3].Transfer, sizeof Layers[3].Transfer, "Normal");
    Layers[3].Opacity = 100.0;  Layers[3].Shown = true;  Layers[3].PaintPacked = 0x8B5CF6u;  Layers[3].TagPacked = 0x3B82F6u;
    Layers[3].Channels = BaseMetalChannels;  Layers[3].ChannelCount = 4u;
}

//------------------------------------------------------------------------------------------------------------------------
//                                              THE CAD DRAFTING SEED
//------------------------------------------------------------------------------------------------------------------------

struct DraftingStand
{
    Slate::Reference::OutlinerRowDeclaration Root[1];
    Slate::Reference::OutlinerRowDeclaration Sketches[2];
    Slate::Reference::OutlinerRowDeclaration Bracket[3];
    Slate::Reference::OutlinerRowDeclaration Bodies[3];
    Slate::Reference::OutlinerRowDeclaration Enclosed[2];

    bool ExpandedRoot     = true;
    bool ExpandedSketches = true;
    bool ExpandedBodies   = true;
    bool ExpandedBracket  = true;
    bool HiddenSketches   = false;
    bool HiddenBasePlate  = false;
    bool RevisionFoldOpen = true;

    void Assemble()
    {
        using Slate::Reference::OutlinerRowDeclaration;

        Sketches[0] = { "SK_BasePlate", "r003", Slate::Reference::DirectoryClassification::Sketch,   nullptr, &HiddenBasePlate, nullptr, 0u };
        Sketches[1] = { "SK_BoltHoles", "r004", Slate::Reference::DirectoryClassification::Sketch,   nullptr, nullptr, nullptr, 0u };

        Bracket[0] = { "SOL_Plate",   "r007", Slate::Reference::DirectoryClassification::Solid,    nullptr, nullptr, nullptr, 0u };
        Bracket[1] = { "SOL_Boss",    "r008", Slate::Reference::DirectoryClassification::Cylinder, nullptr, nullptr, nullptr, 0u };
        Bracket[2] = { "SOL_Rib",     "r009", Slate::Reference::DirectoryClassification::Solid,    nullptr, nullptr, nullptr, 0u };

        Bodies[0] = { "BODY_Bracket", "r006", Slate::Reference::DirectoryClassification::Enclosure, &ExpandedBracket, nullptr, Bracket, 3u };
        Bodies[1] = { "SOL_Housing",  "r010", Slate::Reference::DirectoryClassification::Solid,     nullptr, nullptr, nullptr, 0u };
        Bodies[2] = { "SOL_Dome",     "r011", Slate::Reference::DirectoryClassification::Sphere,    nullptr, nullptr, nullptr, 0u };

        Enclosed[0] = { "Sketches", "r002", Slate::Reference::DirectoryClassification::Enclosure, &ExpandedSketches, &HiddenSketches, Sketches, 2u };
        Enclosed[1] = { "Bodies",   "r005", Slate::Reference::DirectoryClassification::Enclosure, &ExpandedBodies,   nullptr,               Bodies,   3u };

        Root[0] = { "Part", "r001", Slate::Reference::DirectoryClassification::Scene, &ExpandedRoot, nullptr, Enclosed, 2u };
    }

    const Slate::Reference::OutlinerRowDeclaration* Find(const char* Identity) const
    {
        const Slate::Reference::OutlinerRowDeclaration* Stacks[4] = { Root, Enclosed, Bodies, Bracket };
        const std::uint32_t Counts[4] = { 1u, 2u, 3u, 3u };
        for (std::uint32_t StackOrdinal = 0u; StackOrdinal < 4u; ++StackOrdinal)
            for (std::uint32_t Ordinal = 0u; Ordinal < Counts[StackOrdinal]; ++Ordinal)
            {
                const Slate::Reference::OutlinerRowDeclaration& Row = Stacks[StackOrdinal][Ordinal];
                if (std::strcmp(Row.Identity, Identity) == 0)
                    return &Row;
                for (std::uint32_t Inner = 0u; Inner < Row.EnclosureCount; ++Inner)
                    if (std::strcmp(Row.Enclosed[Inner].Identity, Identity) == 0)
                        return &Row.Enclosed[Inner];
            }
        return nullptr;
    }
};

struct RevisionStand
{
    static constexpr std::uint32_t RevisionCapacity = 32u;   // [-]

    bool Folds[RevisionCapacity] = {};
    Slate::Reference::RevisionDeclaration Revisions[RevisionCapacity];
    std::uint32_t Count = 0u;

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
            Revisions[Seated++] = { Tokens[Ordinal], Slate::Reference::RevisionCategory::Start,  CreatedRuns[Ordinal], "Initial state",
                                    "", "System", "", "09:14", "2026-08-17", &Folds[Seated] };
            Revisions[Seated++] = { Tokens[Ordinal], Slate::Reference::RevisionCategory::Create, "Added to scene", "Inserted at origin",
                                    "", "System", "", "09:19", "2026-08-17", &Folds[Seated] };
        }

        Revisions[Seated++] = { "r008", Slate::Reference::RevisionCategory::Parameter, "Adjusted radius", "Radius 6.25 \xE2\x86\x92 6.75",
                                "Increased radius to match new constraints.", "Alex Chen", "6.75", "10:42", "2026-08-17",
                                &Stand.RevisionFoldOpen };
        Revisions[Seated++] = { "r008", Slate::Reference::RevisionCategory::Feature, "Draft angle applied", "Draft angle 3\xC2\xB0",
                                "Customer requested smoother finish.", "Sam Rivera", "3.0", "11:05", "2026-08-17",
                                &Folds[Seated] };
        Revisions[Seated++] = { "r003", Slate::Reference::RevisionCategory::Sketch, "Profile constrained", "12 constraints",
                                "Approximated spline from DXF import.", "Maria Rossi", "", "11:31", "2026-08-17",
                                &Folds[Seated] };
        Count = Seated;
    }
};

/// 🧩 Copies the device handles across the layer seam into the attachment the interface declares.
InterfaceAttachment Attach(const DeviceOffering& Offered)
{
    InterfaceAttachment Arriving = {};

    Arriving.Instance                 = Offered.Instance;
    Arriving.ScoredDevice             = Offered.ScoredDevice;
    Arriving.ActiveDevice             = Offered.ActiveDevice;
    Arriving.GraphicsQueue            = Offered.GraphicsQueue;
    Arriving.GraphicsFamilyOrdinal    = Offered.GraphicsFamilyOrdinal;
    Arriving.ColourTargetFormat       = Offered.ColourTargetFormat;
    Arriving.MinimumDisplayImageCount = Offered.MinimumDisplayImageCount;
    Arriving.DisplayImageCount        = Offered.DisplayImageCount;
    Arriving.NativeWindowSlot         = Offered.NativeWindowSlot;

    return Arriving;
}

}   // namespace

//------------------------------------------------------------------------------------------------------------------------
//                                                          ENTRY
//------------------------------------------------------------------------------------------------------------------------

int main()
{
    using namespace Slate;

    HostDeclaration Declared;
    Declared.Naming              = HostName;
    Declared.WindowCaption       = WindowTitle;
    Declared.InitialWidth        = InitialWidth;
    Declared.InitialHeight       = InitialHeight;
    Declared.Pacing              = LatencyIntent::SteadyPacing;
    Declared.DiagnosticRequested = true;

    HostLifecycle Lifetime;

    if (!Lifetime.Construct(Declared).ContentPresent)
        return 1;

    InterfaceExchange Interface;

    if (!Interface.Construct(Attach(Lifetime.Offering())).ContentPresent)
    {
        std::printf("%s \u2014 the interface context was refused\n", HostName);
        return 1;
    }

    RecordingSurface Surface;

    Slate::Reference::IconDepot Depot;

    if (!Depot.Construct().ContentPresent())
    {
        std::printf("%s \u2014 the glyph depot was refused\n", HostName);
        return 1;
    }
    Depot.SeatVectorGlyph();

    // ① The seated ordinates — everything the panels present, owned here, written through by hand.
    Slate::Reference::LayerOrdinates Layers[6];
    SeatLayers(Layers);

    Slate::Reference::ChannelOrdinates Channels;
    Slate::Reference::SeatChannelOrdinates(Channels);

    Slate::Reference::MaskOrdinates MaskSheet;

    DraftingStand Drafting;
    Drafting.Assemble();
    RevisionStand Revisions;
    Revisions.Assemble(Drafting);

    Slate::Reference::LayerStackPanel StackPanel;
    Slate::Reference::ChannelPropertyPanel ChannelPanel;
    Slate::Reference::MaskPropertyPanel MaskPanel;
    Slate::Reference::OutlinerPanel Directory;
    Slate::Reference::DraftingPanel DraftingSeat;
    Slate::Reference::PropertiesPanel Properties;
    Slate::Reference::ProfileOrdinates Profile;

    const Slate::Reference::OutlinerRowDeclaration* Inspected = Drafting.Find("r008");
    if (Inspected != nullptr)
        Slate::Reference::SeatProfile(Profile, *Inspected);
    Directory.SeatTaken("r008");

    std::uint32_t WorkspaceMode = 0u;   // [-] - 0 texture paint, 1 CAD drafting
    static const char* const ModeCaptions[2] = { "Texture Paint", "CAD Drafting" };

    while (Lifetime.Standing())
    {
        const TickPass Pass = Lifetime.Await(DeskClearInk);

        if (Pass.Standing == TickStanding::Closed)
            break;

        if (Lifetime.DeviceRecovered())
        {
            Interface.Reclaim();

            if (!Interface.Construct(Attach(Lifetime.Offering())).ContentPresent)
            {
                std::printf("%s \u2014 the interface could not be rebuilt on the recovered device\n", HostName);
                break;
            }
            static_cast<void>(Lifetime.DisplayRecovered());
        }
        else if (Lifetime.DisplayRecovered())
        {
            const DeviceOffering Offered = Lifetime.Offering();
            if (!Interface.Renegotiate(Offered.MinimumDisplayImageCount, Offered.DisplayImageCount))
            {
                std::printf("%s \u2014 the interface declined the restated image counts\n", HostName);
            }
        }

        if (Pass.Standing != TickStanding::Recording)
            continue;

        bool ContentBuilt = Interface.Advance().ContentPresent;

        if (ContentBuilt && !Surface.Adopt().ContentPresent)
        {
            Disregard(Interface.Abandon());
            ContentBuilt = false;
        }

        if (ContentBuilt)
        {
            const DisplayCondition& Display = Surface.Display();

            // ② 🔴 The seat window — every panel's widgets stand in it, so every control below is live.
            Slate::Reference::PanelExchange PanelRecordSurface;
            if (PanelRecordSurface.Adopt(Slate::Reference::PanelExchange::ShellLayer::Beneath).ContentPresent() &&
                Slate::Reference::InterfaceSequence::OpenSeatWindow(Display.ExtentAlong, Display.ExtentAcross).ContentPresent())
            {
                Slate::Reference::WorkspaceInk Sheet;
                PanelRecordSurface.Ground(Slate::Reference::PlaneExtent{ 0.0f, 0.0f, Display.ExtentAlong, Display.ExtentAcross },
                                          Sheet.DeskGround, 0.0f);

                // ③ The workspace-mode strip — the same segment the reference seats.
                const Slate::Reference::ControlSheet Controls = Slate::Reference::ControlSheetFromWorkspace(Sheet);
                Slate::Reference::ControlRowDeclaration ModeRow = { "", 88.0f, 13.5f };
                PanelRecordSurface.Ground(Slate::Reference::PlaneExtent{ 16.0f, 12.0f, 336.0f, ModeStripAcross }, Sheet.SunkenGround, 10.0f);
                Slate::Reference::PresentSegmentRow(PanelRecordSurface,
                                                    Slate::Reference::PlaneExtent{ 16.0f, 12.0f, 336.0f, ModeStripAcross },
                                                    ModeRow, ModeCaptions, 2u, WorkspaceMode, Controls, "validation.mode");

                if (WorkspaceMode == 0u)
                {
                    // ④ Texture paint — the stack, the channel seat, the mask seat, all live.
                    StackPanel.Advance(PanelRecordSurface,
                                       Slate::Reference::PlaneExtent{ 16.0f, ModeStripAcross + 20.0f, 500.0f, Display.ExtentAcross - ModeStripAcross - 36.0f },
                                       Layers, 4u, Depot);
                    ChannelPanel.Advance(PanelRecordSurface,
                                         Slate::Reference::PlaneExtent{ 528.0f, ModeStripAcross + 20.0f, 560.0f, Display.ExtentAcross - ModeStripAcross - 36.0f },
                                         Channels, Depot);
                    MaskPanel.Advance(PanelRecordSurface,
                                      Slate::Reference::PlaneExtent{ 1100.0f, ModeStripAcross + 20.0f, 580.0f, Display.ExtentAcross - ModeStripAcross - 36.0f },
                                      MaskSheet, Depot);
                }
                else
                {
                    // ⑤ CAD drafting — the directory with metadata beside the properties/history carousel.
                    DraftingSeat.Advance(PanelRecordSurface,
                                         Slate::Reference::PlaneExtent{ 16.0f, ModeStripAcross + 20.0f, 820.0f, Display.ExtentAcross - ModeStripAcross - 36.0f },
                                         Directory, Drafting.Root, 1u, Inspected, Profile, Depot);
                    Properties.Advance(PanelRecordSurface,
                                       Slate::Reference::PlaneExtent{ 848.0f, ModeStripAcross + 20.0f, 836.0f, Display.ExtentAcross - ModeStripAcross - 36.0f },
                                       Inspected, Profile, Depot, Revisions.Revisions, Revisions.Count, Drafting.Root, 1u);
                }

                Slate::Reference::InterfaceSequence::CloseSeatWindow();
                PanelRecordSurface.Seal();
            }

            Surface.Retire();

            if (Interface.Seal().ContentPresent)
            {
                if (!Interface.Record(Pass.Recording))
                {
                    std::printf("%s \u2014 the interface content was not recorded\n", HostName);
                }
            }
            else
            {
                Disregard(Interface.Abandon());
            }
        }

        if (!Lifetime.Surrender().ContentPresent)
            break;
    }

    const std::uint32_t Serious = Lifetime.StateDiagnostics();

    Surface.Reset();
    Interface.Reclaim();
    Lifetime.Reclaim();

    std::printf("%s \u2014 exited cleanly\n", HostName);
    return (Serious == 0u) ? 0 : 1;
}
