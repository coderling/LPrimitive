#pragma once

#include <EASTL/utility.h>

#include "CommonDefines.hpp"
namespace CDL::Primitive
{
class StaticChunk
{
    void* p_begin;
    void* p_end;

   public:
    StaticChunk() noexcept = default;

    StaticChunk(void* _p_begin, void* _p_end) noexcept
        : p_begin{_p_begin},
          p_end{_p_end}
    {
    }

    ~StaticChunk() noexcept = default;

    NOCOPY_INPLACE(StaticChunk);

    StaticChunk(StaticChunk&& rhs) noexcept = default;
    StaticChunk& operator=(StaticChunk&& rhs) noexcept = default;

    void* Data() const noexcept { return p_begin; }
    void* Begin() const noexcept { return p_begin; }
    void* End() const noexcept { return p_end; }
    size_t Size() const noexcept { return static_cast<size_t>(uintptr_t(p_end) - uintptr_t(p_begin)); }

    friend void swap(StaticChunk& lhs, StaticChunk& rhs) noexcept
    {
        using eastl::swap;
        swap(lhs.p_begin, rhs.p_begin);
        swap(lhs.p_end, rhs.p_end);
    }
};
}  // namespace CDL::Primitive