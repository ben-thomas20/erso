#include <renderer/frontend/passes/ShadowPass.hpp>
#include <renderer/frontend/RenderQueue.hpp>
#include <renderer/frontend/Renderer.hpp>
#include <renderer/frontend/UniformData.hpp>
#include <core/Assert.hpp>
#include <core/Log.hpp>

#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <span>

#ifndef ENGINE_ASSET_DIR
#  define ENGINE_ASSET_DIR "assets"
#endif
#define ASSET(rel) ENGINE_ASSET_DIR "/" rel

namespace engine {

ShadowPass::ShadowPass()
    : fbo_   (kShadowMapSize, kShadowMapSize,
               std::span<const AttachmentSpec>{}, // depth-only
               true)
    , shader_(Shader::FromFiles(ASSET("shaders/shadow/shadow.vert"),
                                ASSET("shaders/shadow/shadow.frag")))
{
    ENGINE_ASSERT(shader_.IsValid(), "ShadowPass: shadow shader failed to compile");
}

void ShadowPass::Execute(const RenderQueue&  queue,
                         const PerFrameData& /*frameData*/,
                         UniformBufferCache& ubos,
                         const glm::vec3&    lightDir,
                         const glm::vec3&    lightColor,
                         float               lightIntensity)
{
    // Orthographic light frustum sized to enclose the visible scene.
    // Use a generous fixed extent for Phase 4; Phase 6 fits it tightly.
    constexpr float kExtent = 8.f;
    constexpr float kDepth  = 20.f;

    const glm::vec3 up      = (std::abs(lightDir.y) < 0.99f)
                                  ? glm::vec3(0.f, 1.f, 0.f)
                                  : glm::vec3(1.f, 0.f, 0.f);
    const glm::vec3 lightPos = -glm::normalize(lightDir) * (kDepth * 0.5f);

    const glm::mat4 lightView  = glm::lookAt(lightPos, glm::vec3(0.f), up);
    const glm::mat4 lightProj  = glm::ortho(-kExtent, kExtent,
                                             -kExtent, kExtent,
                                             0.1f, kDepth);
    const glm::mat4 lightSpace = lightProj * lightView;

    // Upload ShadowData UBO
    ShadowData sd{};
    sd.lightSpaceMatrix = lightSpace;
    sd.lightDir         = lightDir;
    sd.lightColor       = lightColor;
    sd.lightIntensity   = lightIntensity;
    ubos.UploadShadow(sd);

    // Render depth pass
    fbo_.Bind();
    glViewport(0, 0,
               static_cast<GLsizei>(kShadowMapSize),
               static_cast<GLsizei>(kShadowMapSize));
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glCullFace(GL_FRONT); // reduce peter-panning

    shader_.Bind();

    for (const RenderCommand& cmd : queue.ShadowCasters()) {
        PerObjectData obj{};
        obj.model        = cmd.modelMatrix;
        obj.normalMatrix = cmd.normalMatrix;
        ubos.UploadPerObject(obj);

        glBindVertexArray(cmd.vaoID);
        const void* indexOffset = reinterpret_cast<const void*>(
            static_cast<std::uintptr_t>(cmd.baseIndex) * sizeof(std::uint32_t));
        glDrawElementsBaseVertex(GL_TRIANGLES,
                                 static_cast<GLsizei>(cmd.indexCount),
                                 GL_UNSIGNED_INT,
                                 indexOffset,
                                 static_cast<GLint>(cmd.baseVertex));
    }

    glCullFace(GL_BACK);
    glBindVertexArray(0);
    Framebuffer::BindDefault();
}

} // namespace engine
