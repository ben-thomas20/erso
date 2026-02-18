#pragma once

#include <platform/Input.hpp>
#include <glm/vec2.hpp>
#include <string_view>

// Forward-declare the GLFW window type so Window.hpp remains GLFW-free.
struct GLFWwindow;

namespace engine {

class Window {
public:
    Window(int width, int height, std::string_view title);
    ~Window();

    Window(const Window&)            = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&&)                 = delete;
    Window& operator=(Window&&)      = delete;

    void       PollEvents();
    void       SwapBuffers();
    bool       ShouldClose() const;

    // Logical window size in screen-coordinates (used for UI / cursor input).
    glm::ivec2 GetSize() const;

    // Physical framebuffer size in pixels — use this for all OpenGL viewport
    // and FBO operations.  On HiDPI / Retina displays this is 2× GetSize().
    glm::ivec2 GetFramebufferSize() const;

    bool      IsKeyPressed        (Key key) const;
    bool      IsMouseButtonPressed(MouseButton btn) const;
    glm::vec2 GetMousePosition    () const;

    // Escape hatch for back-ends that need the raw handle (e.g. ImGui init).
    // Do not use outside of platform/ or app/ initialisation code.
    GLFWwindow* GetNativeHandle() const { return window_; }

private:
    GLFWwindow* window_ = nullptr;

    // Translate engine key/button codes to GLFW constants.
    static int ToGLFWKey        (Key key)         noexcept;
    static int ToGLFWMouseButton(MouseButton btn) noexcept;
};

} // namespace engine
