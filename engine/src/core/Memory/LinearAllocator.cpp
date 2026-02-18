#include "LinearAllocator.hpp"

#include <core/Assert.hpp>
#include <cstdlib>
#include <new>

namespace engine {

LinearAllocator::LinearAllocator(std::size_t capacity)
    : buffer_(static_cast<std::uint8_t*>(std::malloc(capacity)))
    , capacity_(capacity)
    , used_(0)
{
    ENGINE_ASSERT(buffer_ != nullptr, "LinearAllocator: malloc failed");
}

LinearAllocator::~LinearAllocator()
{
    std::free(buffer_);
}

void* LinearAllocator::Allocate(std::size_t size, std::size_t alignment)
{
    // Compute an aligned start offset inside the buffer.
    const std::uintptr_t raw     = reinterpret_cast<std::uintptr_t>(buffer_) + used_;
    const std::uintptr_t aligned = (raw + alignment - 1u) & ~(alignment - 1u);
    const std::size_t    padding = static_cast<std::size_t>(aligned - raw);
    const std::size_t    total   = padding + size;

    if (used_ + total > capacity_) return nullptr;

    used_ += total;
    return reinterpret_cast<void*>(aligned);
}

void LinearAllocator::Reset()
{
    used_ = 0;
}

} // namespace engine
