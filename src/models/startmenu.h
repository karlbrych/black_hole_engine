#pragma once

#include <GLFW/glfw3.h>

class StartMenu {
public:
    StartMenu();
    ~StartMenu();
    
    // Initialize ImGui context and backends
    void init(GLFWwindow* window, const char* glsl_version = "#version 330 core");
    
    // Shutdown ImGui
    void shutdown();
    
    // Begin new ImGui frame
    void beginFrame();
    
    // Render the menu UI and return menu state
    bool render(int windowWidth, int windowHeight);
    
    // Finalize ImGui rendering
    void endFrame();
    
    // Check if game should start
    bool shouldStartGame() const { return startGameRequested; }
    
    // Check if game should exit
    bool shouldExitGame() const { return exitGameRequested; }
    
    // Reset menu state
    void reset();
    
private:
    bool startGameRequested;
    bool exitGameRequested;
};
