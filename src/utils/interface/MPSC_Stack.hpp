#pragma once

#include "Common.hpp"

namespace DT::Primitive
{
template <typename T>
class alignas(sizeof(intptr_t) * 2) MPSC_Stack final
{
    static_assert(eastl::is_base_of_v<MPSC_Node, T>);

    struct SequencedTopPtr
    {
        T* ptr;
        intptr_t idx;
    };

    // make below member individual cache lines avoid false sharing
    char _cacheline_sapce0[ARC_CACHE_LINE_SIZE];
    eastl::atomic<SequencedTopPtr> top;
    char _cacheline_sapce1[ARC_CACHE_LINE_SIZE];
    eastl::atomic<bool> consumer_lock;
    char _cacheline_sapce2[ARC_CACHE_LINE_SIZE - sizeof(bool)];

   public:
    MPSC_Stack() noexcept;

    bool Empty() const noexcept;

    void PushBack(T* node) noexcept;

    void PushBack(T* first_node, T* last_node) noexcept;

    T* TryPopBack() noexcept;

    T* TryPopAll() noexcept;
};

template <typename T>
MPSC_Stack<T>::MPSC_Stack() noexcept
{
    SequencedTopPtr zero{nullptr, 0};
    top.store(zero, eastl::memory_order_relaxed);
    consumer_lock.store(false, eastl::memory_order_seq_cst);
}

template <typename T>
bool MPSC_Stack<T>::Empty() const noexcept
{
    return top.load(eastl::memory_order_relaxed).ptr == nullptr;
}

template <typename T>
void MPSC_Stack<T>::PushBack(T* node) noexcept
{
    SequencedTopPtr n_top;
    n_top.ptr = node;

    SequencedTopPtr o_top = top.load(eastl::memory_order_relaxed);
    do
        {
            node->next.store(o_top.ptr, eastl::memory_order_relaxed);
            n_top.idx = o_top.idx + 1;
    } while (!top.compare_exchange_strong(o_top, n_top, eastl::memory_order_release, eastl::memory_order_relaxed));
}

template <typename T>
void MPSC_Stack<T>::PushBack(T* first_node, T* last_node) noexcept
{
    SequencedTopPtr n_top;
    n_top.ptr = first_node;

    SequencedTopPtr o_top = top.load(eastl::memory_order_relaxed);
    do
        {
            last_node->next.store(o_top.ptr);
            n_top.idx = o_top.idx + 1;
    } while (!top.compare_exchange_strong(o_top, n_top, eastl::memory_order_release, eastl::memory_order_relaxed));
}

// pop front node, return null if another consumer thread has exculsive reading
// or queue is empty。 you can check stack is empty bay "Empty()" function
template <typename T>
T* MPSC_Stack<T>::TryPopBack() noexcept
{
    if (consumer_lock.exchange(true, eastl::memory_order_acquire))
        {
            return nullptr;
        }

    T* node = nullptr;

    SequencedTopPtr n_top;
    SequencedTopPtr o_top = top.load(eastl::memory_order_relaxed);
    do
        {
            node = o_top.ptr;
            if (!node)
                {
                    break;
                }

            n_top.ptr = static_cast<T*>(node->next.load(eastl::memory_order_relaxed));
            n_top.idx = o_top.idx + 1;
    } while (!top.compare_exchange_strong(o_top, n_top, eastl::memory_order_release, eastl::memory_order_relaxed));

    consumer_lock.store(false, eastl::memory_order_release);
    return node;
}

// pop front node, return null if another consumer thread has exculsive reading
// or queue is empty。 you can check stack is empty bay "Empty()" function
template <typename T>
T* MPSC_Stack<T>::TryPopAll() noexcept
{
    if (consumer_lock.exchange(true, eastl::memory_order_acquire))
        {
            return nullptr;
        }

    T* node = nullptr;

    SequencedTopPtr n_top;
    SequencedTopPtr o_top = top.load(eastl::memory_order_relaxed);
    do
        {
            node = o_top.ptr;
            if (!node)
                {
                    break;
                }

            n_top.idx = o_top.idx + 1;
    } while (!top.compare_exchange_strong(o_top, n_top, eastl::memory_order_release, eastl::memory_order_relaxed));

    consumer_lock.store(false, eastl::memory_order_release);
    return node;
}
}  // namespace DT::Primitive