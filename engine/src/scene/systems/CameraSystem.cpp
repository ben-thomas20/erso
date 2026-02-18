#include <scene/systems/CameraSystem.hpp>
#include <scene/ecs/Components.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

namespace engine {

std::optional<PerFrameData> CameraSystem::Update(
    Registry&    registry,
    glm::ivec2   viewport,
    glm::vec3    lookAtTarget,
    glm::vec3    cameraUp,
    float        time,
    float        deltaTime)
{
    PerFrameData data{};
    bool         found = false;

    registry.Each<TransformComponent, CameraComponent>(
        [&](EntityID, TransformComponent& tc, CameraComponent& cc)
        {
            if (!cc.isPrimary || found) return;

            const float aspect =
                (viewport.y > 0)
                    ? static_cast<float>(viewport.x) / static_cast<float>(viewport.y)
                    : 1.f;

            data.view              = glm::lookAt(tc.position, lookAtTarget, cameraUp);
            data.projection        = glm::perspective(
                glm::radians(cc.fovY), aspect, cc.nearPlane, cc.farPlane);
            data.viewProjection    = data.projection * data.view;
            data.invViewProjection = glm::inverse(data.viewProjection);
            data.cameraPos         = tc.position;
            data.resolution        = glm::vec2(viewport);
            data.time              = time;
            data.deltaTime         = deltaTime;

            found = true;
        });

    if (!found) return std::nullopt;
    return data;
}

} // namespace engine
