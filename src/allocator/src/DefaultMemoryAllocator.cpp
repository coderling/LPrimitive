#include "../../interface/primitive/allocator/DefaultMemoryAllocator.hpp"

namespace CDL::Primitive
{
DefaultMemoryAllocator& GetGlobalAllocator()
{
    static DefaultMemoryAllocator allocator("default heap allocator", NullChunk());
    return allocator;
}
}  // namespace CDL::Primitive