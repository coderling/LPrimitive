#pragma once
#include <concepts>
#include <mutex>
#include <type_traits>

#include "Align.hpp"
#include "CommonDefines.hpp"
#include "IAllocator.hpp"
#include "Misc.hpp"

namespace CDL::Primitive
{

struct NoLock
{
    void lock() noexcept {}
    void unlock() noexcept {}
};

struct UnTracked
{
    UnTracked() noexcept = default;
    UnTracked(const char* name, void* ptr, size_t size) {}

    void OnAllocate(void*, size_t, size_t) noexcept {}

    void OnFree(void*, size_t) noexcept {}

    void OnReset() noexcept {}

    void OnRewind(void*) noexcept {}
};

struct NullChunk
{
    void* Data() const noexcept { return nullptr; }
    size_t Size() const noexcept { return 0; }
};

class HeapChunk
{
    void* p_begin = nullptr;
    void* p_end = nullptr;

   public:
    HeapChunk() noexcept = default;

    explicit HeapChunk(size_t size)
    {
        if (size > 0)
            {
                p_begin = malloc(size);
                p_end = Misc::PtrAdd(p_begin, size);
            }
    }

    ~HeapChunk() noexcept { free(p_begin); }

    NOCOPY_AND_NOMOVE(HeapChunk);
};

template <typename AllocateStrategy,
          typename MemoryStrategy = HeapChunk,
          typename LockStrategy = NoLock,
          typename TrackStrategy = UnTracked>
class TAllocator : public IAllocator
{
    const char* name;
    MemoryStrategy chunk;
    AllocateStrategy allocator;
    LockStrategy lock;
    TrackStrategy track;

    using LockGuard = std::lock_guard<LockStrategy>;

   public:
    TAllocator() = default;

    template <typename... ARGS>
    TAllocator(const char* _name, size_t size, ARGS&&... args)
        : name{_name},
          chunk{size},
          allocator{chunk, std::forward<ARGS>(args)...},
          track{name, chunk.Data(), chunk.Size()}
    {
    }

    template <typename... ARGS>
    TAllocator(const char* _name, MemoryStrategy&& _chunk, ARGS&&... args)
        : name{_name},
          chunk{std::forward<MemoryStrategy>(chunk)},
          allocator{chunk, std::forward<ARGS>(args)...},
          track{name, chunk.Data(), chunk.Size()}
    {
    }

    NOCOPY_INPLACE(TAllocator);

    void* Allocate(size_t size, size_t alignment = alignof(std::max_align_t)) noexcept override
    {
        LockGuard guard(lock);
        void* p = allocator.Allocate(size, alignment);
        track.OnAllocate(p, size, alignment);
        return p;
    }

    // for safety
    // trivially destructible对象的内存可以在不调用析构函数的情况下被重用，不允许申请非trivially destructible 的对象
    // 因为这里并不会在free()的时候调用析构函数
    template <typename T>
        requires std::is_trivially_destructible<T>::value
    T* Allocate(size_t count, size_t alignmet = alignof(std::max_align_t)) noexcept
    {
        return (T*)Allocate(count * sizeof(T), alignmet);
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
        return p ? new (p) T(std::forward<ARGS>(args)...) : nullptr;
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

    const char* GetName() const noexcept { return name; }

    AllocateStrategy& GetAllocateStrategy() noexcept { return allocator; }
    const AllocateStrategy& GetAllocateStrategy() const noexcept { return allocator; }

    TrackStrategy& GetTrackStrategy() noexcept { return track; }
    const TrackStrategy& GetTrackStrategy() const noexcept { return track; }

    MemoryStrategy& GetMemoryStrategy() noexcept { return chunk; }
    const MemoryStrategy& GetMemoryStrategy() const noexcept { return chunk; }

    void SetTrackStrategy(TrackStrategy _track) noexcept { std::swap(track, _track); }

    // https://en.cppreference.com/w/cpp/named_req/Swappable
    friend void swap(TAllocator& lhs, TAllocator& rhs) noexcept
    {
        using std::swap;
        swap(lhs.name, rhs.name);
        swap(lhs.allocator, rhs.allocator);
        swap(lhs.chunk, rhs.chunk);
        swap(lhs.lock, rhs.lock);
        swap(lhs.track, rhs.track);
    }
};

}  // namespace CDL::Primitive