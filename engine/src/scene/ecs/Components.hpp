#pragma once

#include <core/Geometry.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <cstdint>

namespace engine {

// ─── TransformComponent ───────────────────────────────────────────────────────
// TransformSystem recomputes worldMatrix whenever dirty == true.
struct TransformComponent {
    glm::vec3 position    = glm::vec3(0.f);
    glm::vec3 eulerAngles = glm::vec3(0.f);  // degrees; applied X→Y→Z
    glm::vec3 scale       = glm::vec3(1.f);
    glm::mat4 worldMatrix = glm::mat4(1.f);  // computed; do not set manually
    bool      dirty       = true;
};

// ─── MeshComponent ────────────────────────────────────────────────────────────
struct MeshComponent {
    std::uint32_t meshHandle     = 0;      // Handle into ResourceManager mesh pool
    std::uint32_t materialHandle = 0;      // Handle into material pool (Phase 4)
    bool          castsShadow    = true;
    bool          visible        = true;
    AABB          localBounds;
};

// ─── CameraComponent ──────────────────────────────────────────────────────────
struct CameraComponent {
    float fovY      = 60.f;   // degrees
    float nearPlane = 0.1f;
    float farPlane  = 1000.f;
    bool  isPrimary = false;
};

// ─── DirectionalLightComponent ────────────────────────────────────────────────
struct DirectionalLightComponent {
    glm::vec3 color     = glm::vec3(1.f);
    float     intensity = 1.f;
};

// ─── PointLightComponent ──────────────────────────────────────────────────────
struct PointLightComponent {
    glm::vec3 color     = glm::vec3(1.f);
    float     intensity = 1.f;
    float     radius    = 10.f;
};

} // namespace engine
