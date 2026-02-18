#pragma once

#include <cstddef>
#include <cstdint>
#include <utility>

namespace engine {

// Simple bump-pointer allocator.  Freed memory is NOT reclaimed individually;
// call Reset() to reclaim the entire arena at once.
// Non-copyable, non-movable (owns raw memory).
class LinearAllocator {
public:
    explicit LinearAllocator(std::size_t capacity);
    ~LinearAllocator();

    LinearAllocator(const LinearAllocator&)            = delete;
    LinearAllocator& operator=(const LinearAllocator&) = delete;
    LinearAllocator(LinearAllocator&&)                 = delete;
    LinearAllocator& operator=(LinearAllocator&&)      = delete;

    // Allocate `size` bytes with the requested alignment.
    // Returns nullptr if the allocator would overflow.
    void* Allocate(std::size_t size,
                   std::size_t alignment = alignof(std::max_align_t));

    // Typed helper — constructs T in-place with placement new.
    template<typename T, typename... Args>
    T* New(Args&&... args)
    {
        void* mem = Allocate(sizeof(T), alignof(T));
        if (!mem) return nullptr;
        return new (mem) T(std::forward<Args>(args)...);
    }

    // Release all allocations and reset the cursor to the start of the buffer.
    void Reset();

    std::size_t Used()     const { return used_; }
    std::size_t Capacity() const { return capacity_; }

private:
    std::uint8_t* buffer_   = nullptr;
    std::size_t   capacity_ = 0;
    std::size_t   used_     = 0;
};

} // namespace engine
