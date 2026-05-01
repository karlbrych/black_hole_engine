#include "startmenu.h"
#include "core.h"
#include <GLFW/glfw3.h>
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

bool StartMenu::render(int windowWidth, int windowHeight, Plane* plane)
{
    ImGui::SetNextWindowSize(ImVec2((float)windowWidth, (float)windowHeight), ImGuiCond_Once);
    ImGui::SetNextWindowPos(ImVec2(windowWidth / 2.0f - 630 , windowHeight/ 2.0f - 150), ImGuiCond_Once); //vycentrovani


    ImGui::Begin("Main Menu", nullptr, 
                 ImGuiWindowFlags_NoTitleBar | 
                 ImGuiWindowFlags_NoResize | 
                 ImGuiWindowFlags_NoMove | 
                 ImGuiWindowFlags_NoBackground);

    ImGui::Text("Main Menu");
    ImGui::Spacing();
    ImGui::Spacing();
    
    if (ImGui::Button("Start Game", ImVec2(300, 50)))
    {
        startGameRequested = true;
    }
    
    ImGui::SameLine();
    
    if (ImGui::Button("Exit", ImVec2(300, 50)))
    {
        exitGameRequested = true;
    }
    
    ImGui::Spacing();
    ImGui::Spacing();
    
    if (ImGui::Button("Save", ImVec2(300, 50)))
    {
        if (plane) {
            save_to_binary(plane, "saves/save.dat");
        }
    }
    
    ImGui::SameLine();
    
    if (ImGui::Button("Load", ImVec2(300, 50)))
    {
        if (plane) {
            load_from_binary(plane, "saves/save.dat");
        }
    }
    for (size_t i = 0; i < 6; i++)
    {
        ImGui::Spacing();
    }
    ImGui::Text("Use WASD to move, mouse to look around. Press TAB to open the object editor.");
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
