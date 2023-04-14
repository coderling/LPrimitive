#pragma once
#include <EASTL/type_traits.h>
#include <EASTL/utility.h>
#include <assert.h>
#include <stdint.h>

#include "../utils/Align.hpp"
#include "../utils/Common.hpp"
#include "../utils/CommonDefines.hpp"


namespace DT::Primitive::AllocateStrategy
{

/*
1. allocate linearly
2. free memory back up to specified point, not support free individual block

*/
class LinearAllocator
{
    void* p_begin;
    size_t size;
    size_t cur;

   public:
    LinearAllocator(void* _p_begin, void* _p_end) noexcept
        : p_begin{_p_begin},
          size{uintptr_t(_p_end) - uintptr_t(_p_begin)},
          cur{0}
    {
    }

    template <typename Chunk>
    explicit LinearAllocator(const Chunk& chunk)
        : LinearAllocator{chunk.Begin(), chunk.End()}
    {
    }

    NOCOPY_INPLACE(LinearAllocator);

    LinearAllocator(LinearAllocator&& rhs) noexcept { this->swap(rhs); }
    LinearAllocator& operator=(LinearAllocator&& rhs) noexcept
    {
        if (this != &rhs)
            {
                this->swap(rhs);
            }

        return *this;
    }

    ~LinearAllocator() noexcept = default;

    void* Allocate(size_t size, size_t alignment = alignof(std::max_align_t), size_t offset = 0)
    {
        assert(offset == 0);
        void* const ptr = AlignUp(GetCurrent(), alignment);
        void* const e_ptr = PtrAdd(ptr, size);
        bool suc = e_ptr <= End();
        if (suc)
            {
                SetCurrent(e_ptr);
                return ptr;
            }

        return nullptr;
    }

    // can not spec free block, but we must define the function
    // use STDAllocator
    void Free(void*, size_t) noexcept {}

    void* GetCurrent() noexcept { return PtrAdd(p_begin, cur); }

    void Rewind(void* ptr) noexcept
    {
        assert(ptr >= p_begin && ptr < End());
        SetCurrent(ptr);
    }

    void Reset() noexcept { Rewind(p_begin); }

    size_t MaxSize() const noexcept { return size; }

    size_t Available() const noexcept { return size - cur; }

    void swap(LinearAllocator& rhs) noexcept
    {
        eastl::swap(p_begin, rhs.p_begin);
        eastl::swap(size, rhs.size);
        eastl::swap(cur, rhs.cur);
    }

    void* Begin() noexcept { return p_begin; }

   private:
    void* End() const noexcept { return PtrAdd(p_begin, size); }

    void SetCurrent(void* ptr) { cur = static_cast<size_t>(uintptr_t(ptr) - uintptr_t(p_begin)); }
};
}  // namespace DT::Primitive::AllocateStrategy