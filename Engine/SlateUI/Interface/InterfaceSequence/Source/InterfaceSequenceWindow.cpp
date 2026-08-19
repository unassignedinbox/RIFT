//============================================================================================================================================
//                                                    INTERFACESEQUENCEWINDOW.CPP
//============================================================================================================================================
// 🧩 The interactive seat — a platform window driving the vendored context, translated wholly inside SlateUI.

#include "SlateUI/Interface/InterfaceSequence/Api/InterfaceSequence.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include <GLFW/glfw3.h>

#include <cstdint>
#include <cstdio>

namespace Rift
{

namespace
{

GLFWwindow* StandingWindow = nullptr;   // [-] - the one windowed seat

/// 🧩 Closes the window when the escape key is pressed.
void CloseOnEscape(GLFWwindow* Window, int Key, int Scancode, int Action, int Mods)
{
    (void)Scancode;  (void)Mods;
    if (Key == GLFW_KEY_ESCAPE && Action == GLFW_PRESS)
        glfwSetWindowShouldClose(Window, GLFW_TRUE);
}

}   // namespace

Deliver<bool> InterfaceSequence::AdoptWindowed(double DisplayAlong, double DisplayAcross, const char* TitleRun)
{
    if (glfwInit() != GLFW_TRUE)
        return Deliver<bool>::Refuse({ RefusalReason::CapabilityAbsent, "the platform window library refused to initialise" });

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    StandingWindow = glfwCreateWindow(static_cast<int>(DisplayAlong), static_cast<int>(DisplayAcross),
                                      TitleRun != nullptr ? TitleRun : "RIFT", nullptr, nullptr);
    if (StandingWindow == nullptr)
        return Deliver<bool>::Refuse({ RefusalReason::CapabilityAbsent, "the window refused to open" });

    glfwMakeContextCurrent(StandingWindow);
    glfwSwapInterval(1);
    glfwSetKeyCallback(StandingWindow, CloseOnEscape);

    Adopt(DisplayAlong, DisplayAcross);
    ImGui_ImplGlfw_InitForOpenGL(StandingWindow, true);
    ImGui_ImplOpenGL3_Init();
    return Deliver<bool>::Delivered(true);
}

bool InterfaceSequence::WindowStanding()
{
    return StandingWindow != nullptr && glfwWindowShouldClose(StandingWindow) == GLFW_FALSE;
}

Deliver<bool> InterfaceSequence::BeginWindowTick()
{
    if (StandingWindow == nullptr)
        return Deliver<bool>::Refuse({ RefusalReason::CapabilityAbsent, "no window stands adopted" });

    glfwPollEvents();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    OpenTick();
    return Deliver<bool>::Delivered(true);
}

void InterfaceSequence::EndWindowTick()
{
    if (StandingWindow == nullptr)
        return;

    SealTick();
    ImGui_ImplOpenGL3_RenderDrawData(static_cast<ImDrawData*>(SealTickDrawData()));
    glfwSwapBuffers(StandingWindow);
}

void InterfaceSequence::SeatGlyphPicture(const IconDepot& Depot)
{
    if (StandingWindow == nullptr)
        return;

    unsigned int GlyphTexture = 0u;
    glGenTextures(1, &GlyphTexture);
    glBindTexture(GL_TEXTURE_2D, GlyphTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, static_cast<int>(IconDepot::GlyphExtent),
                 static_cast<int>(IconDepot::GlyphExtent), 0, GL_RGBA, GL_UNSIGNED_BYTE, Depot.PictureOrdinates());
    const_cast<IconDepot&>(Depot).AdoptIdentity(reinterpret_cast<void*>(static_cast<std::uintptr_t>(GlyphTexture)));
}

void InterfaceSequence::DismissWindowed()
{
    if (StandingWindow == nullptr)
        return;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    Dismiss();
    glfwDestroyWindow(StandingWindow);
    StandingWindow = nullptr;
    glfwTerminate();
}

}   // namespace Rift
