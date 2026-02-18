#pragma once

#include <scene/ecs/Registry.hpp>
#include <renderer/frontend/UniformData.hpp>
#include <core/Frustum.hpp>
#include <glm/vec3.hpp>
#include <cstdint>

namespace engine {

class RenderQueue;
class ResourceManager;

// ─── RenderSystem ─────────────────────────────────────────────────────────────
// Walks the ECS registry and builds RenderCommands for every visible
// MeshComponent that passes frustum culling.  Material textures are resolved
// to raw GL IDs at submission time so render passes have zero dependency on
// ResourceManager.
class RenderSystem {
public:
    struct CullStats {
        std::uint32_t total   = 0;  // all mesh entities
        std::uint32_t culled  = 0;  // rejected by frustum
        std::uint32_t visible = 0;  // submitted to queue
    };

    // Populate queue with draw commands from all mesh entities that pass
    // frustum culling.  cameraPos is used to compute distance sort keys.
    static CullStats GatherCommands(Registry&              registry,
                                    const ResourceManager& rm,
                                    RenderQueue&           queue,
                                    const glm::vec3&       cameraPos,
                                    const Frustum&         frustum);
};

} // namespace engine
