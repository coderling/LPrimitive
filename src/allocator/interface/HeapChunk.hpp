#pragma once
#include <type_traits>

#include "CommonDefines.hpp"
#include "Misc.hpp"

namespace CDL::Primitive
{
class HeapChunk
{
    void* p_begin;
    void* p_end;

   public:
    HeapChunk() noexcept = default;
    HeapChunk(size_t size)
    {
        if (size > 0)
            {
                const auto& alignment = alignof(std::max_align_t);
#if defined(_WIN32)
                p_begin = ::_aligned_malloc(size, alignment);
#else
                ::posix_memalign(&p, alignment, size);
#endif
                p_end = Misc::PtrAdd(p_begin, size);
            }
    }

    ~HeapChunk() noexcept
    {
#if defined(_WIN32)
        ::_aligned_free(p_begin);
#else
        ::free(&p_begin, alignment, size);
#endif
    }

    NOCOPY_AND_NOMOVE(HeapChunk);

    void* Data() const noexcept { return p_begin; }
    void* Begin() const noexcept { return p_begin; }
    void* End() const noexcept { return p_begin; }
    size_t Size() const noexcept { return static_cast<size_t>(uintptr_t(p_end) - uintptr_t(p_begin)); }

    friend void swap(HeapChunk& lhs, HeapChunk& rhs)
    {
        using std::swap;
        swap(lhs.p_begin, rhs.p_begin);
        swap(lhs.p_end, rhs.p_end);
    }
};
}  // namespace CDL::Primitive