#pragma once

#include <scene/ecs/System.hpp>
#include <scene/ecs/Registry.hpp>

namespace engine {

class TransformSystem : public System {
public:
    // Recomputes worldMatrix for every TransformComponent with dirty == true,
    // then clears the dirty flag.
    static void Update(Registry& registry);
};

} // namespace engine
