#pragma once

#include <platform/Window.hpp>
#include <core/Timer.hpp>
#include <renderer/backend/Shader.hpp>
#include <renderer/backend/VertexArray.hpp>
#include <renderer/frontend/Renderer.hpp>
#include <renderer/debug/DebugRenderer.hpp>
#include <resources/ResourceManager.hpp>
#include <scene/Scene.hpp>
#include <app/DebugUI.hpp>
#include <scene/systems/RenderSystem.hpp>

namespace engine {

// Top-level application — owns window, renderer, scene, and drives the loop.
//
// Phase 6: frustum culling, GPU timers, immediate-mode debug geometry, and an
// ImGui overlay (performance timers, G-buffer previews, light inspector).
class Application {
public:
    Application();
    ~Application();

    Application(const Application&)            = delete;
    Application& operator=(const Application&) = delete;

    void Run();

private:
    Window          window_;
    Timer           timer_;
    ResourceManager resourceManager_;
    Renderer        renderer_;
    Scene           scene_;

    // Fullscreen blit to swapchain.
    VertexArray blitVao_;
    Shader      blitShader_;

    // Phase 6 systems.
    DebugRenderer              debugRenderer_;
    DebugUI                    debugUI_;
    RenderSystem::CullStats    lastCullStats_;
    float                      lastFrameMs_ = 0.f;

    void RenderFrame(float time, float deltaTime);
};

} // namespace engine
