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

struct A
{
    void* (*Alloc)(size_t size, size_t alignment, size_t offset, int flags);
    void* (*ReAllooc)(size_t size, size_t alignment, size_t offset, int flags);
    void (*Free)(void* ptr, size_t size);
};
}  // namespace CDL::Primitive