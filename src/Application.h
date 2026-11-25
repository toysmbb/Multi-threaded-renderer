#pragma once
#include <cstddef>
#include <filesystem>
#include "Renderer.h"
#include "Window.h"

struct AppOptions
{
    std::filesystem::path shaderDir;
    int width = 1280;
    int height = 720;
    std::size_t particleCapacity = 1000000; 
    std::size_t threads = 0;
    bool vsync = false;
};

class Application
{
public:
    explicit Application(const AppOptions& options);

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    void Run();

private:
    void ProcessInput(float dt);
    void UpdateWindowTitle(double fps);

    static void MouseCallback(GLFWwindow* window, double x, double y);
    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);

    Window m_window;
    Renderer m_renderer;

    bool m_vsync;
    bool m_firstMouse = true;
    double m_lastMouseX = 0.0;
    double m_lastMouseY = 0.0;
};
