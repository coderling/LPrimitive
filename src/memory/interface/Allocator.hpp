#pragma once
#include <mutex>
#include "../../utils/interface/CommonDefines.hpp"
#include "../../utils/interface/DebugUtility.hpp"

namespace DT::Primitive
{

class IDTAllocator
{
   public:
    virtual void* Allocate(std::size_t size, std::size_t alignment) noexcept = 0;
    virtual void Free(void* ptr, std::size_t size) noexcept = 0;
};

struct NonLock
{
    void lock() noexcept {}
    void unlock() noexcept {}
};

struct UnTracker
{
    UnTracker() noexcept = default;
    UnTracker(const char* name, void* ptr, std::size_t size) {}

    void OnAllocate(void*, std::size_t, std::size_t) noexcept {}

    void OnFree(void*, std::size_t) noexcept {}
};

template <typename MemoryResource, typename Allocator, typename MemoryTracker = UnTracker, typename Locker = NonLock>
class DTAllocator : public IDTAllocator
{
    using LockGuard = std::lock_guard<Locker>;
    MemoryResource resource;
    Allocator allocator;
    MemoryTracker tracker;
    Locker lock;

   public:
    template <typename... ARGS>
    DTAllocator(const char* name, std::size_t size, ARGS&&... args) noexcept
        : resource{size},
          allocator{resource, std::forward<ARGS>(args)...},
          tracker{name, resource.GetBasePtr(), resource.GetSize()}
    {
    }

    template <typename... ARGS>
    DTAllocator(const char* name, void* begin, void* end, ARGS&&... args) noexcept
        : resource{begin, end},
          allocator{resource, std::forward<ARGS>(args)...},
          tracker{name, resource.GetBasePtr(), resource.GetSize()}
    {
    }

    NOCOPY_INPLACE(DTAllocator);

    virtual void* Allocate(std::size_t size, std::size_t alignment) noexcept override
    {
        LockGuard guard(lock);
        allocator.Allocate(size, alignment);
        tracker.OnAllocate(size, alignment);
    }

    virtual void Free(void* ptr, size_t size) noexcept override
    {
        if (UTILS_LIKELY(ptr))
            {
                LockGuard guard(lock);
                allocator.Free(ptr, size);
                tracker.OnFree(ptr, size);
            }
    }

    const MemoryResource& GetResource() const noexcept { return resource; }
    const Allocator& GetAllocator() const noexcept { return allocator; }
};
}  // namespace DT::Primitive