#pragma once
#include <cassert>
#include <type_traits>

#include "../../utils/interface/Align.hpp"
#include "Allocator.hpp"
#include "MemResource.hpp"

namespace DT::Primitive::AllocateStrategy
{
class HeapAllocator
{
   public:
    HeapAllocator() noexcept = default;

    explicit HeapAllocator(const NullResource& resource) {}

    void* Allocate(size_t size, size_t alignment = alignof(std::max_align_t))
    {
        alignment = (alignment < sizeof(void*)) ? sizeof(void*) : alignment;
        assert(IsPowerOfTow(alignment));
        assert((alignment % sizeof(void*)) == 0);

        if (alignment < MIN_ALLOCATE_ALIGNMENT) alignment = MIN_ALLOCATE_ALIGNMENT;

        void* p = ::operator new(size, std::align_val_t{alignment});
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
}  // namespace DT::Primitive::AllocateStrategy