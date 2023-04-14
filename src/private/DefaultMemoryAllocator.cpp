#include "../../interface/primitive/allocator/DefaultMemoryAllocator.hpp"

namespace DT::Primitive
{
DefaultMemoryAllocator& GetGlobalAllocator()
{
    static DefaultMemoryAllocator allocator("default heap allocator", NullChunk());
    return allocator;
}
}  // namespace DT::Primitive