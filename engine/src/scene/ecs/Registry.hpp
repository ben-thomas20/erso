#pragma once

#include <core/Assert.hpp>
#include <any>
#include <cstdint>
#include <typeindex>
#include <unordered_map>

namespace engine {

using EntityID = std::uint32_t;
constexpr EntityID INVALID_ENTITY = 0;

// ─── Registry ─────────────────────────────────────────────────────────────────
// Archetype-free ECS registry.
// Each entity owns an unordered_map<type_index, any> of components.
// Simple and correct; cache-unfriendly but sufficient until Phase 6+ profiling
// reveals a need for archetypes.
class Registry {
public:
    // ── Entity management ─────────────────────────────────────────────────────

    EntityID CreateEntity()
    {
        const EntityID id = nextID_++;
        entities_.emplace(id, ComponentMap{});
        return id;
    }

    void DestroyEntity(EntityID id)
    {
        entities_.erase(id);
    }

    bool IsAlive(EntityID id) const { return entities_.contains(id); }

    // ── Component management ──────────────────────────────────────────────────

    template<typename T>
    T& AddComponent(EntityID id, T component = {})
    {
        ENGINE_ASSERT(entities_.contains(id), "AddComponent: entity does not exist");
        auto& map = entities_.at(id);
        map[std::type_index(typeid(T))] = std::move(component);
        return std::any_cast<T&>(map.at(std::type_index(typeid(T))));
    }

    template<typename T>
    T& GetComponent(EntityID id)
    {
        ENGINE_ASSERT(HasComponent<T>(id), "GetComponent: component not present");
        return std::any_cast<T&>(
            entities_.at(id).at(std::type_index(typeid(T))));
    }

    template<typename T>
    const T& GetComponent(EntityID id) const
    {
        ENGINE_ASSERT(HasComponent<T>(id), "GetComponent: component not present");
        return std::any_cast<const T&>(
            entities_.at(id).at(std::type_index(typeid(T))));
    }

    template<typename T>
    bool HasComponent(EntityID id) const
    {
        const auto it = entities_.find(id);
        if (it == entities_.end()) return false;
        return it->second.contains(std::type_index(typeid(T)));
    }

    template<typename T>
    void RemoveComponent(EntityID id)
    {
        ENGINE_ASSERT(entities_.contains(id), "RemoveComponent: entity does not exist");
        entities_.at(id).erase(std::type_index(typeid(T)));
    }

    // ── Iteration ─────────────────────────────────────────────────────────────

    // Call fn(EntityID, Ts&...) for every entity that has ALL of Ts.
    // Fn is deduced from the callable; Ts must be explicitly provided:
    //   registry.Each<A, B>([](EntityID id, A& a, B& b){ ... });
    template<typename... Ts, typename Fn>
    void Each(Fn&& fn)
    {
        for (auto& [id, map] : entities_) {
            if ((map.contains(std::type_index(typeid(Ts))) && ...)) {
                fn(id,
                   std::any_cast<Ts&>(map.at(std::type_index(typeid(Ts))))...);
            }
        }
    }

    // Const overload — fn receives const component refs.
    template<typename... Ts, typename Fn>
    void Each(Fn&& fn) const
    {
        for (const auto& [id, map] : entities_) {
            if ((map.contains(std::type_index(typeid(Ts))) && ...)) {
                fn(id,
                   std::any_cast<const Ts&>(map.at(std::type_index(typeid(Ts))))...);
            }
        }
    }

    std::size_t EntityCount() const { return entities_.size(); }

private:
    using ComponentMap = std::unordered_map<std::type_index, std::any>;

    std::unordered_map<EntityID, ComponentMap> entities_;
    EntityID nextID_ = 1; // 0 is INVALID_ENTITY
};

} // namespace engine
