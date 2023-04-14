#pragma once
#include <EASTL/utility.h>
#include <concepts>
#include <mutex>
#include <type_traits>

#include "Align.hpp"
#include "CommonDefines.hpp"
#include "HeapChunk.hpp"
#include "IAllocator.hpp"
#include "Misc.hpp"


namespace DT::Primitive
{
#define CDL_DEFAULT_ALLOCATOR_NAME "cdl allocator"

struct NoLock
{
    void lock() noexcept {}
    void unlock() noexcept {}
};

struct UnTracked
{
    UnTracked() noexcept = default;
    UnTracked(const char* name, void* ptr, size_t size) {}

    void OnAllocate(void*, size_t, size_t, size_t offset, int flags) noexcept {}

    void OnFree(void*, size_t) noexcept {}

    void OnReset() noexcept {}

    void OnRewind(void*) noexcept {}
};

struct NullChunk
{
    void* Data() const noexcept { return nullptr; }
    size_t Size() const noexcept { return 0; }
};

template <typename AllocateStrategy,
          typename MemoryStrategy = HeapChunk,
          typename LockStrategy = NoLock,
          typename TrackStrategy = UnTracked>
class TAllocator : public IAllocator
{
#if CDL_NAME_ENABLED
    const char* name = CDL_DEFAULT_ALLOCATOR_NAME;
#endif
    MemoryStrategy chunk;
    AllocateStrategy allocator;
    LockStrategy lock;
    TrackStrategy track;

    using LockGuard = std::lock_guard<LockStrategy>;

   public:
    TAllocator() noexcept = default;
    explicit TAllocator(const char* _name) noexcept
#if CDL_NAME_ENABLED
        : name{_name}
#endif
    {
    }

    template <typename... ARGS>
    TAllocator(const char* _name, size_t size, ARGS&&... args)
        :
#if CDL_NAME_ENABLED
          name{_name},
#endif
          chunk{size},
          allocator{chunk, eastl::forward<ARGS>(args)...},
          track{GetName(), chunk.Data(), chunk.Size()}
    {
    }

    template <typename... ARGS>
    TAllocator(const char* _name, MemoryStrategy&& _chunk, ARGS&&... args)
        :
#if CDL_NAME_ENABLED
          name{_name},
#endif
          chunk{eastl::forward<MemoryStrategy>(chunk)},
          allocator{chunk, eastl::forward<ARGS>(args)...},
          track{GetName(), chunk.Data(), chunk.Size()}
    {
    }

    NOCOPY_INPLACE(TAllocator);

    void* Allocate(size_t size, size_t alignment = alignof(std::max_align_t), size_t offset = 0, int flags = 0) noexcept override
    {
        LockGuard guard(lock);
        void* p = allocator.Allocate(size, alignment, offset);
        track.OnAllocate(p, size, alignment, offset, flags);
        return p;
    }

    // for safety
    // trivially destructible对象的内存可以在不调用析构函数的情况下被重用，不允许申请非trivially destructible 的对象
    // 因为这里并不会在free()的时候调用析构函数
    template <typename T>
        requires eastl::is_trivially_destructible<T>::value
    T* Allocate(size_t count, size_t alignmet = alignof(std::max_align_t), int flags = 0) noexcept
    {
        return (T*)Allocate(count * sizeof(T), alignmet, 0, flags);
    }

    void Free(void* ptr, size_t size) noexcept override
    {
        if (ptr)
            {
                LockGuard guard(lock);
                track.OnFree(ptr, size);
                allocator.Free(ptr, size);
            }
    }

    void Reset() noexcept
    {
        LockGuard guard(lock);
        track.OnReset();
        allocator.Reset();
    }

    void Rewind(void* address) noexcept
    {
        LockGuard guard(lock);
        track.OnRewind(address);
        allocator.Rewind(address);
    }

    template <typename T, size_t alignment = alignof(T), typename... ARGS>
    T* MakeObject(ARGS&&... args)
    {
        void* const p = this->Allocate(sizeof(T), alignment);
        return p ? new (p) T(eastl::forward<ARGS>(args)...) : nullptr;
    }

    template <typename T>
    void Destroy(T* ptr) noexcept
    {
        if (ptr)
            {
                ptr->~T();
                this->Free(ptr, sizeof(T));
            }
    }

    const char* GetName() const noexcept
    {
#if CDL_NAME_ENABLED
        return name;
#else
        return CDL_DEFAULT_ALLOCATOR_NAME;
#endif
    }

    void SetName(const char* name) const noexcept
    {
#if CDL_NAME_ENABLED
        this->name = name;
#else
#endif
    }

    AllocateStrategy& GetAllocateStrategy() noexcept { return allocator; }
    const AllocateStrategy& GetAllocateStrategy() const noexcept { return allocator; }

    TrackStrategy& GetTrackStrategy() noexcept { return track; }
    const TrackStrategy& GetTrackStrategy() const noexcept { return track; }

    MemoryStrategy& GetMemoryStrategy() noexcept { return chunk; }
    const MemoryStrategy& GetMemoryStrategy() const noexcept { return chunk; }

    void SetTrackStrategy(TrackStrategy _track) noexcept { eastl::swap(track, _track); }

    // https://en.cppreference.com/w/cpp/named_req/Swappable
    friend void swap(TAllocator& lhs, TAllocator& rhs) noexcept
    {
        using eastl::swap;
#if CDL_NAME_ENABLED
        swap(lhs.name, rhs.name);
#endif
        swap(lhs.allocator, rhs.allocator);
        swap(lhs.chunk, rhs.chunk);
        swap(lhs.lock, rhs.lock);
        swap(lhs.track, rhs.track);
    }
};

}  // namespace DT::Primitive