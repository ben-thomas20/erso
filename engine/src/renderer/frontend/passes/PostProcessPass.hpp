#pragma once

#include <renderer/backend/Framebuffer.hpp>
#include <renderer/backend/Shader.hpp>
#include <renderer/backend/VertexArray.hpp>
#include <cstdint>

namespace engine {

// ─── PostProcessPass ──────────────────────────────────────────────────────────
// Applies the full post-processing chain to the HDR lighting output:
//   1. Bloom  — threshold + 4× dual-Kawase blur at half resolution
//   2. Tonemap — ACES filmic curve + bloom composite
//   3. FXAA   — edge-directed spatial anti-aliasing
//
// The final output is an LDR RGBA8 texture ready to blit to the swapchain.
class PostProcessPass {
public:
    PostProcessPass(std::uint32_t w, std::uint32_t h);

    void OnResize(std::uint32_t w, std::uint32_t h);

    // Execute the full post-process chain.  Returns the FXAA output texture.
    const Texture& Execute(const Texture& hdrColor);

    float BloomThreshold  = 1.0f;
    float BloomStrength   = 0.08f;

    // Expose all shaders for hot-reload registration.
    void ForEachShader(auto&& fn) {
        fn(bloomThresholdShader_);
        fn(bloomBlurShader_);
        fn(tonemapShader_);
        fn(fxaaShader_);
    }

private:
    std::uint32_t width_, height_;

    // Bloom at half resolution (ping-pong)
    Framebuffer bloomA_;
    Framebuffer bloomB_;

    // Tonemapped LDR output
    Framebuffer tonemapFBO_;

    // FXAA output (final result)
    Framebuffer fxaaFBO_;

    // Shaders (all use the shared blit.vert fullscreen-triangle vertex shader)
    Shader bloomThresholdShader_;
    Shader bloomBlurShader_;
    Shader tonemapShader_;
    Shader fxaaShader_;

    VertexArray quadVAO_;

    void DrawFullscreenTriangle();
};

} // namespace engine
