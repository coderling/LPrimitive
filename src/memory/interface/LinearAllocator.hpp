#pragma once
#include "../../utils/interface/Align.hpp"
#include "../../utils/interface/Common.hpp"
#include "../../utils/interface/CommonDefines.hpp"

namespace DT::Primitive
{
class LinearAllocator
{
    void* const begin = nullptr;
    std::size_t size;
    void* cur;

   public:
    LinearAllocator(void* _begin, void* _end) noexcept
        : begin{_begin},
          size{uintptr_t(_end) - uintptr_t(_begin)},
          cur{_begin}
    {
    }

    template <typename Resource>
    explicit LinearAllocator(const Resource& resource)
        : LinearAllocator{resource.GetBasePtr(), resource.GetEndPtr()}
    {
    }

    NOCOPY_INPLACE(LinearAllocator);

    ~LinearAllocator() noexcept = default;

    void* Allocate(std::size_t size, std::size_t alignment)
    {
        void* const ptr = AlignUp(cur, alignment);
        void* const e_ptr = PtrAdd(ptr, size);
        bool suc = e_ptr <= End();
        if (suc)
            {
                cur = e_ptr;
                return ptr;
            }

        return nullptr;
    }

    // can not spec free block, but we must define the function
    void Free(void*, std::size_t) noexcept {}

    std::size_t MaxSize() const noexcept { return size; }

    std::size_t FreeSize() const noexcept { return size - (uintptr_t(cur) - uintptr_t(begin)); }

   private:
    void* End() const noexcept { return PtrAdd(begin, size); }
};
}  // namespace DT::Primitive