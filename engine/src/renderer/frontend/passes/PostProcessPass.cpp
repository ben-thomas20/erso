#include <renderer/frontend/passes/PostProcessPass.hpp>
#include <core/Assert.hpp>
#include <core/Log.hpp>

#include <glad/gl.h>
#include <array>

#ifndef ENGINE_ASSET_DIR
#  define ENGINE_ASSET_DIR "assets"
#endif
#define ASSET(rel) ENGINE_ASSET_DIR "/" rel

namespace engine {

static constexpr std::array<AttachmentSpec, 1> kHalfResHDR = {{
    {TextureFormat::RGBA16F, TextureFilter::Linear, TextureWrap::ClampToEdge},
}};
static constexpr std::array<AttachmentSpec, 1> kLDR = {{
    {TextureFormat::RGBA8, TextureFilter::Linear, TextureWrap::ClampToEdge},
}};

PostProcessPass::PostProcessPass(std::uint32_t w, std::uint32_t h)
    : width_(w), height_(h)
    , bloomA_   (w / 2, h / 2, std::span(kHalfResHDR), false)
    , bloomB_   (w / 2, h / 2, std::span(kHalfResHDR), false)
    , tonemapFBO_(w, h, std::span(kLDR), false)
    , fxaaFBO_  (w, h, std::span(kLDR), false)
    , bloomThresholdShader_(Shader::FromFiles(ASSET("shaders/post/blit.vert"),
                                              ASSET("shaders/post/bloom_threshold.frag")))
    , bloomBlurShader_     (Shader::FromFiles(ASSET("shaders/post/blit.vert"),
                                              ASSET("shaders/post/bloom_blur.frag")))
    , tonemapShader_       (Shader::FromFiles(ASSET("shaders/post/blit.vert"),
                                              ASSET("shaders/post/tonemap.frag")))
    , fxaaShader_          (Shader::FromFiles(ASSET("shaders/post/blit.vert"),
                                              ASSET("shaders/post/fxaa.frag")))
{
    ENGINE_ASSERT(bloomThresholdShader_.IsValid(), "PostProcess: bloom_threshold shader failed");
    ENGINE_ASSERT(bloomBlurShader_.IsValid(),      "PostProcess: bloom_blur shader failed");
    ENGINE_ASSERT(tonemapShader_.IsValid(),        "PostProcess: tonemap shader failed");
    ENGINE_ASSERT(fxaaShader_.IsValid(),           "PostProcess: fxaa shader failed");
}

void PostProcessPass::OnResize(std::uint32_t w, std::uint32_t h)
{
    width_  = w;
    height_ = h;
    bloomA_.Resize(w / 2, h / 2);
    bloomB_.Resize(w / 2, h / 2);
    tonemapFBO_.Resize(w, h);
    fxaaFBO_.Resize(w, h);
}

void PostProcessPass::DrawFullscreenTriangle()
{
    quadVAO_.Bind();
    glDrawArrays(GL_TRIANGLES, 0, 3);
    quadVAO_.Unbind();
}

const Texture& PostProcessPass::Execute(const Texture& hdrColor)
{
    glDisable(GL_DEPTH_TEST);

    // ── 1. Bloom threshold → bloomA (half res) ────────────────────────────────
    bloomA_.Bind();
    glViewport(0, 0,
               static_cast<GLsizei>(width_ / 2),
               static_cast<GLsizei>(height_ / 2));
    bloomThresholdShader_.Bind();
    bloomThresholdShader_.SetTexture("u_HDR", 0);
    bloomThresholdShader_.SetFloat  ("u_Threshold", BloomThreshold);
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, hdrColor.GetID());
    DrawFullscreenTriangle();

    // ── 2. Kawase blur — 4 passes ping-pong at half res ───────────────────────
    bloomBlurShader_.Bind();
    bloomBlurShader_.SetTexture("u_Source", 0);

    Framebuffer* src = &bloomA_;
    Framebuffer* dst = &bloomB_;

    for (int i = 0; i < 4; ++i) {
        dst->Bind();
        bloomBlurShader_.SetInt("u_Iteration", i);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, src->GetColorAttachment(0).GetID());
        DrawFullscreenTriangle();
        std::swap(src, dst);
    }
    // After 4 passes: src points to the final blurred FBO

    // ── 3. Tone map + bloom composite ─────────────────────────────────────────
    tonemapFBO_.Bind();
    glViewport(0, 0, static_cast<GLsizei>(width_), static_cast<GLsizei>(height_));
    tonemapShader_.Bind();
    tonemapShader_.SetTexture("u_HDR",          0);
    tonemapShader_.SetTexture("u_Bloom",         1);
    tonemapShader_.SetFloat  ("u_BloomStrength", BloomStrength);
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, hdrColor.GetID());
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, src->GetColorAttachment(0).GetID());
    DrawFullscreenTriangle();

    // ── 4. FXAA ───────────────────────────────────────────────────────────────
    fxaaFBO_.Bind();
    fxaaShader_.Bind();
    fxaaShader_.SetTexture("u_Source", 0);
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, tonemapFBO_.GetColorAttachment(0).GetID());
    DrawFullscreenTriangle();

    Framebuffer::BindDefault();
    return fxaaFBO_.GetColorAttachment(0);
}

} // namespace engine
