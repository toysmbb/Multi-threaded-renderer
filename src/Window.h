#pragma once
#include <GLFW/glfw3.h> 
#include <string>

class Window
{
public:
    Window(int width, int height, const std::string& title);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    bool ShouldClose() const;
    void Close();
    void SwapBuffers();
    void PollEvents();

    void SetTitle(const std::string& title);
    void SetVSync(bool enabled);
    void GetFramebufferSize(int& width, int& height) const;

    GLFWwindow* GetHandle() const { return m_window; }

private:
    GLFWwindow* m_window = nullptr;
};
