//============================================================================================================================================
//                                                           WINDOWHOST.CPP
//============================================================================================================================================
// 🧩 The interactive standalone directory — a GLFW window presenting the same seat the headless host dumps.
// note  Built only where GLFW and an OpenGL development package stand (`make outliner-window`); the headless
//       build never compiles this file, exactly as the sandbox build does not.

#include "Engine/SlateUI/Interface/IconDepot/Api/IconDepot.h"
#include "Engine/SlateUI/Interface/OutlinerPanel/Api/OutlinerPanel.h"
#include "Engine/SlateUI/Interface/RecordingSurface/Api/RecordingSurface.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include <GLFW/glfw3.h>

#include <cstdint>
#include <cstdio>
#include <cstring>

namespace Rift
{

/// 🧩 Constructs the context, the default typeface at three crisp sizes, and the styled window chrome.
/// tag   internal
void ConstructInterfaceContext();

}   // namespace Rift

//------------------------------------------------------------------------------------------------------------------------
//                                                    THE SEED FOREST
//------------------------------------------------------------------------------------------------------------------------

namespace
{

using namespace Slate;

constexpr float DirectoryAlong  = 350.0f;   // [px] - the reference's directory column

/// 🧩 The seed forest, verbatim from the reference's initialStore — Bracket_Rev4.
/// tag   internal
struct ForestStand
{
    OutlinerRowDeclaration Root[1];
    OutlinerRowDeclaration Sketches[2];
    OutlinerRowDeclaration Bracket[3];
    OutlinerRowDeclaration Bodies[3];
    OutlinerRowDeclaration Enclosed[2];
};

/// 🧩 The host-owned disclosures and presences the forest borrows.
/// tag   internal
struct SeedStand
{
    bool ExpandedRoot     = true;
    bool ExpandedSketches = true;
    bool ExpandedBodies   = true;
    bool ExpandedBracket  = true;
    bool HiddenSketches   = false;
    bool HiddenBasePlate  = false;
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

}   // namespace

//------------------------------------------------------------------------------------------------------------------------
//                                                          ENTRY
//------------------------------------------------------------------------------------------------------------------------

int main()
{
    using namespace Slate;
    using namespace Rift;

    if (glfwInit() != GLFW_TRUE)
    {
        std::fprintf(stderr, "WindowHost: GLFW refused to initialise\n");
        return 1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWwindow* Window = glfwCreateWindow(400, 760, "RIFT \u2014 Directory (standalone outliner)", nullptr, nullptr);
    if (Window == nullptr)
    {
        std::fprintf(stderr, "WindowHost: the window refused to open\n");
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(Window);
    glfwSwapInterval(1);

    ConstructInterfaceContext();
    ImGui_ImplGlfw_InitForOpenGL(Window, true);
    ImGui_ImplOpenGL3_Init();

    // ① The glyph uploads once; the depot adopts the platform identity over the raster one.
    IconDepot Depot;
    Depot.Construct();
    GLuint GlyphTexture = 0u;
    glGenTextures(1, &GlyphTexture);
    glBindTexture(GL_TEXTURE_2D, GlyphTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, static_cast<GLsizei>(IconDepot::GlyphExtent),
                 static_cast<GLsizei>(IconDepot::GlyphExtent), 0, GL_RGBA, GL_UNSIGNED_BYTE, Depot.PictureOrdinates());
    Depot.AdoptIdentity(reinterpret_cast<void*>(static_cast<std::uintptr_t>(GlyphTexture)));

    OutlinerPanel Directory;
    SeedStand Stand;
    ForestStand Forest;
    AssembleForest(Stand, Forest);
    Directory.SeatTaken("r007");

    while (glfwWindowShouldClose(Window) == GLFW_FALSE)
    {
        glfwPollEvents();
        if (glfwGetKey(Window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(Window, GLFW_TRUE);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        int FramebufferAlong = 0;
        int FramebufferAcross = 0;
        glfwGetFramebufferSize(Window, &FramebufferAlong, &FramebufferAcross);

        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(ImVec2(static_cast<float>(FramebufferAlong), static_cast<float>(FramebufferAcross)));
        ImGui::Begin("RIFT \u2014 Directory", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollbar);

        RecordingSurface Surface;
        if (Surface.Adopt(RecordingSurface::ShellLayer::Beneath).ContentPresent())
        {
            WorkspaceInk Sheet;
            const float DeskAlong = static_cast<float>(FramebufferAlong);
            const float DeskAcross = static_cast<float>(FramebufferAcross);
            Surface.Ground(Spanning(0.0f, 0.0f, DeskAlong, DeskAcross), Sheet.DeskGround, 0.0f);
            const float Margin = (DeskAlong - DirectoryAlong) * 0.5f;
            Directory.Advance(Surface, Spanning(Margin, 20.0f, DirectoryAlong, DeskAcross - 40.0f),
                              Forest.Root, 1u, OutlinerComposition{ "Directory", "Bracket_Rev4" }, Depot);
            Surface.Seal();
        }

        ImGui::End();
        ImGui::Render();

        glViewport(0, 0, FramebufferAlong, FramebufferAcross);
        glClearColor(0.039f, 0.039f, 0.043f, 1.0f);   // [-] - --desk #0a0a0b
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(Window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glDeleteTextures(1, &GlyphTexture);
    glfwDestroyWindow(Window);
    glfwTerminate();
    return 0;
}
