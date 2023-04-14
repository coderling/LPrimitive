#pragma once
#include <concepts>
#include "Common.hpp"

namespace DT::Primitive
{

template <typename T>
    requires std::derived_from<T, MPMC_Node>
class alignas(sizeof(intptr_t) * 2) MPMC_Stack final
{
    struct SequencedTopPtr
    {
        T* ptr;
        // mask if other thread had update this ptr
        intptr_t idx;
    };

    // make below member individual cache lines avoid false sharing
    char _cacheline_sapce0[ARC_CACHE_LINE_SIZE];
    eastl::atomic<SequencedTopPtr> top;
    char _cacheline_sapce1[ARC_CACHE_LINE_SIZE - sizeof(SequencedTopPtr)];

   public:
    MPMC_Stack() noexcept;

    bool Empty() const noexcept;

    void PushBack(T*) noexcept;

    void PushBack(T* first, T* last) noexcept;

    T* TryPopBack() noexcept;

    T* TryPopAll() noexcept;
};

template <typename T>
    requires std::derived_from<T, MPMC_Node>
MPMC_Stack<T>::MPMC_Stack() noexcept
{
    SequencedTopPtr zero{nullptr, 0};
    top.store(zero, eastl::memory_order_seq_cst);
}

template <typename T>
    requires std::derived_from<T, MPMC_Node> bool
MPMC_Stack<T>::Empty() const noexcept
{
    return top.load(eastl::memory_order_relaxed).ptr == nullptr;
}

template <typename T>
    requires std::derived_from<T, MPMC_Node>
void MPMC_Stack<T>::PushBack(T* node) noexcept
{
    SequencedTopPtr n_top;
    n_top.ptr = node;

    SequencedTopPtr o_top = top.load(eastl::memory_order_relaxed);

    do
        {
            node->next.store(o_top.ptr, eastl::memory_order_relaxed);
            n_top.idx = o_top.idx + 1;
            // try update top until success
    } while (!top.compare_exchange_strong(o_top, n_top, eastl::memory_order_release, eastl::memory_order_relaxed));
}

template <typename T>
    requires std::derived_from<T, MPMC_Node>
void MPMC_Stack<T>::PushBack(T* first_node, T* last_node) noexcept
{
    SequencedTopPtr n_top;
    n_top.ptr = first_node;

    SequencedTopPtr o_top = top.load(eastl::memory_order_relaxed);
    do
        {
            last_node->next.store(o_top.ptr, eastl::memory_order_relaxed);
            n_top.idx = o_top.idx + 1;
            // if fail other consumer had update this top  ,try update top until success
    } while (!top.compare_exchange_strong(o_top, n_top, eastl::memory_order_release, eastl::memory_order_relaxed));
}

template <typename T>
    requires std::derived_from<T, MPMC_Node>
T* MPMC_Stack<T>::TryPopBack() noexcept
{
    T* node = nullptr;
    SequencedTopPtr n_top{nullptr, 0};
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
            // if fail other consumer had update this top  ,try update top until success
    } while (!top.compare_exchange_strong(o_top, n_top, eastl::memory_order_acquire, eastl::memory_order_relaxed));

    return node;
}

template <typename T>
    requires std::derived_from<T, MPMC_Node>
T* MPMC_Stack<T>::TryPopAll() noexcept
{
    T* node = nullptr;

    SequencedTopPtr n_top{nullptr, 0};
    SequencedTopPtr o_top = top.load(eastl::memory_order_relaxed);
    do
        {
            node = o_top.ptr;
            if (!node)
                {
                    break;
                }

            n_top.idx = o_top.idx + 1;
            // if fail other consumer had update this top  ,try update top until success
    } while (!top.compare_exchange_strong(o_top, n_top, eastl::memory_order_acquire, eastl::memory_order_relaxed));

    return node;
}

}  // namespace DT::Primitive