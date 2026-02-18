#pragma once

namespace engine {

// Minimal base interface for ECS systems.
// Phase 3 systems (TransformSystem, CameraSystem) do not share a common
// Update signature, so this base class is used for documentation and
// for any future polymorphic system management.
class System {
public:
    virtual ~System() = default;
};

} // namespace engine
