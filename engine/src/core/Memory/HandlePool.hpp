#pragma once

#include <core/Assert.hpp>
#include <cstdint>
#include <limits>
#include <vector>

namespace engine {

// ─── Handle ──────────────────────────────────────────────────────────────────
// Typed, versioned slot-map handle.
// Tag disambiguates handles to different pool types at compile time.
template<typename Tag>
struct Handle {
    static constexpr std::uint32_t kInvalid = std::numeric_limits<std::uint32_t>::max();

    std::uint32_t index      = kInvalid;
    std::uint32_t generation = 0;

    bool IsValid() const { return index != kInvalid; }

    bool operator==(const Handle&) const = default;
};

// ─── HandlePool ──────────────────────────────────────────────────────────────
// A typed slot-map that stores values of type T keyed by Handle<Tag>.
// Handles remain stable across insertions; stale handles are detected via
// the generation counter.
template<typename T, typename Tag>
class HandlePool {
public:
    using HandleType = Handle<Tag>;

    HandleType Insert(T value)
    {
        std::uint32_t idx = 0;
        if (!freeList_.empty()) {
            idx = freeList_.back();
            freeList_.pop_back();
            slots_[idx].value    = std::move(value);
            slots_[idx].occupied = true;
        } else {
            idx = static_cast<std::uint32_t>(slots_.size());
            slots_.push_back({std::move(value), 0u, true});
        }
        return HandleType{idx, slots_[idx].generation};
    }

    T& Get(HandleType handle)
    {
        ENGINE_ASSERT(IsValid(handle), "HandlePool::Get — stale or invalid handle");
        return slots_[handle.index].value;
    }

    const T& Get(HandleType handle) const
    {
        ENGINE_ASSERT(IsValid(handle), "HandlePool::Get — stale or invalid handle");
        return slots_[handle.index].value;
    }

    bool IsValid(HandleType handle) const
    {
        if (handle.index >= slots_.size())               return false;
        if (!slots_[handle.index].occupied)              return false;
        return slots_[handle.index].generation == handle.generation;
    }

    void Remove(HandleType handle)
    {
        ENGINE_ASSERT(IsValid(handle), "HandlePool::Remove — stale or invalid handle");
        Slot& s = slots_[handle.index];
        s.occupied = false;
        ++s.generation;
        freeList_.push_back(handle.index);
    }

    std::size_t Size() const { return slots_.size() - freeList_.size(); }

private:
    struct Slot {
        T            value;
        std::uint32_t generation = 0;
        bool          occupied   = false;
    };

    std::vector<Slot>          slots_;
    std::vector<std::uint32_t> freeList_;
};

} // namespace engine
