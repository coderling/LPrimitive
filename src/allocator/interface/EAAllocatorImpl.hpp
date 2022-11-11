#pragma once

#include <EASTL/allocator.h>
#include "DefaultMemoryAllocator.hpp"
#include "STDAllocator.hpp"

namespace eastl
{
inline allocator::allocator(const char* EASTL_NAME(pName))
{
#if EASTL_NAME_ENABLED
    mpName = pName ? pName : EASTL_ALLOCATOR_DEFAULT_NAME;
#endif
}

inline allocator::allocator(const allocator& EASTL_NAME(alloc))
{
#if EASTL_NAME_ENABLED
    mpName = alloc.mpName;
#endif
}

inline allocator::allocator(const allocator&, const char* EASTL_NAME(pName))
{
#if EASTL_NAME_ENABLED
    mpName = pName ? pName : EASTL_ALLOCATOR_DEFAULT_NAME;
#endif
}

inline allocator& allocator::operator=(const allocator& EASTL_NAME(alloc))
{
#if EASTL_NAME_ENABLED
    mpName = alloc.mpName;
#endif
    return *this;
}

inline const char* allocator::get_name() const
{
#if EASTL_NAME_ENABLED
    return mpName;
#else
    return EASTL_ALLOCATOR_DEFAULT_NAME;
#endif
}

inline void allocator::set_name(const char* EASTL_NAME(pName))
{
#if EASTL_NAME_ENABLED
    mpName = pName;
#endif
}

inline void* allocator::allocate(size_t n, int flags)
{
    return CDL::Primitive::GetGlobalAllocator().Allocate(n, alignof(std::max_align_t), 0, flags);
}

inline void* allocator::allocate(size_t n, size_t alignment, size_t offset, int flags)
{
    return CDL::Primitive::GetGlobalAllocator().Allocate(n, alignment, offset, flags);
}

inline void allocator::deallocate(void* p, size_t n) { return CDL::Primitive::GetGlobalAllocator().Free(p, n); }

inline bool operator==(const allocator&, const allocator&)
{
    return true;  // All allocators are considered equal, as they merely use global new/delete.
}

#if !defined(EA_COMPILER_HAS_THREE_WAY_COMPARISON)
inline bool operator!=(const allocator&, const allocator&)
{
    return false;  // All allocators are considered equal, as they merely use global new/delete.
}
#endif
}  // namespace eastl

namespace CDL::Primitive
{

template <typename AllocatorType>
class EAAllocator
{
    AllocatorType& allocator;

   public:
    explicit EAAllocator(AllocatorType& _allocator) noexcept
        : allocator{_allocator}
    {
    }

    inline EAAllocator(const EAAllocator& alloc) noexcept
        : allocator{alloc.allocator}

    {
    }

    inline EAAllocator(const EAAllocator& alloc, const char* pName) noexcept
        : allocator{alloc.allocator}
    {
    }

    inline EAAllocator& operator=(const EAAllocator& alloc) noexcept
    {
        this->allocator = alloc.allocator;
        return *this;
    }

    void* allocate(size_t n, int flags = 0) { return allocator.Allocate(n, alignof(std::max_align_t), 0, flags); }

    void* allocate(size_t n, size_t alignment, size_t offset, int flags = 0) { return allocator.Allocate(n, alignment, offset, flags); }

    void deallocate(void* p, size_t n) { allocator.Free(p, n); }

    const char* get_name() const { return allocator.GetName(); }
    void set_name(const char* pName) { allocator.SetName(pName); }
};

template <typename AllocatorType>
inline bool operator==(const EAAllocator<AllocatorType>& lhs, const EAAllocator<AllocatorType>& rhs)
{
    return lhs.allocator == rhs.allocator;
}

template <typename AllocatorType>
inline bool operator!=(const EAAllocator<AllocatorType>& lhs, const EAAllocator<AllocatorType>& rhs)
{
    return !(lhs == rhs);
}
}  // namespace CDL::Primitive