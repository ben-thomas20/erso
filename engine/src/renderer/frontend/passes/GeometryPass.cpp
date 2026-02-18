#include <renderer/frontend/passes/GeometryPass.hpp>
#include <renderer/frontend/RenderQueue.hpp>
#include <renderer/frontend/Renderer.hpp>
#include <core/Assert.hpp>

#include <glad/gl.h>
#include <array>

#ifndef ENGINE_ASSET_DIR
#  define ENGINE_ASSET_DIR "assets"
#endif
#define ASSET(rel) ENGINE_ASSET_DIR "/" rel

namespace engine {

static constexpr std::array<AttachmentSpec, 3> kGBufferAttachments = {{
    {TextureFormat::RGBA16F, TextureFilter::Nearest, TextureWrap::ClampToEdge}, // normal
    {TextureFormat::RGBA8,   TextureFilter::Nearest, TextureWrap::ClampToEdge}, // albedo
    {TextureFormat::RGBA8,   TextureFilter::Nearest, TextureWrap::ClampToEdge}, // material
}};

GeometryPass::GeometryPass(std::uint32_t w, std::uint32_t h)
    : fbo_   (w, h, std::span(kGBufferAttachments), true)
    , shader_(Shader::FromFiles(ASSET("shaders/geometry/gbuffer.vert"),
                                ASSET("shaders/geometry/gbuffer.frag")))
{
    ENGINE_ASSERT(shader_.IsValid(), "GeometryPass: gbuffer shader failed to compile");
}

void GeometryPass::OnResize(std::uint32_t w, std::uint32_t h) { fbo_.Resize(w, h); }

void GeometryPass::Execute(const RenderQueue&  queue,
                           const PerFrameData& /*frameData*/,
                           UniformBufferCache& ubos)
{
    const auto sz = fbo_.GetSize();
    fbo_.Bind();
    glViewport(0, 0, static_cast<GLsizei>(sz.x), static_cast<GLsizei>(sz.y));
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    shader_.Bind();

    // Bind fixed texture units once for the whole pass
    shader_.SetTexture("u_AlbedoMap",    0);
    shader_.SetTexture("u_NormalMap",    1);
    shader_.SetTexture("u_MetalRoughMap",2);

    for (const RenderCommand& cmd : queue.OpaqueCommands()) {
        // Per-object UBO
        PerObjectData obj{};
        obj.model        = cmd.modelMatrix;
        obj.normalMatrix = cmd.normalMatrix;
        ubos.UploadPerObject(obj);

        // Material uniforms (texture unit bindings are the only direct set)
        glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, cmd.albedoTexID);
        glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, cmd.normalTexID);
        glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, cmd.metallicRoughTexID);

        shader_.SetVec3 ("u_AlbedoFactor",    cmd.albedoFactor);
        shader_.SetFloat("u_MetallicFactor",  cmd.metallicFactor);
        shader_.SetFloat("u_RoughnessFactor", cmd.roughnessFactor);

        glBindVertexArray(cmd.vaoID);
        const void* indexOffset = reinterpret_cast<const void*>(
            static_cast<std::uintptr_t>(cmd.baseIndex) * sizeof(std::uint32_t));
        glDrawElementsBaseVertex(GL_TRIANGLES,
                                 static_cast<GLsizei>(cmd.indexCount),
                                 GL_UNSIGNED_INT,
                                 indexOffset,
                                 static_cast<GLint>(cmd.baseVertex));
    }

    glBindVertexArray(0);
    Framebuffer::BindDefault();
}

} // namespace engine
