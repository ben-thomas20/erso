#include <scene/Scene.hpp>
#include <scene/systems/TransformSystem.hpp>
#include <scene/systems/CameraSystem.hpp>
#include <resources/MeshLoader.hpp>
#include <resources/Material.hpp>
#include <core/Log.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/common.hpp>
#include <cmath>
#include <algorithm>

namespace engine {

// ─── Setup ────────────────────────────────────────────────────────────────────

void Scene::SetupOrbitBoxDemo(ResourceManager& rm)
{
    // Normalise light direction
    lightDir_ = glm::normalize(lightDir_);

    // ── Box entity ────────────────────────────────────────────────────────────
    boxEntity_ = registry.CreateEntity();
    auto& tc = registry.AddComponent<TransformComponent>(boxEntity_);
    tc.dirty = true;

    // Create a PBR material: warm orange, non-metallic, moderately rough.
    Material mat;
    mat.albedoFactor    = glm::vec3(0.9f, 0.42f, 0.12f);
    mat.metallicFactor  = 0.0f;
    mat.roughnessFactor = 0.75f;
    const MaterialHandle matHandle = rm.CreateMaterial(mat);

    auto& mc = registry.AddComponent<MeshComponent>(boxEntity_);
    mc.meshHandle    = rm.AddMesh(MeshLoader::CreateBox(0.5f)).index;  // RawMesh → mega-buffer
    mc.materialHandle = matHandle.index;
    mc.visible       = true;
    mc.castsShadow   = true;

    // Camera entity
    cameraEntity_ = registry.CreateEntity();
    auto& camTc = registry.AddComponent<TransformComponent>(cameraEntity_);
    camTc.position = ComputeOrbitPosition();
    camTc.dirty    = true;

    auto& cc = registry.AddComponent<CameraComponent>(cameraEntity_);
    cc.fovY      = 60.f;
    cc.nearPlane = 0.1f;
    cc.farPlane  = 100.f;
    cc.isPrimary = true;

    LOG_INFO("Scene: SetupOrbitBoxDemo — {} entities", registry.EntityCount());
}

// ─── Orbit helpers ────────────────────────────────────────────────────────────

glm::vec3 Scene::ComputeOrbitPosition() const
{
    const float yaw   = glm::radians(orbitYaw_);
    const float pitch = glm::radians(orbitPitch_);
    return orbitTarget_ + glm::vec3(
        orbitRadius_ * std::cos(pitch) * std::sin(yaw),
        orbitRadius_ * std::sin(pitch),
        orbitRadius_ * std::cos(pitch) * std::cos(yaw));
}

// ─── Update ───────────────────────────────────────────────────────────────────

std::optional<PerFrameData> Scene::Update(float time, float deltaTime,
                                          const Window& window)
{
    // ── Orbit camera input ────────────────────────────────────────────────────
    const glm::vec2 mousePos = window.GetMousePosition();
    const bool      lmb      = window.IsMouseButtonPressed(MouseButton::Left);

    if (lmb) {
        if (mouseWasPressed_) {
            constexpr float kSensitivity = 0.3f;
            const glm::vec2 delta        = mousePos - prevMousePos_;
            orbitYaw_   -= delta.x * kSensitivity;
            orbitPitch_ += delta.y * kSensitivity;
            orbitPitch_  = std::clamp(orbitPitch_, -89.f, 89.f);
        }
        mouseWasPressed_ = true;
    } else {
        mouseWasPressed_ = false;
    }
    prevMousePos_ = mousePos;

    // Update camera entity position from orbit state
    if (cameraEntity_ != INVALID_ENTITY &&
        registry.HasComponent<TransformComponent>(cameraEntity_))
    {
        auto& ct      = registry.GetComponent<TransformComponent>(cameraEntity_);
        ct.position   = ComputeOrbitPosition();
        ct.dirty      = true;
    }

    // ── Systems ───────────────────────────────────────────────────────────────
    TransformSystem::Update(registry);

    // Compute a pole-safe up vector.  When pitch approaches ±90° the standard
    // world-up (0,1,0) becomes nearly parallel to the view direction, making
    // glm::lookAt degenerate.  Instead, derive the up vector continuously from
    // the orbit yaw so the transition is always smooth.
    const glm::vec3 up = [&]() -> glm::vec3 {
        const float yRad   = glm::radians(orbitYaw_);
        const float weight = glm::clamp(
            (std::abs(orbitPitch_) - 70.f) / 20.f,  // ramps 0→1 over [70°, 90°]
            0.f, 1.f);
        const glm::vec3 worldUp = glm::vec3(0.f, 1.f, 0.f);
        // The pole-safe XZ up vector must face *away* from the orbit centre
        // when above (pitch > 0) and *toward* it when below (pitch < 0) so
        // that the camera never flips at either pole.
        const float     sign   = (orbitPitch_ >= 0.f) ? -1.f : 1.f;
        const glm::vec3 poleUp = glm::vec3(sign * std::sin(yRad), 0.f,
                                           sign * std::cos(yRad));
        return glm::normalize(glm::mix(worldUp, poleUp, weight));
    }();

    return CameraSystem::Update(
        registry,
        window.GetFramebufferSize(),   // use physical pixels for correct aspect
        orbitTarget_,
        up,
        time,
        deltaTime);
}

} // namespace engine
