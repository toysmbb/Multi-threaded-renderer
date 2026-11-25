#include "Application.h"
#include <spdlog/spdlog.h>
#include <algorithm>
#include <cstdio>
#include <thread>

namespace
{
    constexpr float kMaxFrameTime = 0.05f;
}

Application::Application(const AppOptions& options)
    : m_window(options.width, options.height, "Multithreaded renderer"),
      m_renderer(Renderer::Config{ options.shaderDir, options.width, options.height, options.particleCapacity,
                                   options.threads }),
      m_vsync(options.vsync)
{
    GLFWwindow* handle = m_window.GetHandle();
    glfwSetWindowUserPointer(handle, this);
    glfwSetCursorPosCallback(handle, MouseCallback);
    glfwSetKeyCallback(handle, KeyCallback);
    glfwSetFramebufferSizeCallback(handle, FramebufferSizeCallback);
    glfwSetInputMode(handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    m_window.SetVSync(m_vsync);
}

void Application::Run()
{
    double lastTime = glfwGetTime();
    double statsTime = lastTime;
    int framesSinceStats = 0;

    while (!m_window.ShouldClose())
    {
        const double now = glfwGetTime();
        const float dt = std::min(static_cast<float>(now - lastTime), kMaxFrameTime);
        lastTime = now;

        ProcessInput(dt);
        m_renderer.Update(dt);
        m_renderer.Draw();

        m_window.SwapBuffers();
        m_window.PollEvents();

        ++framesSinceStats;
        if (now - statsTime >= 1.0)
        {
            UpdateWindowTitle(framesSinceStats / (now - statsTime));
            framesSinceStats = 0;
            statsTime = now;
        }
    }
}

void Application::ProcessInput(float dt)
{
    GLFWwindow* handle = m_window.GetHandle();
    Camera& camera = m_renderer.GetCamera();

    if (glfwGetKey(handle, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboardInput(CameraMovement::Forward, dt);
    if (glfwGetKey(handle, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboardInput(CameraMovement::Backward, dt);
    if (glfwGetKey(handle, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboardInput(CameraMovement::Left, dt);
    if (glfwGetKey(handle, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboardInput(CameraMovement::Right, dt);
}

void Application::UpdateWindowTitle(double fps)
{
    char title[192];
    std::snprintf(title, sizeof(title),
                  "Particle renderer | %.0f FPS | %zu threads | %zu particles | sim %.2f ms | [ ] threads, V vsync (%s)",
                  fps, m_renderer.GetThreadCount(), m_renderer.GetActiveParticles(), m_renderer.GetUpdateMs(),
                  m_vsync ? "on" : "off");
    m_window.SetTitle(title);
}

void Application::MouseCallback(GLFWwindow* window, double x, double y)
{
    auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));

    if (app->m_firstMouse)
    {
        app->m_lastMouseX = x;
        app->m_lastMouseY = y;
        app->m_firstMouse = false;
    }

    const float xOffset = static_cast<float>(x - app->m_lastMouseX);
    const float yOffset = static_cast<float>(app->m_lastMouseY - y);

    app->m_lastMouseX = x;
    app->m_lastMouseY = y;

    app->m_renderer.GetCamera().ProcessMouseMovement(xOffset, yOffset);
}

void Application::KeyCallback(GLFWwindow* window, int key, int, int action, int)
{
    if (action != GLFW_PRESS)
        return;

    auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    Renderer& renderer = app->m_renderer;

    const std::size_t maxThreads = 2 * std::max<std::size_t>(1, std::thread::hardware_concurrency());

    switch (key)
    {
    case GLFW_KEY_ESCAPE:
        app->m_window.Close();
        break;
    case GLFW_KEY_LEFT_BRACKET: 
        if (renderer.GetThreadCount() > 1)
            renderer.SetThreadCount(renderer.GetThreadCount() - 1);
        break;
    case GLFW_KEY_RIGHT_BRACKET: 
        if (renderer.GetThreadCount() < maxThreads)
            renderer.SetThreadCount(renderer.GetThreadCount() + 1);
        break;
    case GLFW_KEY_V:
        app->m_vsync = !app->m_vsync;
        app->m_window.SetVSync(app->m_vsync);
        break;
    default:
        break;
    }
}

void Application::FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    app->m_renderer.OnResize(width, height);
}
