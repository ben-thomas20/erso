#pragma once

#include <renderer/backend/Framebuffer.hpp>
#include <renderer/backend/Shader.hpp>
#include <renderer/frontend/UniformData.hpp>
#include <renderer/frontend/RenderPass.hpp>
#include <glm/vec3.hpp>
#include <cstdint>

namespace engine {

class RenderQueue;
class UniformBufferCache;

// ─── ShadowPass ───────────────────────────────────────────────────────────────
// Renders all shadow-casting meshes into a 2048×2048 depth-only FBO from the
// directional light's perspective. Computes the LightSpaceMatrix and uploads
// it via the ShadowData UBO.
class ShadowPass {
public:
    static constexpr std::uint32_t kShadowMapSize = 2048;

    explicit ShadowPass();

    void OnResize(std::uint32_t /*w*/, std::uint32_t /*h*/) {}

    // Execute the depth-only shadow render.
    // Updates ubos with the computed ShadowData.
    void Execute(const RenderQueue&  queue,
                 const PerFrameData& frameData,
                 UniformBufferCache& ubos,
                 const glm::vec3&    lightDir,
                 const glm::vec3&    lightColor,
                 float               lightIntensity);

    const Texture& ShadowMap() const { return fbo_.GetDepthAttachment(); }
    Shader&        GetShader()       { return shader_; }

private:
    Framebuffer fbo_;
    Shader      shader_;
};

} // namespace engine
