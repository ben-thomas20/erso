#pragma once

#include <renderer/backend/Framebuffer.hpp>
#include <renderer/backend/Shader.hpp>
#include <renderer/backend/VertexArray.hpp>
#include <renderer/frontend/UniformData.hpp>
#include <cstdint>

namespace engine {

class UniformBufferCache;

// ─── LightingPass ─────────────────────────────────────────────────────────────
// Fullscreen deferred shading pass.  Reads the G-Buffer + shadow map, evaluates
// the Cook-Torrance BRDF for the directional light, and outputs HDR radiance to
// an RGB16F framebuffer.
class LightingPass {
public:
    LightingPass(std::uint32_t w, std::uint32_t h);

    void OnResize(std::uint32_t w, std::uint32_t h);

    void Execute(const Texture& gNormal,
                 const Texture& gAlbedo,
                 const Texture& gMaterial,
                 const Texture& gDepth,
                 const Texture& shadowMap,
                 UniformBufferCache& ubos);

    const Texture& HDROutput() const { return fbo_.GetColorAttachment(0); }
    Shader&        GetShader()      { return shader_; }

private:
    Framebuffer fbo_;
    Shader      shader_;
    VertexArray quadVAO_;
};

} // namespace engine
