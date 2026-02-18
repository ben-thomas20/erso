#pragma once

#include <scene/ecs/System.hpp>
#include <scene/ecs/Registry.hpp>
#include <renderer/frontend/UniformData.hpp>
#include <optional>
#include <glm/vec2.hpp>

namespace engine {

class CameraSystem : public System {
public:
    // Find the primary CameraComponent, build view and projection matrices,
    // and return a fully populated PerFrameData struct.
    // Returns std::nullopt if no primary camera exists.
    //
    // cameraUp: the desired world-space up vector for the view matrix.
    // The caller is responsible for providing a stable up vector that avoids
    // the lookAt singularity (forward ≈ ±up).
    static std::optional<PerFrameData> Update(
        Registry&    registry,
        glm::ivec2   viewport,
        glm::vec3    lookAtTarget,
        glm::vec3    cameraUp,
        float        time,
        float        deltaTime);
};

} // namespace engine
