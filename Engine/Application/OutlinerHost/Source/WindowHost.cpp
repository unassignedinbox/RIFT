//============================================================================================================================================
//                                                           WINDOWHOST.CPP
//============================================================================================================================================
// 🧩 The interactive standalone outliner — a GLFW window presenting the same seat the headless host dumps.
// note  Built only where GLFW and an OpenGL development package stand (`make outliner-window`); the headless
//       build never compiles this file, exactly as the sandbox build does not.

#include "Engine/Application/OutlinerHost/Api/WorldEditorSeat.h"
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

    GLFWwindow* Window = glfwCreateWindow(1500, 860, "RIFT \u2014 World Outliner (standalone)", nullptr, nullptr);
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

    OutlinerPanel Outliner;
    Outliner.Construct(Depot);
    EntryInspectorPanel Inspector;

    SeedStand Stand;
    ForestStand Forest;
    AssembleForest(Stand, Forest);

    std::snprintf(Outliner.TakenIdentity, sizeof Outliner.TakenIdentity, "g_03");

    bool SlideOpen = false;

    while (glfwWindowShouldClose(Window) == GLFW_FALSE)
    {
        glfwPollEvents();
        if (glfwGetKey(Window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(Window, GLFW_TRUE);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ① Tab summons the inspector slide, exactly as the reference binds it.
        ImGuiIO& VendorIO = ImGui::GetIO();
        if (VendorIO.KeysDown[ImGuiKey_Tab] && !VendorIO.KeysDownDurationPrevious[ImGuiKey_Tab] &&
            ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow) == false)
            SlideOpen = !SlideOpen;

        int FramebufferAlong = 0;
        int FramebufferAcross = 0;
        glfwGetFramebufferSize(Window, &FramebufferAlong, &FramebufferAcross);

        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(ImVec2(static_cast<float>(FramebufferAlong), static_cast<float>(FramebufferAcross)));
        ImGui::Begin("RIFT \u2014 World Outliner", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollbar);

        RecordingSurface Surface;
        if (Surface.Adopt(RecordingSurface::ShellLayer::Beneath).ContentPresent())
        {
            PresentWorldEditorSeat(Surface,
                                   Spanning(0.0f, 0.0f, static_cast<float>(FramebufferAlong), static_cast<float>(FramebufferAcross)),
                                   Outliner, Inspector, Depot, Stand, Forest, SlideOpen);
            if (Outliner.InspectRaised)
                SlideOpen = true;
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
