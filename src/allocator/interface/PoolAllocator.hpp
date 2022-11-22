#pragma once

#include <EASTL/atomic.h>
#include <EASTL/utility.h>
#include "Align.hpp"
#include "CommonDefines.hpp"
#include "DebugUtility.hpp"
#include "Misc.hpp"

namespace CDL::Primitive
{
class FreeList
{
    struct Node
    {
        Node* next;
    };

    Node* head;

#ifndef NDEBUG
    void* begin;
    void* end;
    size_t edistance;
#endif

   public:
    FreeList(void* _begin, void* _end, size_t element_size, size_t alignment, size_t offset) noexcept
    {
        void* const ptr = Misc::PtrAdd(begin, offset);
        void* const b_ptr = AlignUp(ptr, alignment);
        void* const next = AlignUp(Misc::PtrAdd(b_ptr, element_size), alignment);

        L_ASSERT_EXPR(next > b_ptr);

        const size_t distance = uintptr_t(next) - uintptr_t(b_ptr);

        const size_t count = (uintptr_t(end) - uintptr_t(b_ptr)) / distance;

        // element_size 至少是大于sizeof(void*)的，这里地址强转为Node*
        // 复用内存作为内部记录

        head = static_cast<Node*>(b_ptr);
#ifndef NDEBUG
        begin = b_ptr;
        end = _end;
        edistance = distance;
#endif

        Node* current = head;
        for (size_t index = 1; index < count; ++index)
            {
                current->next = Misc::PtrAdd(current, distance);
                current = current->next;
            }

        L_ASSERT_EXPR(current < end);
        L_ASSERT_EXPR(Misc::PtrAdd(current, distance) <= end);
        current->next = nullptr;
    }

    NOCOPY_AND_NOMOVE(FreeList);

    // FreeList(FreeList&& rhs) noexcept;
    // FreeList& operator=(FreeList&& rhs) noexcept;

    void* Pop() noexcept
    {
        Node* const c_head = head;
        head = c_head ? head->next : nullptr;
        L_ASSERT_EXPR(!head || head >= begin && head < end);
        return head;
    }

    void Push(void* ptr) noexcept
    {
        L_ASSERT_EXPR(ptr != nullptr);
        L_ASSERT_EXPR(ptr >= begin && ptr <= end);
#ifndef NDDEBUG
        void* current = begin;
        bool valid = false;
        do
            {
                if (current == ptr)
                    {
                        valid = true;
                        break;
                    }

                current = Misc::PtrAdd(current, edistance);
        } while (current < end);

        L_ASSERT_EXPR(valid, "try push a ptr that address not out of element in list");
#endif
        Node* node = static_cast<Node*>(ptr);
        node->next = head;
        head = node;
    }

    void* First() noexcept { return head; }
};

class ThreadSafeFreeList
{
    struct Node
    {
        eastl::atomic<Node*> next;
    };

    struct alignas(8) HeadPtr
    {
        int32_t offset;
        uint32_t tag;
    };

    eastl::atomic<HeadPtr> head;

    Node* p_storage = nullptr;

   public:
    ThreadSafeFreeList(void* begin, void* end, size_t element_size, size_t alignment, size_t offset) noexcept
    {
        void* const ptr = Misc::PtrAdd(begin, offset);
        void* const b_ptr = AlignUp(ptr, alignment);
        void* const next = AlignUp(Misc::PtrAdd(b_ptr, element_size), alignment);

        L_ASSERT_EXPR(next > b_ptr);

        const size_t distance = uintptr_t(next) - uintptr_t(b_ptr);

        const size_t count = (uintptr_t(end) - uintptr_t(b_ptr)) / distance;

        // element_size 至少是大于sizeof(void*)的，这里地址强转为Node*
        // 复用内存作为内部记录

        Node* head = static_cast<Node*>(b_ptr);
        p_storage = head;

        Node* current = head;
        for (size_t index = 1; index < count; ++index)
            {
                current->next = Misc::PtrAdd(current, distance);
                current = current->next;
            }

        L_ASSERT_EXPR(current < end);
        L_ASSERT_EXPR(Misc::PtrAdd(current, distance) <= end);
        current->next = nullptr;

        this->head.store({int32_t(head - p_storage), 0});
    }

    NOCOPY_AND_NOMOVE(ThreadSafeFreeList);

    void* Pop() noexcept
    {
        Node* const storage = p_storage;
        HeadPtr current_head = head.load();
        while (current_head.offset >= 0)
            {
                Node* const next = storage[current_head.offset].next.load(eastl::memory_order_relaxed);

                const HeadPtr n_head{next ? int32_t(next - storage) : -1, current_head.tag + 1};

                // 此时读到的current_head有可能可能被别的线程先修改了，比较当前值，主要是比较tag的值，因为其他线程提前pop了的话offset是一致的
                // 1. 如果等于我们读取到的值，那么说明没有其他线程修改了head,这里进行修改，完成pop操作
                // 2. 如果不等于我们读取到的值，说明其他线程先于这里进行了修改(tag++ other
                // thread)，if判断失败，并更新当前值到current_head，下一个循环进行新一轮比较，直到满足条件，或者offset<0，表示list已经空了。
                if (head.compare_exchange_weak(current_head, n_head))
                    {
                        assert(!next || next >= storage);
                        break;
                    }
            }

        void* p = (current_head.offset >= 0) ? (storage + current_head.offset) : nullptr;
        assert(!p || p >= storage);

        return p;
    }

    void Push(void* ptr) noexcept
    {
        Node* const storage = p_storage;
        Node* const node = static_cast<Node*>(ptr);
        HeadPtr current_head = head.load();
        HeadPtr n_head = {int32_t(node - storage), current_head.tag + 1};
        do
            {
                n_head.tag = current_head.tag + 1;
                Node* const next = (current_head.offset >= 0) ? (storage + current_head.offset) : nullptr;
                node->next.store(next, eastl::memory_order_relaxed);

        } while (!head.compare_exchange_weak(current_head, n_head));
    }

    void* First() noexcept { return p_storage + head.load(eastl::memory_order_relaxed).offset; }
};

template <size_t ELEMENT_SIZE, size_t ALIGNMENT = alignof(std::max_align_t), size_t OFFSET = 0, typename FreeListType = FreeList>
class PoolAllocator
{
    static_assert(ELEMENT_SIZE >= sizeof(void*), "ELEMENT_SIZE should greater a pointer size");

    FreeListType list;

   public:
    PoolAllocator(void* begin, void* end) noexcept
        : list{begin, end, ELEMENT_SIZE, ALIGNMENT, OFFSET}
    {
    }

    template <typename CHUNK>
    explicit PoolAllocator(CHUNK& chunk) noexcept
        : list{chunk.Begin(), chunk.End(), ELEMENT_SIZE, ALIGNMENT, OFFSET}
    {
    }

    NOCOPY_INPLACE(PoolAllocator);

    PoolAllocator(PoolAllocator&& rhs) noexcept
        : list{eastl::move(rhs.list)}
    {
    }

    PoolAllocator& operator=(PoolAllocator&& rhs) noexcept { list = eastl::move(rhs.list); }

    void* Allocate(size_t size, size_t alignment = alignof(std::max_align_t), size_t offset = 0)
    {
        L_ASSERT_EXPR(size == ELEMENT_SIZE);
        L_ASSERT_EXPR(alignment == ALIGNMENT);
        L_ASSERT_EXPR(offset == OFFSET);

        return list.Pop();
    }

    void Free(void* ptr, size_t) noexcept { list.Push(ptr); }

    constexpr size_t GetSize() const noexcept { return ELEMENT_SIZE; }

    const void* GetBasePtr() noexcept { return list.First(); }
};

template <typename T, size_t ELEMENT_SIZE, size_t OFFSET = 0>
using ObjectPool = PoolAllocator<ELEMENT_SIZE, alignof(T), OFFSET, FreeList>;

template <typename T, size_t ELEMENT_SIZE, size_t OFFSET = 0>
using ThreadSafeObjectPool = PoolAllocator<ELEMENT_SIZE, alignof(T), OFFSET, ThreadSafeFreeList>;

}  // namespace CDL::Primitive