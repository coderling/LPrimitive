#pragma once
#include <cassert>
#include <type_traits>

#include "Align.hpp"
#include "Allocator.hpp"

namespace CDL::Primitive::AllocateStrategy
{
class HeapAllocator
{
   public:
    HeapAllocator() noexcept = default;

    explicit HeapAllocator(const NullChunk& chunk) {}

    void* Allocate(size_t size, size_t alignment = alignof(std::max_align_t), size_t offset = 0)
    {
        alignment = (alignment < sizeof(void*)) ? sizeof(void*) : alignment;
        assert(IsPowerOfTow(alignment));
        assert((alignment % sizeof(void*)) == 0);
        assert(offset == 0);

        void* p = nullptr;
#if defined(_WIN32)
        p = ::_aligned_malloc(size, alignment);
#else
        ::posix_memalign(&p, alignment, size);
#endif
        return p;
    }

    void Free(void* ptr) noexcept
    {
#if defined(_WIN32)
        ::_aligned_free(ptr);
#else
        ::free(&p, alignment, size);
#endif
    }

    void Free(void* ptr, size_t) noexcept { this->Free(ptr); }
};
}  // namespace CDL::Primitive::AllocateStrategy