// Include GLAD before GLFW so GLFW does not pull in its own GL header.
#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "Window.hpp"
#include <core/Log.hpp>
#include <core/Assert.hpp>

namespace engine {

// ─── GL debug callback ────────────────────────────────────────────────────────

static void GLAD_API_PTR GLDebugCallback(
    GLenum        source,
    GLenum        type,
    GLuint        id,
    GLenum        severity,
    GLsizei       /*length*/,
    const GLchar* message,
    const void*   /*userParam*/)
{
    // Notifications are suppressed at the glDebugMessageControl level;
    // this guard is an extra safety net.
    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) return;

    const char* severityStr = [severity]() noexcept -> const char* {
        switch (severity) {
            case GL_DEBUG_SEVERITY_HIGH:   return "HIGH";
            case GL_DEBUG_SEVERITY_MEDIUM: return "MEDIUM";
            case GL_DEBUG_SEVERITY_LOW:    return "LOW";
            default:                       return "UNKNOWN";
        }
    }();

    LOG_ERROR("[GL][{}] id={} src={:#x} type={:#x}: {}", severityStr, id, source, type, message);
}

// ─── Window ──────────────────────────────────────────────────────────────────

Window::Window(int width, int height, std::string_view title)
{
    const int ok = glfwInit();
    ENGINE_ASSERT(ok, "glfwInit() failed");

    // OpenGL context hints ─ macOS caps at 4.1; other platforms use 4.6.
#ifdef __APPLE__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
#endif
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE,      GLFW_TRUE);
    // Request a debug context so glDebugMessageCallback is available in Debug builds.
#ifndef NDEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

    window_ = glfwCreateWindow(width, height, title.data(), nullptr, nullptr);
    if (!window_) {
        glfwTerminate();
        LOG_FATAL("glfwCreateWindow failed");
    }

    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1); // vsync

    if (!gladLoadGL(glfwGetProcAddress)) {
        glfwDestroyWindow(window_);
        glfwTerminate();
        LOG_FATAL("gladLoadGL failed — could not initialise GLAD");
    }

    // ── GL debug output ──────────────────────────────────────────────────────
    // glDebugMessageCallback is core in 4.3; on macOS 4.1 it is available via
    // the KHR_debug extension.  GLAD loads it as a regular function pointer;
    // check it is non-null before enabling.
    if (glDebugMessageCallback) {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(GLDebugCallback, nullptr);
        // Suppress notification-level messages at the driver level.
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE,
                              GL_DEBUG_SEVERITY_NOTIFICATION,
                              0, nullptr, GL_FALSE);
        LOG_INFO("GL debug output enabled");
    } else {
        LOG_WARN("glDebugMessageCallback unavailable on this driver");
    }

    const auto* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    const auto* version  = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    LOG_INFO("OpenGL {} | {}", version, renderer);
}

Window::~Window()
{
    if (window_) {
        glfwDestroyWindow(window_);
        window_ = nullptr;
    }
    glfwTerminate();
}

void Window::PollEvents()    { glfwPollEvents(); }
void Window::SwapBuffers()   { glfwSwapBuffers(window_); }
bool Window::ShouldClose()  const { return glfwWindowShouldClose(window_) != 0; }

glm::ivec2 Window::GetSize() const
{
    int w = 0, h = 0;
    glfwGetWindowSize(window_, &w, &h);
    return {w, h};
}

glm::ivec2 Window::GetFramebufferSize() const
{
    int w = 0, h = 0;
    glfwGetFramebufferSize(window_, &w, &h);
    return {w, h};
}

bool Window::IsKeyPressed(Key key) const
{
    const int glfwKey = ToGLFWKey(key);
    if (glfwKey == GLFW_KEY_UNKNOWN) return false;
    return glfwGetKey(window_, glfwKey) == GLFW_PRESS;
}

bool Window::IsMouseButtonPressed(MouseButton btn) const
{
    const int glfwBtn = ToGLFWMouseButton(btn);
    if (glfwBtn < 0) return false;
    return glfwGetMouseButton(window_, glfwBtn) == GLFW_PRESS;
}

glm::vec2 Window::GetMousePosition() const
{
    double x = 0.0, y = 0.0;
    glfwGetCursorPos(window_, &x, &y);
    return {static_cast<float>(x), static_cast<float>(y)};
}

// ─── Key / button translation ─────────────────────────────────────────────────

int Window::ToGLFWKey(Key key) noexcept
{
    switch (key) {
        case Key::A: return GLFW_KEY_A;
        case Key::B: return GLFW_KEY_B;
        case Key::C: return GLFW_KEY_C;
        case Key::D: return GLFW_KEY_D;
        case Key::E: return GLFW_KEY_E;
        case Key::F: return GLFW_KEY_F;
        case Key::G: return GLFW_KEY_G;
        case Key::H: return GLFW_KEY_H;
        case Key::I: return GLFW_KEY_I;
        case Key::J: return GLFW_KEY_J;
        case Key::K: return GLFW_KEY_K;
        case Key::L: return GLFW_KEY_L;
        case Key::M: return GLFW_KEY_M;
        case Key::N: return GLFW_KEY_N;
        case Key::O: return GLFW_KEY_O;
        case Key::P: return GLFW_KEY_P;
        case Key::Q: return GLFW_KEY_Q;
        case Key::R: return GLFW_KEY_R;
        case Key::S: return GLFW_KEY_S;
        case Key::T: return GLFW_KEY_T;
        case Key::U: return GLFW_KEY_U;
        case Key::V: return GLFW_KEY_V;
        case Key::W: return GLFW_KEY_W;
        case Key::X: return GLFW_KEY_X;
        case Key::Y: return GLFW_KEY_Y;
        case Key::Z: return GLFW_KEY_Z;
        case Key::Num0: return GLFW_KEY_0;
        case Key::Num1: return GLFW_KEY_1;
        case Key::Num2: return GLFW_KEY_2;
        case Key::Num3: return GLFW_KEY_3;
        case Key::Num4: return GLFW_KEY_4;
        case Key::Num5: return GLFW_KEY_5;
        case Key::Num6: return GLFW_KEY_6;
        case Key::Num7: return GLFW_KEY_7;
        case Key::Num8: return GLFW_KEY_8;
        case Key::Num9: return GLFW_KEY_9;
        case Key::F1:  return GLFW_KEY_F1;
        case Key::F2:  return GLFW_KEY_F2;
        case Key::F3:  return GLFW_KEY_F3;
        case Key::F4:  return GLFW_KEY_F4;
        case Key::F5:  return GLFW_KEY_F5;
        case Key::F6:  return GLFW_KEY_F6;
        case Key::F7:  return GLFW_KEY_F7;
        case Key::F8:  return GLFW_KEY_F8;
        case Key::F9:  return GLFW_KEY_F9;
        case Key::F10: return GLFW_KEY_F10;
        case Key::F11: return GLFW_KEY_F11;
        case Key::F12: return GLFW_KEY_F12;
        case Key::Escape:    return GLFW_KEY_ESCAPE;
        case Key::Enter:     return GLFW_KEY_ENTER;
        case Key::Tab:       return GLFW_KEY_TAB;
        case Key::Backspace: return GLFW_KEY_BACKSPACE;
        case Key::Delete:    return GLFW_KEY_DELETE;
        case Key::Insert:    return GLFW_KEY_INSERT;
        case Key::Left:      return GLFW_KEY_LEFT;
        case Key::Right:     return GLFW_KEY_RIGHT;
        case Key::Up:        return GLFW_KEY_UP;
        case Key::Down:      return GLFW_KEY_DOWN;
        case Key::PageUp:    return GLFW_KEY_PAGE_UP;
        case Key::PageDown:  return GLFW_KEY_PAGE_DOWN;
        case Key::Home:      return GLFW_KEY_HOME;
        case Key::End:       return GLFW_KEY_END;
        case Key::LeftShift:  return GLFW_KEY_LEFT_SHIFT;
        case Key::RightShift: return GLFW_KEY_RIGHT_SHIFT;
        case Key::LeftCtrl:   return GLFW_KEY_LEFT_CONTROL;
        case Key::RightCtrl:  return GLFW_KEY_RIGHT_CONTROL;
        case Key::LeftAlt:    return GLFW_KEY_LEFT_ALT;
        case Key::RightAlt:   return GLFW_KEY_RIGHT_ALT;
        case Key::Space:      return GLFW_KEY_SPACE;
        default:              return GLFW_KEY_UNKNOWN;
    }
}

int Window::ToGLFWMouseButton(MouseButton btn) noexcept
{
    switch (btn) {
        case MouseButton::Left:    return GLFW_MOUSE_BUTTON_LEFT;
        case MouseButton::Right:   return GLFW_MOUSE_BUTTON_RIGHT;
        case MouseButton::Middle:  return GLFW_MOUSE_BUTTON_MIDDLE;
        case MouseButton::Button4: return GLFW_MOUSE_BUTTON_4;
        case MouseButton::Button5: return GLFW_MOUSE_BUTTON_5;
        default:                   return -1;
    }
}

} // namespace engine
