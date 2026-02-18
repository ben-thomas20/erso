#pragma once

#include <glm/vec3.hpp>
#include <string>
#include <unordered_map>
#include <cstdint>

// Forward-declare GLFW and ImGui types so this header stays lean.
struct GLFWwindow;

namespace engine {

// ─── DebugUIData ──────────────────────────────────────────────────────────────
// All per-frame data the overlay needs.  Pointers to mutable fields allow
// the ImGui sliders to write back directly (light inspector).
struct DebugUIData {
    // Performance timers (milliseconds per pass, from GPUTimer).
    std::unordered_map<std::string, float> gpuTimes;
    float frameMs      = 0.f;

    // Culling stats (from RenderSystem::CullStats).
    std::uint32_t totalMeshCount = 0;
    std::uint32_t culledCount    = 0;
    std::uint32_t drawCallCount  = 0;

    // G-buffer preview textures (raw GL IDs for ImGui::Image).
    std::uint32_t gNormalTexID   = 0;
    std::uint32_t gAlbedoTexID   = 0;
    std::uint32_t gMaterialTexID = 0;
    std::uint32_t hdrTexID       = 0;

    // Light (pointers to Scene members — ImGui writes directly).
    glm::vec3* lightDirPtr       = nullptr;
    glm::vec3* lightColorPtr     = nullptr;
    float*     lightIntensityPtr = nullptr;

    // Shader hot-reload status.
    std::string lastReloadedShader;
};

// ─── DebugUI ──────────────────────────────────────────────────────────────────
// Manages the ImGui context and renders the engine debug overlay.
//
// Usage each frame:
//   debugUI.BeginFrame();
//   debugUI.Draw(data);
//   debugUI.EndFrame();
class DebugUI {
public:
    explicit DebugUI(GLFWwindow* window);
    ~DebugUI();

    DebugUI(const DebugUI&)            = delete;
    DebugUI& operator=(const DebugUI&) = delete;

    void BeginFrame();
    void Draw(DebugUIData& data);
    void EndFrame();

    // Track the name of the last reloaded shader (call from ResourceManager callback).
    void NotifyReload(const std::string& shaderName) { lastReloaded_ = shaderName; }
    const std::string& LastReloaded() const          { return lastReloaded_; }

private:
    std::string lastReloaded_;
};

} // namespace engine
