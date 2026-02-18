#pragma once

#include <core/Memory/HandlePool.hpp>
#include <glm/vec3.hpp>
#include <cstdint>
#include <limits>

namespace engine {

// ─── Handle type ─────────────────────────────────────────────────────────────
struct MaterialTag {};
using MaterialHandle = Handle<MaterialTag>;

// Sentinel: texture index not set — resolved to a default fallback at draw time.
static constexpr std::uint32_t kInvalidTexIndex = std::numeric_limits<std::uint32_t>::max();

// ─── Material ─────────────────────────────────────────────────────────────────
// PBR metallic-roughness material.  Texture indices are raw handle indices into
// the ResourceManager's texture pool.  kInvalidTexIndex means "use default".
struct Material {
    std::uint32_t albedoTexIndex     = kInvalidTexIndex;
    std::uint32_t normalTexIndex     = kInvalidTexIndex;
    std::uint32_t metallicRoughIndex = kInvalidTexIndex;

    // Multiplied with the sampled texture value
    glm::vec3 albedoFactor    = glm::vec3(1.f);
    float     metallicFactor  = 0.f;
    float     roughnessFactor = 0.5f;
};

} // namespace engine
