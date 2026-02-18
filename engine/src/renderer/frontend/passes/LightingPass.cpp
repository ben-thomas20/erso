#include <renderer/frontend/passes/LightingPass.hpp>
#include <renderer/frontend/Renderer.hpp>
#include <core/Assert.hpp>

#include <glad/gl.h>
#include <array>

#ifndef ENGINE_ASSET_DIR
#  define ENGINE_ASSET_DIR "assets"
#endif
#define ASSET(rel) ENGINE_ASSET_DIR "/" rel

namespace engine {

static constexpr std::array<AttachmentSpec, 1> kHDRAttachment = {{
    {TextureFormat::RGBA16F, TextureFilter::Linear, TextureWrap::ClampToEdge},
}};

LightingPass::LightingPass(std::uint32_t w, std::uint32_t h)
    : fbo_   (w, h, std::span(kHDRAttachment), false) // no depth needed
    , shader_(Shader::FromFiles(ASSET("shaders/lighting/lighting.vert"),
                                ASSET("shaders/lighting/lighting.frag")))
{
    ENGINE_ASSERT(shader_.IsValid(), "LightingPass: lighting shader failed to compile");
}

void LightingPass::OnResize(std::uint32_t w, std::uint32_t h) { fbo_.Resize(w, h); }

void LightingPass::Execute(const Texture& gNormal,
                           const Texture& gAlbedo,
                           const Texture& gMaterial,
                           const Texture& gDepth,
                           const Texture& shadowMap,
                           UniformBufferCache& /*ubos*/)
{
    const auto sz = fbo_.GetSize();
    fbo_.Bind();
    glViewport(0, 0, static_cast<GLsizei>(sz.x), static_cast<GLsizei>(sz.y));
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);

    shader_.Bind();
    shader_.SetTexture("u_GNormal",   0);
    shader_.SetTexture("u_GAlbedo",   1);
    shader_.SetTexture("u_GMaterial", 2);
    shader_.SetTexture("u_GDepth",    3);
    shader_.SetTexture("u_ShadowMap", 4);

    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, gNormal.GetID());
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, gAlbedo.GetID());
    glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, gMaterial.GetID());
    glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D, gDepth.GetID());
    glActiveTexture(GL_TEXTURE4); glBindTexture(GL_TEXTURE_2D, shadowMap.GetID());

    quadVAO_.Bind();
    glDrawArrays(GL_TRIANGLES, 0, 3);
    quadVAO_.Unbind();

    Framebuffer::BindDefault();
}

} // namespace engine
