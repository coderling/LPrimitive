#pragma once
#include <array>
#include <memory_resource>
#include "../../utils/interface/Align.hpp"
#include "../../utils/interface/Common.hpp"
#include "../../utils/interface/CommonDefines.hpp"

namespace DT::Primitive
{
class PMRResource : public std::pmr::memory_resource
{
    void* do_allocate(std::size_t bytes, std::size_t alignment) override
    {
        if (alignment < MIN_ALLOCATE_ALIGNMENT) alignment = MIN_ALLOCATE_ALIGNMENT;

        void* p = ::operator new(bytes, std::align_val_t{alignment});
        return p;
    }

    void do_deallocate(void* p, std::size_t bytes, std::size_t alignment) override
    {
        if (alignment < MIN_ALLOCATE_ALIGNMENT) alignment = MIN_ALLOCATE_ALIGNMENT;
        return ::operator delete(p, bytes, ::std::align_val_t{alignment});
    }

    bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override { return this == &other; }
};

class NullResource
{
   public:
    void* const GetBasePtr() const noexcept { return nullptr; }
    constexpr std::size_t GetSize() const noexcept { return 0; }
};

class HeapResource
{
    void* const begin = nullptr;
    void* const end = nullptr;

   public:
    explicit HeapResource(std::size_t _size) noexcept
        : begin{::operator new(_size, std::align_val_t{MIN_ALLOCATE_ALIGNMENT})},
          end{PtrAdd(begin, _size)}
    {
    }

    ~HeapResource() noexcept { ::operator delete(begin, GetSize(), std::align_val_t{MIN_ALLOCATE_ALIGNMENT}); }

    NOCOPY_AND_NOMOVE(HeapResource);

    void* const GetBasePtr() const noexcept { return begin; }
    void* const GetEndPtr() const noexcept { return end; }
    std::size_t GetSize() const noexcept { return uintptr_t(end) - uintptr_t(begin); }
};

class StaticResource
{
    void* const begin = nullptr;
    void* const end = nullptr;

   public:
    explicit StaticResource(void* _begin, void* _end) noexcept
        : begin{_begin},
          end{_end}
    {
    }

    ~StaticResource() noexcept {}

    NOCOPY_AND_NOMOVE(StaticResource);

    void* const GetBasePtr() const noexcept { return begin; }
    void* const GetEndPtr() const noexcept { return end; }
    std::size_t GetSize() const noexcept { return uintptr_t(end) - uintptr_t(begin); }
};

template <typename T, typename std::size_t size>
class FixedStackResource
{
    std::array<T, size> buffer;

   public:
    explicit FixedStackResource() noexcept {}
    NOCOPY_AND_NOMOVE(FixedStackResource);
    void* const GetBasePtr() const noexcept { return buffer.data(); }
    std::size_t GetSize() const noexcept { return buffer.size(); }
};

}  // namespace DT::Primitive