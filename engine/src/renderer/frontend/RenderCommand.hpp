#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <cstdint>

namespace engine {

// ─── RenderCommand ────────────────────────────────────────────────────────────
// POD draw-call descriptor with all material data pre-resolved to raw GL IDs.
// Built by RenderSystem each frame; consumed by render passes.
struct RenderCommand {
    // ── Geometry (all meshes share one MeshBuffer VAO) ────────────────────────
    std::uint32_t vaoID      = 0;   // shared VAO from ResourceManager::MeshBuffer
    std::uint32_t indexCount = 0;
    std::uint32_t baseVertex = 0;   // offset into the shared VBO
    std::uint32_t baseIndex  = 0;   // offset into the shared IBO

    // ── Transform ─────────────────────────────────────────────────────────────
    glm::mat4 modelMatrix  = glm::mat4(1.f);
    glm::mat4 normalMatrix = glm::mat4(1.f);  // transpose(inverse(model))

    // ── Material (pre-resolved GL texture IDs) ────────────────────────────────
    std::uint32_t albedoTexID       = 0;   // texture unit 0 in G-buffer pass
    std::uint32_t normalTexID       = 0;   // texture unit 1
    std::uint32_t metallicRoughTexID= 0;   // texture unit 2

    glm::vec3 albedoFactor    = glm::vec3(1.f);
    float     metallicFactor  = 0.f;
    float     roughnessFactor = 0.5f;

    // ── Flags and sorting ─────────────────────────────────────────────────────
    bool  castsShadow      = true;
    bool  transparent      = false;
    float distanceToCamera = 0.f;   // for back-to-front sorting of transparents
};

} // namespace engine
