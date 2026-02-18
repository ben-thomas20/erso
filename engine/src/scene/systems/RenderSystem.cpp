#include <scene/systems/RenderSystem.hpp>
#include <scene/ecs/Components.hpp>
#include <renderer/frontend/RenderQueue.hpp>
#include <renderer/frontend/RenderCommand.hpp>
#include <resources/ResourceManager.hpp>
#include <resources/GPUMesh.hpp>
#include <resources/Material.hpp>
#include <core/Log.hpp>

#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace engine {

RenderSystem::CullStats RenderSystem::GatherCommands(Registry&              registry,
                                                      const ResourceManager& rm,
                                                      RenderQueue&           queue,
                                                      const glm::vec3&       cameraPos,
                                                      const Frustum&         frustum)
{
    CullStats stats{};

    registry.Each<TransformComponent, MeshComponent>(
        [&](EntityID, TransformComponent& tc, MeshComponent& mc)
        {
            if (!mc.visible) return;

            const MeshHandle meshHandle{mc.meshHandle, 0u};
            const GPUMesh& mesh = rm.GetMesh(meshHandle);

            ++stats.total;

            // Frustum cull — skip the entity if its AABB is fully outside.
            if (!frustum.ContainsAABB(mesh.localBounds, tc.worldMatrix)) {
                ++stats.culled;
                return;
            }

            ++stats.visible;

            RenderCommand cmd;
            cmd.vaoID       = mesh.sharedVAOID;
            cmd.indexCount  = mesh.indexCount;
            cmd.baseVertex  = mesh.baseVertex;
            cmd.baseIndex   = mesh.baseIndex;
            cmd.modelMatrix = tc.worldMatrix;
            cmd.normalMatrix= glm::transpose(glm::inverse(tc.worldMatrix));
            cmd.castsShadow = mc.castsShadow;

            // Resolve material textures.
            const MaterialHandle matHandle{mc.materialHandle, 0u};
            if (matHandle.IsValid()) {
                const Material& mat = rm.GetMaterial(matHandle);
                cmd.albedoFactor    = mat.albedoFactor;
                cmd.metallicFactor  = mat.metallicFactor;
                cmd.roughnessFactor = mat.roughnessFactor;

                auto resolveTexID = [&](std::uint32_t idx,
                                        const Texture& fallback) -> std::uint32_t {
                    if (idx == kInvalidTexIndex) return fallback.GetID();
                    return rm.GetTexture(TextureHandle{idx, 0u}).GetID();
                };
                cmd.albedoTexID        = resolveTexID(mat.albedoTexIndex,    rm.DefaultAlbedo());
                cmd.normalTexID        = resolveTexID(mat.normalTexIndex,     rm.DefaultNormal());
                cmd.metallicRoughTexID = resolveTexID(mat.metallicRoughIndex, rm.DefaultMetalRough());
            } else {
                cmd.albedoTexID        = rm.DefaultAlbedo().GetID();
                cmd.normalTexID        = rm.DefaultNormal().GetID();
                cmd.metallicRoughTexID = rm.DefaultMetalRough().GetID();
            }

            const glm::vec3 origin = glm::vec3(tc.worldMatrix[3]);
            cmd.distanceToCamera   = glm::length(origin - cameraPos);

            queue.Submit(cmd);
        });

    if (stats.culled > 0) {
        LOG_TRACE("RenderSystem: {}/{} meshes culled ({:.0f}%)",
                  stats.culled, stats.total,
                  100.f * static_cast<float>(stats.culled) /
                          static_cast<float>(stats.total));
    }

    return stats;
}

} // namespace engine
