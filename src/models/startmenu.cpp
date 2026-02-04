#include "startmenu.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

StartMenu::StartMenu()
    : startGameRequested(false), exitGameRequested(false)
{
}

StartMenu::~StartMenu()
{
}

void StartMenu::init(GLFWwindow* window, const char* glsl_version)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
}

void StartMenu::shutdown()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void StartMenu::beginFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

bool StartMenu::render(int windowWidth, int windowHeight)
{
    ImGui::SetNextWindowPos(ImVec2(windowWidth / 2.0f - 200, windowHeight / 2.0f - 100), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_Once);

    ImGui::Begin("Main Menu", nullptr, 
                 ImGuiWindowFlags_NoTitleBar | 
                 ImGuiWindowFlags_NoResize | 
                 ImGuiWindowFlags_NoMove | 
                 ImGuiWindowFlags_NoBackground);

    ImGui::Text("Main Menu");
    
    if (ImGui::Button("Start Game", ImVec2(200, 50)))
    {
        startGameRequested = true;
    }
    
    ImGui::SameLine();
    
    if (ImGui::Button("Exit", ImVec2(200, 50)))
    {
        exitGameRequested = true;
    }

    ImGui::End();
    
    return startGameRequested || exitGameRequested;
}

void StartMenu::endFrame()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void StartMenu::reset()
{
    startGameRequested = false;
    exitGameRequested = false;
}
