#pragma once
#include <type_traits>

namespace CDL::Primitive
{
struct IAllocator
{
    virtual ~IAllocator() {}
    virtual void* Allocate(size_t size, size_t alignment = alignof(std::max_align_t), size_t offset = 0, int flags = 0) = 0;
    virtual void Free(void* ptr, size_t size) = 0;
};
}  // namespace CDL::Primitive