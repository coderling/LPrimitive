#pragma once

#include <limits>
#include <memory>

namespace CDL::Primitive
{
template <typename T>
typename std::enable_if<std::is_destructible<T>::value, void>::type Destruct(T* ptr)
{
    ptr->~T();
}

template <typename T>
typename std::enable_if<!std::is_destructible<T>::value, void>::type Destruct(T* ptr)
{
}

template <typename T, typename AllocatorType>
struct STDAllocator
{
    AllocatorType& allocator;

   public:
    using value_type = T;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using reference = value_type&;
    using const_reference = const value_type&;
    using size_type = size_t;
    using difference_type = std::ptrdiff_t;

    explicit STDAllocator(AllocatorType& allocator) noexcept
        : allocator(allocator)
    {
    }

    template <typename U>
    STDAllocator(const STDAllocator<U, AllocatorType>& other) noexcept
        : allocator(other.allocator)
    {
    }

    template <typename U>
    STDAllocator(STDAllocator<U, AllocatorType>&& other) noexcept
        : allocator(other.allocator)
    {
    }

    template <typename U>
    struct rebind
    {
        typedef STDAllocator<U, AllocatorType> other;
    };

    T* allocate(size_t count) { return reinterpret_cast<T*>(allocator.Allocate(count * sizeof(value_type), 0)); }

    pointer address(reference r) { return &r; }
    const_pointer address(const_reference r) { return &r; }

    void deallocate(T* p, size_t size) { allocator.Free(p, size * sizeof(value_type)); }

    inline size_type max_size() const { return std::numeric_limits<size_type>::max() / sizeof(value_type); }
};

template <class T, class U, class A>
bool operator==(const STDAllocator<T, A>& left, const STDAllocator<U, A>& right)
{
    return &left.allocator == &right.allocator;
}

template <class T, class U, class A>
bool operator!=(const STDAllocator<T, A>& left, const STDAllocator<U, A>& right)
{
    return !(left == right);
}
}  // namespace CDL::Primitive