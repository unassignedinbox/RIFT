//============================================================================================================================================
//                                                      OUTLINERWINDOWHOST.CPP
//============================================================================================================================================
// 🧩 The interactive standalone directory — a real platform window; click rows, filter, ctrl-select, inspect.

#include "SlateUI/Interface/IconDepot/Api/IconDepot.h"
#include "SlateUI/Interface/InterfaceSequence/Api/InterfaceSequence.h"
#include "SlateUI/Interface/OutlinerPanel/Api/OutlinerPanel.h"
#include "SlateUI/Interface/PanelExchange/Api/PanelExchange.h"
#include "SlateUI/Interface/ThemeSpecification/Api/ThemeSpecification.h"

#include <cstdint>
#include <cstdio>

//------------------------------------------------------------------------------------------------------------------------
//                                                    THE SEED FOREST
//------------------------------------------------------------------------------------------------------------------------

namespace
{

using namespace Rift;

constexpr float WindowAlong     = 400.0f;   // [px]
constexpr float WindowAcross    = 760.0f;   // [px]
constexpr float DirectoryAlong  = 350.0f;   // [px] - the reference's directory column

struct SeedStand
{
    bool ExpandedRoot     = true;
    bool ExpandedSketches = true;
    bool ExpandedBodies   = true;
    bool ExpandedBracket  = true;
    bool HiddenSketches   = false;
    bool HiddenBasePlate  = false;
};

struct ForestStand
{
    OutlinerRowDeclaration Root[1];
    OutlinerRowDeclaration Sketches[2];
    OutlinerRowDeclaration Bracket[3];
    OutlinerRowDeclaration Bodies[3];
    OutlinerRowDeclaration Enclosed[2];
};

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

}   // namespace

//------------------------------------------------------------------------------------------------------------------------
//                                                          ENTRY
//------------------------------------------------------------------------------------------------------------------------

int main()
{
    InterfaceSequence::SeatFaultReporter();

    if (!InterfaceSequence::AdoptWindowed(WindowAlong, WindowAcross, "RIFT \u2014 Directory (scene outliner)").ContentPresent())
    {
        std::fprintf(stderr, "OutlinerWindowHost: the window refused to adopt\n");
        return 1;
    }

    IconDepot Depot;
    if (!Depot.Construct().ContentPresent())
    {
        std::fprintf(stderr, "OutlinerWindowHost: the glyph depot refused to construct\n");
        return 1;
    }
    InterfaceSequence::SeatGlyphPicture(Depot);

    OutlinerPanel Directory;
    SeedStand Stand;
    ForestStand Forest;
    AssembleForest(Stand, Forest);
    Directory.SeatTaken("r007");

    while (InterfaceSequence::WindowStanding())
    {
        if (!InterfaceSequence::BeginWindowTick().ContentPresent())
            break;

        PanelExchange Surface;
        if (Surface.Adopt(PanelExchange::ShellLayer::Beneath).ContentPresent())
        {
            ThemeSpecification Sheet;
            Surface.Ground(PlaneExtent{ 0.0f, 0.0f, WindowAlong, WindowAcross }, Sheet.DeskGround, 0.0f);
            const float Margin = (WindowAlong - DirectoryAlong) * 0.5f;
            Directory.Advance(Surface, PlaneExtent{ Margin, 20.0f, Margin + DirectoryAlong, WindowAcross - 40.0f },
                              Forest.Root, 1u, OutlinerComposition{ "Directory", "Bracket_Rev4" }, Depot);
            if (Directory.InspectRaised)
                Directory.InspectRaised = false;   // 📝 the windowed seat inspects in place; nothing slides
            Surface.Seal();
        }
        InterfaceSequence::EndWindowTick();
    }

    InterfaceSequence::DismissWindowed();
    return 0;
}
