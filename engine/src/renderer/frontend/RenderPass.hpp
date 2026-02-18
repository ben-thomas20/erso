#pragma once

#include <renderer/frontend/RenderCommand.hpp>
#include <renderer/frontend/UniformData.hpp>
#include <cstdint>

namespace engine {

// Forward declaration — defined in Phase 4.
class RenderQueue;

// Base interface for a single render pass.
// Phase 4 implements ShadowPass, GeometryPass, LightingPass, PostProcessPass.
class RenderPass {
public:
    virtual ~RenderPass() = default;

    // Called whenever the swapchain / viewport is resized.
    virtual void OnResize(std::uint32_t w, std::uint32_t h) = 0;

    // Execute the pass for one frame.
    // Implementations bind their own FBO, shaders, and issue draw calls.
    virtual void Execute(const RenderQueue& queue,
                         const PerFrameData& frameData) = 0;
};

} // namespace engine
