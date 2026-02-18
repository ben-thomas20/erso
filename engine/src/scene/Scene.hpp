#pragma once

#include <scene/ecs/Registry.hpp>
#include <scene/ecs/Components.hpp>
#include <renderer/frontend/UniformData.hpp>
#include <resources/ResourceManager.hpp>
#include <platform/Window.hpp>
#include <optional>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace engine {

// Scene owns the ECS registry and orchestrates system updates each frame.
//
// Orbit camera: hold LMB + drag to orbit; scroll (or pinch) to zoom.
class Scene {
public:
    Registry registry;

    // Set up a demo scene:
    //   • One PBR box entity at the origin.
    //   • One primary orbit camera.
    //   • One directional light (sun-like, from upper-right).
    void SetupOrbitBoxDemo(ResourceManager& rm);

    // Update ECS, orbit input, and systems. Returns PerFrameData or nullopt if
    // no primary camera exists.
    std::optional<PerFrameData> Update(float time, float deltaTime, const Window& window);

    // Directional light parameters (read by Application to build FrameContext).
    glm::vec3 GetLightDir()       const { return lightDir_;       }
    glm::vec3 GetLightColor()     const { return lightColor_;     }
    float     GetLightIntensity() const { return lightIntensity_; }

    // Mutable references for ImGui live-editing.
    glm::vec3& LightDirMut()       { return lightDir_;       }
    glm::vec3& LightColorMut()     { return lightColor_;     }
    float&     LightIntensityMut() { return lightIntensity_; }

    EntityID CameraEntity() const { return cameraEntity_; }
    EntityID BoxEntity()    const { return boxEntity_;    }

private:
    // Orbit state
    float     orbitYaw_    =  30.f;
    float     orbitPitch_  =  20.f;
    float     orbitRadius_ =   3.5f;
    glm::vec3 orbitTarget_ = glm::vec3(0.f);

    glm::vec2 prevMousePos_    = {};
    bool      mouseWasPressed_ = false;

    EntityID cameraEntity_ = INVALID_ENTITY;
    EntityID boxEntity_    = INVALID_ENTITY;

    // Directional light (constant for Phase 4; editable via ImGui in Phase 6)
    glm::vec3 lightDir_       = glm::vec3(0.4f, -0.8f, 0.4f); // normalised in SetupOrbitBoxDemo
    glm::vec3 lightColor_     = glm::vec3(1.0f, 0.95f, 0.85f);
    float     lightIntensity_ = 4.f;

    glm::vec3 ComputeOrbitPosition() const;
};

} // namespace engine
