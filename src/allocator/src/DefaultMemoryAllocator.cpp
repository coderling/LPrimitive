#include "DefaultMemoryAllocator.hpp"
#include "DebugUtility.hpp"

namespace LPrimitive
{
DefaultMemoryAllocator::DefaultMemoryAllocator() {}

NODISCARD void* DefaultMemoryAllocator::Allocate(size_t size, const char* dbg_descirption, const char* dbg_filename, const int& dbg_line)
{
    L_ASSERT_EXPR(size > 0);
    return new uint8_t[size];
}

void DefaultMemoryAllocator::Free(void* ptr) { delete[] reinterpret_cast<uint8_t*>(ptr); }

DefaultMemoryAllocator& DefaultMemoryAllocator::GetGobalAllocator()
{
    static DefaultMemoryAllocator allocator;
    return allocator;
}
}  // namespace LPrimitive