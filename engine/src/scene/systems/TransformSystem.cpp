#include <scene/systems/TransformSystem.hpp>
#include <scene/ecs/Components.hpp>

#include <glm/gtc/matrix_transform.hpp>

namespace engine {

void TransformSystem::Update(Registry& registry)
{
    registry.Each<TransformComponent>(
        [](EntityID, TransformComponent& tc)
        {
            if (!tc.dirty) return;

            const glm::mat4 T = glm::translate(glm::mat4(1.f), tc.position);

            // Apply rotations in X → Y → Z order
            glm::mat4 R = glm::rotate(glm::mat4(1.f),
                                      glm::radians(tc.eulerAngles.x),
                                      glm::vec3(1.f, 0.f, 0.f));
            R = glm::rotate(R,
                            glm::radians(tc.eulerAngles.y),
                            glm::vec3(0.f, 1.f, 0.f));
            R = glm::rotate(R,
                            glm::radians(tc.eulerAngles.z),
                            glm::vec3(0.f, 0.f, 1.f));

            const glm::mat4 S = glm::scale(glm::mat4(1.f), tc.scale);

            tc.worldMatrix = T * R * S;
            tc.dirty       = false;
        });
}

} // namespace engine
