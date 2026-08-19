//============================================================================================================================================
//                                                      OUTLINERWINDOWHOST.CPP
//============================================================================================================================================
// 🧩 The interactive scene directory — HostLifecycle's own window and Vulkan seat, the InterfaceValidationHost pattern, Slate::Reference panels inside.

#include "Contract/DeliveryContract.h"
#include "SlateUI/Interface/InterfaceExchange/Api/InterfaceExchange.h"
#include "SlateUI/Interface/InterfaceExchange/Api/RecordingSurface.h"
#include "SlateVulkan/Device/HostLifecycle/Api/HostLifecycle.h"

#include "SlateUI/Interface/IconDepot/Api/IconDepot.h"
#include "SlateUI/Interface/InterfaceSequence/Api/InterfaceSequence.h"
#include "SlateUI/Interface/OutlinerPanel/Api/OutlinerPanel.h"
#include "SlateUI/Interface/PropertiesPanel/Api/PropertiesPanel.h"
#include "SlateUI/Interface/PanelExchange/Api/PanelExchange.h"
#include "SlateUI/Interface/ThemeSpecification/Api/ThemeSpecification.h"

#include <cstdio>
#include <cstring>

//------------------------------------------------------------------------------------------------------------------------
//                                                          FIGURES
//------------------------------------------------------------------------------------------------------------------------

namespace
{

using namespace Slate;

constexpr std::uint32_t InitialWidth  = 980u;    // [px] - the directory column with the inspector beside it
constexpr std::uint32_t InitialHeight = 780u;    // [px]

constexpr const char* WindowTitle = "Slate \u2014 Directory (scene outliner)";
constexpr const char* HostName    = "OutlinerWindowHost";

constexpr float DirectoryAlong = 350.0f;   // [px] - the reference's directory column

// 📝 The desk ground, #0a0a0b, as the clear ink Await paces every frame with.
constexpr float DeskClearInk[4] = { 0.039f, 0.039f, 0.043f, 1.0f };   // [-]

//------------------------------------------------------------------------------------------------------------------------
//                                                    THE SEED FOREST
//------------------------------------------------------------------------------------------------------------------------

struct SeedStand
{
    bool ExpandedRoot     = true;    // [-] - r001
    bool ExpandedSketches = true;    // [-] - r002
    bool ExpandedBodies   = true;    // [-] - r005
    bool ExpandedBracket  = true;    // [-] - r006
    bool HiddenSketches   = false;   // [-] - r002
    bool HiddenBasePlate  = false;   // [-] - r003
};

struct ForestStand
{
    Slate::Reference::OutlinerRowDeclaration Root[1];          // [-] - r001 Part
    Slate::Reference::OutlinerRowDeclaration Sketches[2];      // [-] - r003, r004
    Slate::Reference::OutlinerRowDeclaration Bracket[3];       // [-] - r007..r009
    Slate::Reference::OutlinerRowDeclaration Bodies[3];        // [-] - r006, r010, r011
    Slate::Reference::OutlinerRowDeclaration Enclosed[2];      // [-] - r002 Sketches, r005 Bodies
};

void AssembleForest(SeedStand& Stand, ForestStand& Forest)
{
    using namespace Slate::Reference;

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

struct RevisionStand
{
    static constexpr std::uint32_t RevisionCapacity = 32u;   // [-]

    bool Folds[RevisionCapacity] = {};
    Slate::Reference::RevisionDeclaration Revisions[RevisionCapacity];
    std::uint32_t Count = 0u;

    void Assemble()
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

        Revisions[Seated++] = { "r008", Slate::Reference::RevisionCategory::Parameter, "Adjusted radius", "Radius 6.25 → 6.75",
                                "Increased radius to match new constraints.", "Alex Chen", "6.75", "10:42", "2026-08-17",
                                &Folds[Seated] };
        Revisions[Seated++] = { "r008", Slate::Reference::RevisionCategory::Feature, "Draft angle applied", "Draft angle 3°",
                                "Customer requested smoother finish.", "Sam Rivera", "3.0", "11:05", "2026-08-17",
                                &Folds[Seated] };
        Revisions[Seated++] = { "r003", Slate::Reference::RevisionCategory::Sketch, "Profile constrained", "12 constraints",
                                "Approximated spline from DXF import.", "Maria Rossi", "", "11:31", "2026-08-17",
                                &Folds[Seated] };
        Count = Seated;
    }
};

/// 🧩 Finds the row carrying the identity, through the whole forest.
const Slate::Reference::OutlinerRowDeclaration* FindRow(const ForestStand& Forest, const char* Identity)
{
    const Slate::Reference::OutlinerRowDeclaration* Stacks[5] = { Forest.Root, Forest.Enclosed, Forest.Bodies, Forest.Bracket, Forest.Sketches };
    const std::uint32_t Counts[5] = { 1u, 2u, 3u, 3u, 2u };
    for (std::uint32_t StackOrdinal = 0u; StackOrdinal < 5u; ++StackOrdinal)
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

}   // namespace

//------------------------------------------------------------------------------------------------------------------------
//                                                          ENTRY
//------------------------------------------------------------------------------------------------------------------------

int main()
{
    using namespace Slate;

    // ① The lifetimes — window, instance, surface, device, chain, slots, recordings — HostLifecycle's own.
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

    // ② The interface seam over the same Vulkan lifetimes.
    InterfaceExchange Interface;

    if (!Interface.Construct(Attach(Lifetime.Offering())).ContentPresent)
    {
        std::printf("%s \u2014 the interface context was refused\n", HostName);
        return 1;
    }

    RecordingSurface Surface;

    // ③ The Slate::Reference seat — the glyph drawn as primitives, the directory as the reference seats it.
    Slate::Reference::IconDepot Depot;

    if (!Depot.Construct().ContentPresent())
    {
        std::printf("%s \u2014 the glyph depot was refused\n", HostName);
        return 1;
    }
    Depot.SeatVectorGlyph();

    Slate::Reference::OutlinerPanel Directory;
    Slate::Reference::PropertiesPanel Inspector;
    Slate::Reference::ProfileOrdinates Profile;
    RevisionStand Revisions;
    Revisions.Assemble();
    char InspectedIdentity[16] = "r007";
    const Slate::Reference::OutlinerRowDeclaration* Inspected = nullptr;
    SeedStand Stand;
    ForestStand Forest;
    AssembleForest(Stand, Forest);
    Inspected = FindRow(Forest, InspectedIdentity);
    if (Inspected != nullptr)
        Slate::Reference::SeatProfile(Profile, *Inspected);
    Directory.SeatTaken("r007");

    // ④ The paced loop — Await acquires and opens, Surrender submits and presents.
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

            // ①① 🔴 The seat window is where the panels' widgets stand — without it the directory is a
            //     painting: visible, but no row, eye or filter field can receive focus or the pointer.
            Slate::Reference::PanelExchange PanelRecordSurface;
            if (PanelRecordSurface.Adopt(Slate::Reference::PanelExchange::ShellLayer::Beneath).ContentPresent() &&
                Slate::Reference::InterfaceSequence::OpenSeatWindow(Display.ExtentAlong, Display.ExtentAcross).ContentPresent())
            {
                Slate::Reference::WorkspaceInk Sheet;
                PanelRecordSurface.Ground(Slate::Reference::PlaneExtent{ 0.0f, 0.0f, Display.ExtentAlong, Display.ExtentAcross },
                                   Sheet.DeskGround, 0.0f);
                // ①① The live seat: the directory left, the record inspector right. Selection drives the
                //     inspector every tick; a double press raises it the same way Tab does in the reference.
                Directory.Advance(PanelRecordSurface,
                                  Slate::Reference::PlaneExtent{ 20.0f, 20.0f, 20.0f + DirectoryAlong, Display.ExtentAcross - 40.0f },
                                  Forest.Root, 1u, Slate::Reference::OutlinerComposition{ "Directory", "Bracket_Rev4" }, Depot);

                if (Directory.TakenCount > 0u &&
                    std::strcmp(Directory.TakenIdentities[0], InspectedIdentity) != 0)
                {
                    std::snprintf(InspectedIdentity, sizeof InspectedIdentity, "%s", Directory.TakenIdentities[0]);
                    Inspected = FindRow(Forest, InspectedIdentity);
                    if (Inspected != nullptr)
                        Slate::Reference::SeatProfile(Profile, *Inspected);
                }
                if (Directory.InspectRaised)
                    Directory.InspectRaised = false;   // 📝 the inspector already stands beside; the raise is honoured

                if (Inspected != nullptr)
                    Inspector.Advance(PanelRecordSurface,
                                      Slate::Reference::PlaneExtent{ 20.0f + DirectoryAlong + 20.0f, 20.0f,
                                                                    Display.ExtentAlong - 40.0f, Display.ExtentAcross - 40.0f },
                                      Inspected, Profile, Depot, Revisions.Revisions, Revisions.Count, Forest.Root, 1u);
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

    // ⑤ Reclamation — the interface retires before the lifetimes it was constructed over.
    const std::uint32_t Serious = Lifetime.StateDiagnostics();

    Surface.Reset();
    Interface.Reclaim();
    Lifetime.Reclaim();

    std::printf("%s \u2014 exited cleanly\n", HostName);
    return (Serious == 0u) ? 0 : 1;
}
