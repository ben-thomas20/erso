#pragma once

#include <renderer/backend/Buffer.hpp>
#include <renderer/backend/Texture.hpp>
#include <renderer/frontend/UniformData.hpp>
#include <renderer/frontend/RenderQueue.hpp>
#include <renderer/frontend/passes/ShadowPass.hpp>
#include <renderer/frontend/passes/GeometryPass.hpp>
#include <renderer/frontend/passes/LightingPass.hpp>
#include <renderer/frontend/passes/PostProcessPass.hpp>
#include <renderer/debug/GPUTimer.hpp>
#include <resources/ResourceManager.hpp>
#include <glm/vec3.hpp>
#include <optional>
#include <cstdint>
#include <unordered_map>
#include <string>

namespace engine {

// ─── UniformBufferCache ───────────────────────────────────────────────────────
class UniformBufferCache {
public:
    void UploadPerFrame (const PerFrameData&  data);
    void UploadPerObject(const PerObjectData& data);
    void UploadShadow   (const ShadowData&    data);

private:
    std::optional<Buffer> perFrameUBO_;
    std::optional<Buffer> perObjectUBO_;
    std::optional<Buffer> shadowUBO_;

    template<typename T>
    static void Upload(std::optional<Buffer>& slot,
                       std::uint32_t          bindingPoint,
                       const T&               data);
};

// ─── FrameContext ─────────────────────────────────────────────────────────────
// All per-frame inputs the Renderer needs to drive a complete frame.
struct FrameContext {
    PerFrameData frame;

    // Directional light (drives shadow pass + lighting pass)
    glm::vec3 lightDir       = glm::normalize(glm::vec3(1.f, -2.f, 1.f));
    glm::vec3 lightColor     = glm::vec3(1.f);
    float     lightIntensity = 3.f;
};

// ─── Renderer ────────────────────────────────────────────────────────────────
// Top-level renderer. Owns all render passes and drives a complete
// deferred-shading frame: Shadow → GBuffer → Lighting → PostProcess.
class Renderer {
public:
    Renderer(std::uint32_t viewportW, std::uint32_t viewportH);

    // Call whenever the window framebuffer changes size.
    void Resize(std::uint32_t w, std::uint32_t h);

    // Submit a draw command for the current frame.
    void Submit(const RenderCommand& cmd);

    // Direct queue access for RenderSystem (submits commands in bulk).
    RenderQueue& GetQueue() { return queue_; }

    // Execute all passes in order and return the LDR output texture.
    // Call once per frame after all Submit()/GatherCommands calls.
    const Texture& RenderFrame(const FrameContext& ctx);

    // Access the UBO cache if a caller needs to upload custom data.
    UniformBufferCache& UBOs() { return ubos_; }

    float& BloomThreshold() { return postPass_.BloomThreshold; }
    float& BloomStrength()  { return postPass_.BloomStrength;  }

    // Register all owned shaders for hot-reload tracking.
    void RegisterShadersForReload(ResourceManager& rm) {
        rm.TrackShaderForReload(shadowPass_.GetShader());
        rm.TrackShaderForReload(geoPass_.GetShader());
        rm.TrackShaderForReload(lightingPass_.GetShader());
        postPass_.ForEachShader([&](Shader& s){ rm.TrackShaderForReload(s); });
    }

    // GPU pass timings from the previous frame (milliseconds).
    const std::unordered_map<std::string, float>& GetLastGPUTimes() const {
        return lastGPUTimes_;
    }

    // G-buffer / HDR texture accessors for debug previews.
    std::uint32_t GetGNormalTexID()   const { return geoPass_.Normal().GetID();      }
    std::uint32_t GetGAlbedoTexID()   const { return geoPass_.Albedo().GetID();      }
    std::uint32_t GetGMaterialTexID() const { return geoPass_.Material().GetID();    }
    std::uint32_t GetHDRTexID()       const { return lightingPass_.HDROutput().GetID(); }

private:
    UniformBufferCache ubos_;
    RenderQueue        queue_;

    ShadowPass      shadowPass_;
    GeometryPass    geoPass_;
    LightingPass    lightingPass_;
    PostProcessPass postPass_;

    GPUTimer                                 gpuTimer_;
    std::unordered_map<std::string, float>   lastGPUTimes_;
};

} // namespace engine
