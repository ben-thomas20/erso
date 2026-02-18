#pragma once

#include <renderer/backend/Framebuffer.hpp>
#include <renderer/backend/Shader.hpp>
#include <renderer/frontend/UniformData.hpp>
#include <cstdint>

namespace engine {

class RenderQueue;
class UniformBufferCache;

// ─── GeometryPass ─────────────────────────────────────────────────────────────
// Fills the G-Buffer (MRT) with world-space normal, albedo, and PBR material
// data.  The hardware depth buffer is shared with the subsequent lighting pass.
//
// MRT layout:
//   Color 0 (RGBA16F) — world-space normal
//   Color 1 (RGBA8)   — albedo
//   Color 2 (RGBA8)   — metallic(r), roughness(g), ao(b)
//   Depth              — hardware depth
class GeometryPass {
public:
    GeometryPass(std::uint32_t w, std::uint32_t h);

    void OnResize(std::uint32_t w, std::uint32_t h);

    void Execute(const RenderQueue&  queue,
                 const PerFrameData& frameData,
                 UniformBufferCache& ubos);

    const Texture& Normal()   const { return fbo_.GetColorAttachment(0); }
    const Texture& Albedo()   const { return fbo_.GetColorAttachment(1); }
    const Texture& Material() const { return fbo_.GetColorAttachment(2); }
    const Texture& Depth()    const { return fbo_.GetDepthAttachment();  }
    Shader&        GetShader()      { return shader_; }

private:
    Framebuffer fbo_;
    Shader      shader_;
};

} // namespace engine
