#include "Application.hpp"
#include <scene/systems/RenderSystem.hpp>
#include <scene/ecs/Components.hpp>
#include <resources/GPUMesh.hpp>
#include <renderer/backend/Framebuffer.hpp>
#include <core/Assert.hpp>
#include <core/Log.hpp>
#include <core/Frustum.hpp>

#include <glad/gl.h>

#ifndef ENGINE_ASSET_DIR
#  define ENGINE_ASSET_DIR "assets"
#endif
#define ASSET(rel) ENGINE_ASSET_DIR "/" rel

namespace engine {

Application::Application()
    : window_         (1280, 720, "Engine | Phase 6 — Frustum Culling + Debug Tooling")
    , resourceManager_()
    , renderer_       (static_cast<std::uint32_t>(window_.GetFramebufferSize().x),
                       static_cast<std::uint32_t>(window_.GetFramebufferSize().y))
    , blitShader_     (Shader::FromFiles(ASSET("shaders/post/blit.vert"),
                                         ASSET("shaders/post/blit.frag")))
    , debugRenderer_  ()
    , debugUI_        (window_.GetNativeHandle())
{
    LOG_INFO("Application initialising (Phase 6 — Frustum Culling + Debug Tooling)");
    ENGINE_ASSERT(blitShader_.IsValid(), "Blit shader failed to compile");

    scene_.SetupOrbitBoxDemo(resourceManager_);

    renderer_.RegisterShadersForReload(resourceManager_);
    resourceManager_.TrackShaderForReload(blitShader_);

    LOG_INFO("Phase 6 ready — LMB drag to orbit; ImGui overlay top-left; edit .glsl to hot-reload");
}

Application::~Application()
{
    LOG_INFO("Application shutting down");
}

void Application::Run()
{
    LOG_INFO("Entering main loop — press Escape to exit");

    float prevTime = timer_.ElapsedSeconds();
    while (!window_.ShouldClose()) {
        window_.PollEvents();
        if (window_.IsKeyPressed(Key::Escape)) break;

        const float now       = timer_.ElapsedSeconds();
        const float deltaTime = now - prevTime;
        prevTime = now;

        RenderFrame(now, deltaTime);
        window_.SwapBuffers();
    }
}

void Application::RenderFrame(float time, float deltaTime)
{
    lastFrameMs_ = deltaTime * 1000.f;

    // Hot-reload any edited shader source files.
    resourceManager_.PollShaderReload();

    // Sync last-reload name to DebugUI.
    const std::string& lastReload = resourceManager_.LastReloadedShader();
    if (!lastReload.empty())
        debugUI_.NotifyReload(lastReload);

    const glm::ivec2 fbSize = window_.GetFramebufferSize();
    const auto w = static_cast<std::uint32_t>(fbSize.x);
    const auto h = static_cast<std::uint32_t>(fbSize.y);

    renderer_.Resize(w, h);

    // Run ECS: orbit input, transforms, camera system.
    const auto frameDataOpt = scene_.Update(time, deltaTime, window_);
    if (!frameDataOpt) return;

    // Build view-projection frustum for culling.
    const Frustum frustum = Frustum::FromViewProjection(frameDataOpt->viewProjection);

    // Gather draw commands — entities that fail ContainsAABB are skipped.
    lastCullStats_ = RenderSystem::GatherCommands(
        scene_.registry, resourceManager_, renderer_.GetQueue(),
        frameDataOpt->cameraPos, frustum);

    // Build frame context.
    FrameContext ctx;
    ctx.frame          = *frameDataOpt;
    ctx.lightDir       = scene_.GetLightDir();
    ctx.lightColor     = scene_.GetLightColor();
    ctx.lightIntensity = scene_.GetLightIntensity();

    // Execute full deferred pipeline (Shadow → GBuffer → Lighting → PostFX).
    const Texture& output = renderer_.RenderFrame(ctx);

    // ── Blit LDR to default framebuffer ──────────────────────────────────────
    Framebuffer::BindDefault();
    glViewport(0, 0, fbSize.x, fbSize.y);
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);

    blitShader_.Bind();
    blitShader_.SetTexture("u_Texture", 0);
    output.Bind(0);

    blitVao_.Bind();
    glDrawArrays(GL_TRIANGLES, 0, 3);
    blitVao_.Unbind();

    // ── Debug geometry ────────────────────────────────────────────────────────
    // Draw AABB wireframes for all mesh entities so frustum culling can be
    // visually confirmed (an entity whose box leaves the view disappears).
    scene_.registry.Each<TransformComponent, MeshComponent>(
        [&](EntityID, TransformComponent& tc, MeshComponent& mc) {
            if (!mc.visible) return;
            const GPUMesh& mesh = resourceManager_.GetMesh(
                MeshHandle{mc.meshHandle, 0u});
            debugRenderer_.DrawAABB(mesh.localBounds, tc.worldMatrix,
                                    {0.2f, 1.0f, 0.2f, 1.0f});
        });

    debugRenderer_.FlushAndClear(frameDataOpt->viewProjection);

    // ── ImGui overlay ─────────────────────────────────────────────────────────
    debugUI_.BeginFrame();

    DebugUIData uiData;
    uiData.gpuTimes       = renderer_.GetLastGPUTimes();
    uiData.frameMs        = lastFrameMs_;
    uiData.totalMeshCount = lastCullStats_.total;
    uiData.culledCount    = lastCullStats_.culled;
    uiData.drawCallCount  = lastCullStats_.visible;
    uiData.gNormalTexID   = renderer_.GetGNormalTexID();
    uiData.gAlbedoTexID   = renderer_.GetGAlbedoTexID();
    uiData.gMaterialTexID = renderer_.GetGMaterialTexID();
    uiData.hdrTexID       = renderer_.GetHDRTexID();
    uiData.lightDirPtr       = &scene_.LightDirMut();
    uiData.lightColorPtr     = &scene_.LightColorMut();
    uiData.lightIntensityPtr = &scene_.LightIntensityMut();
    uiData.lastReloadedShader = debugUI_.LastReloaded();

    debugUI_.Draw(uiData);
    debugUI_.EndFrame();
}

} // namespace engine
