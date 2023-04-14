#pragma once

#include <EASTL/atomic.h>
#include <EASTL/unordered_map.h>
#include <eathread/eathread_mutex.h>
#include <stdint.h>
#include "../allocator/AllocatorInterfaces.hpp"
#include "../utils/Align.hpp"
#include "../utils/CommonDefines.hpp"
#include "Common.hpp"

namespace DT::Primitive
{
template <uint32_t ELEMENT_SIZE, typename IDType>
class PObjectFreeQueue
{
    struct Node
    {
        // first 32 bit save version same as PObject
        IDType version;
        eastl::atomic<Node*> next;
    };

    struct SequencedFrontPtr
    {
        Node* ptr;
        intptr_t idx;
    };

    struct FrontPair
    {
        eastl::atomic<Node*> ptr;
        eastl::atomic<intptr_t> idx;
    };

    static_assert(sizeof(PObjectFreeQueue<ELEMENT_SIZE, IDType>::FrontPair) ==
                  sizeof(PObjectFreeQueue<ELEMENT_SIZE, IDType>::SequencedFrontPtr));

    char _cacheline_sapce0[ARC_CACHE_LINE_SIZE];
    union
    {
        FrontPair front_pair;
        eastl::atomic<SequencedFrontPtr> front;
    };

    char _cacheline_sapce1[ARC_CACHE_LINE_SIZE - sizeof(SequencedFrontPtr)];
    eastl::atomic<Node*> back;
    char _cacheline_sapce2[ARC_CACHE_LINE_SIZE - sizeof(void*)];

    union
    {
        struct
        {
            void* base_ptr;
#ifndef NDEBUG
            void* end_ptr;
            uint32_t distance;
#endif
        };

        struct
        {
            eastl::atomic<void*> a_base_ptr;
#ifndef NDEBUG
            eastl::atomic<void*> a_end_ptr;
            eastl::atomic<uint32_t> a_disance;
#endif
        };
    };

   public:
    PObjectFreeQueue(void* _begin, void* _end, size_t aligment, size_t offset) noexcept
        : base_ptr{Primitive::AlignUp(PtrAdd(_begin, offset), aligment)},
          end_ptr{_end}
    {
        uint32_t element_size = ELEMENT_SIZE > sizeof(Node) ? ELEMENT_SIZE : sizeof(Node);
        void* const next = Primitive::AlignUp(PtrAdd(base_ptr, element_size), aligment);
        L_ASSERT_EXPR(next > base_ptr);
        distance = (uint32_t)((uintptr_t)next - (uintptr_t)base_ptr);
        const uint32_t count = (uint32_t)(((uintptr_t)end_ptr - (uintptr_t)base_ptr) / distance);

        Node* current = (Node*)base_ptr;
        for (uint32_t index = 1; index < count; ++index)
            {
                auto tail = PtrAdd(current, distance);
                current->next.store(tail, eastl::memory_order_relaxed);
                current->version = 0;
                current = tail;
            }

        current->next = nullptr;
        current->version = 0;
        front_pair.idx = 0;
        front_pair.ptr = (Node*)base_ptr;
        back.store(current, eastl::memory_order_seq_cst);
    }

    NOCOPY_AND_NOMOVE(PObjectFreeQueue);

    void* PopFront() noexcept
    {
        Node *node, *next;

        SequencedFrontPtr t_front = front.load(eastl::memory_order_acquire);
        SequencedFrontPtr n_front;
        do
            {
                if (t_front.idx & 1)
                    {
                        return nullptr;
                    }

                node = t_front.ptr;
                // check next if nullptr, that is last node here
                if (!(next = node->next.load(eastl::memory_order_relaxed)))
                    {
                        goto BackNodeDCAS;
                    }

                n_front.ptr = next;
                // increase 2, so idx & 1 always zero
                n_front.idx = t_front.idx + 2;
        } while (front.compare_exchange_strong(t_front, n_front, eastl::memory_order_acquire, eastl::memory_order_relaxed));

    BackNodeDCAS:
        // last element make front.idx & 1 ~= 0, so other consumer can't acess the front anymore
        n_front.ptr = t_front.ptr;
        n_front.idx = t_front.idx | 1;

        if (!front.compare_exchange_strong(t_front, n_front, eastl::memory_order_acquire, eastl::memory_order_relaxed))
            {
                // if fail update the front that means other consumer get the front, return nullptr
                return nullptr;
            }

        // success access front, try set the back to nullptr
        n_front.ptr = node;
        if (!back.compare_exchange_strong(n_front.ptr, nullptr, eastl::memory_order_acquire, eastl::memory_order_relaxed))
            {
                // fail, the back is update by other thread, restore access front to 0
                front_pair.idx.fetch_and(~1, eastl::memory_order_release);
            }
        return node;
    }

    void PushBack(void* ptr) noexcept
    {
#ifndef NDEBUG
        auto b_ptr = a_base_ptr.load(eastl::memory_order_relaxed);
        auto e_ptr = a_end_ptr.load(eastl::memory_order_relaxed);
        L_ASSERT_EXPR(ptr >= b_ptr && ptr < e_ptr);
        auto d = a_disance.load(eastl::memory_order_relaxed);
        bool is_valid = false;
        void* current = b_ptr;
        do
            {
                if (current == ptr)
                    {
                        is_valid = true;
                        break;
                    }
                current = PtrAdd(current, d);
        } while (current < e_ptr);

        L_ASSERT_EXPR(is_valid, "try push a ptr that address not out of element in list");
#endif
        Node* node = (Node*)ptr;
        node->next.store(nullptr, eastl::memory_order_relaxed);
        if (Node* prev = back.exchange(node))
            {
                prev->next.store(node, eastl::memory_order_release);
            }
        else
            {
                // first element push
                front_pair.ptr.store(node, eastl::memory_order_relaxed);
                front_pair.idx.fetch_add(1, eastl::memory_order_release);
            }
    }

    void* const GetBasePtr() const noexcept { return base_ptr; }
};

template <uint32_t ELEMENT_SIZE, typename IDType, size_t ALIGNMENT = alignof(std::max_align_t), size_t OFFSET = 0>
class PoolObjectAllocator
{
    PObjectFreeQueue<ELEMENT_SIZE, IDType> queue;

   public:
    PoolObjectAllocator(void* begin, void* end) noexcept
        : queue{begin, end, ALIGNMENT, OFFSET}
    {
    }

    template <typename CHUNK>
    PoolObjectAllocator(CHUNK& chunk) noexcept
        : queue{chunk.Begin(), chunk.End(), ALIGNMENT, OFFSET}
    {
    }

    NOCOPY_AND_NOMOVE(PoolObjectAllocator);

    void* Allocate(size_t size, size_t alignment = ALIGNMENT, size_t offset = OFFSET) noexcept
    {
        L_ASSERT_EXPR(size == ELEMENT_SIZE);
        L_ASSERT_EXPR(alignment == ALIGNMENT);
        L_ASSERT_EXPR(offset == OFFSET);

        return queue.PopFront();
    }

    void Free(void* ptr, size_t) noexcept { queue.PushBack(ptr); }

    void* const GetBasePtr() const noexcept { return queue.GetBasePtr(); }
};

template <typename T, uint32_t FixedCount, typename IDType = uint32_t, uint32_t ALIGNMENT = 16, bool EnableOverFlow = true>
class VersionObjectPool final
{
    // alignment must power of 2
    static_assert((ALIGNMENT & (ALIGNMENT - 1)) == 0);
    // idtype must unsigned integral type
    static_assert(eastl::is_integral_v<IDType> && eastl::is_unsigned_v<IDType>);
    struct Element
    {
        // first 32 bit save version
        eastl::atomic<IDType> version;
        T value;
    };

    static constexpr uint32_t IDBits = sizeof(IDType) * 8;
    static constexpr IDType OverFlowFlagMask = (IDType)1 << (IDBits - 2);
    static constexpr IDType RecycleVersionMask = (IDType)1 << (IDBits - 1);  // if in pool the last bit is zero
    static_assert((OverFlowFlagMask & RecycleVersionMask) == 0);
    static constexpr uint32_t Version_Size = sizeof(Element::version);

   public:
    using PoolAllocator = Primitive::TAllocator<PoolObjectAllocator<sizeof(Element), IDType, ALIGNMENT, 0>,
                                                Primitive::HeapChunk,
                                                Primitive::NoLock,
                                                Primitive::UnTracked>;
    PoolAllocator allocator;
    void const* const pool_ptr_base;
    Primitive::DefaultMemoryAllocator& overflow_allocator;
    eastl::unordered_map<IDType, void*> overflow_map;
    IDType overflow_version_gen = 0;
    mutable EA::Thread::Mutex over_mutex;
#if CDL_DEBUG
    eastl::atomic<uint32_t> allocate_count;
#endif
   public:
    VersionObjectPool(const char* name) noexcept
        : allocator{name, FixedCount * sizeof(Element)},
          pool_ptr_base{allocator.GetAllocateStrategy().GetBasePtr()},
          overflow_allocator{Primitive::GetGlobalAllocator()},
          overflow_map{},
          overflow_version_gen{0}
#if CDL_DEBUG
          ,
          allocate_count{0}
#endif
    {
    }

    ~VersionObjectPool() noexcept
    {
#if CDL_DEBUG
        //L_ASSERT_EXPR(allocate_count == 0);
        LOG_ERROR("un release object count :", allocate_count.load());
#endif
    }

    template <typename... ARGS>
    T* Allocate(const ARGS&&... args) noexcept
    {
        void* ptr = allocator.Allocate(sizeof(Element), ALIGNMENT);
        if (UTILS_LIKELY(ptr))
            {
                // no need to increase version, defautl is one ,and will increase when Deallocate function
                Element* e = (Element*)ptr;
                e->version.fetch_or(RecycleVersionMask, eastl::memory_order_relaxed);
            }
        else
            {
                ptr = overflow_allocator.Allocate(sizeof(Element), ALIGNMENT);
                over_mutex.Lock();
                IDType version = ((++overflow_version_gen) | OverFlowFlagMask);
                Element* e = (Element*)ptr;
                version = version | (RecycleVersionMask);
                e->version.store(version, eastl::memory_order_relaxed);
                overflow_map.emplace(version, ptr);
                over_mutex.Unlock();
            }

        ptr = PtrAdd(ptr, Version_Size);
        T* t = new (ptr) T(eastl::forward<ARGS>(args)...);
#if CDL_DEBUG
        ++allocate_count;
#endif
        return t;
    }

    void DeAllocate(T* t) noexcept
    {
        if (t == nullptr)
            {
                return;
            }

        Element* e = (Element*)PtrSub(t, Version_Size);
        IDType version = e->version.load(eastl::memory_order_acquire);

        if (IsVersionRecyCle(version))
            {
                // already recycle
                return;
            }

        IDType recycle_version = (version + 1) & (~RecycleVersionMask);
        if (!e->version.compare_exchange_strong(version, recycle_version, eastl::memory_order_acquire, eastl::memory_order_relaxed))
            {
                // other thread recyle the ptr first or this ptr has a new version
                return;
            }
        Destruct(*t);
        if (UTILS_LIKELY(IsPoolHandle(version)))
            {
                allocator.Free(e, sizeof(Element));
            }
        else
            {
                overflow_allocator.Free(e, sizeof(Element));
                over_mutex.Lock();
                overflow_map.erase(version);
                over_mutex.Unlock();
            }
#if CDL_DEBUG
        --allocate_count;
#endif
    }

    IDType PointerToID(void* ptr) const noexcept
    {
        if (!ptr)
            {
                return 0;
            }

        Element* e = (Element*)PtrSub(ptr, Version_Size);
        auto version = e->version.load(eastl::memory_order_relaxed);

        if (UTILS_LIKELY(IsPoolHandle(version)))
            {
                auto id = e - (Element*)pool_ptr_base;  //(uintptr_t)e - (uintptr_t)pool_ptr_base;
                return (IDType)id;
            }
        else
            {
                return version;
            }
    }

    void* IDToPointer(const IDType& id) const noexcept
    {
        if (UTILS_LIKELY(IsPoolHandle(id)))
            {
                Element* ptr = (Element*)pool_ptr_base + id;
                ptr = PtrAdd(ptr, Version_Size);
                return ptr;
            }
        else
            {
                void* ptr = nullptr;
                over_mutex.Lock();
                auto iter = overflow_map.find(id);
                if (iter != overflow_map.end())
                    {
                        ptr = iter->second;
                        ptr = PtrAdd(ptr, Version_Size);
                    }
                over_mutex.Unlock();
                return ptr;
            }

        return nullptr;
    }

    bool IsPointerInRecycle(void* ptr) const noexcept { return IsVersionRecyCle(GetPtrVersion(ptr)); }

    bool IsVersionRecyCle(const uint32_t& version) const noexcept { return (version & RecycleVersionMask) == 0; }

    IDType GetPtrVersion(void* ptr) const noexcept
    {
        if (!ptr)
            {
                return 0;
            }

        Element* e = (Element*)PtrSub(ptr, Version_Size);
        return e->version.load(eastl::memory_order_relaxed);
    }

   private:
    bool IsPoolHandle(const uint32_t& version) const noexcept { return (version & OverFlowFlagMask) == 0u; }
};
}  // namespace DT::Primitive