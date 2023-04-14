#pragma once
#include <EASTL/type_traits.h>

#include "Common.hpp"

namespace DT::Primitive
{
template <typename T>
class alignas(sizeof(intptr_t) * 2) MPMC_Queue final
{
   public:
   private:
    struct SequencedFrontPtr
    {
        T* ptr;
        intptr_t idx;
    };

    struct FrontPair
    {
        eastl::atomic<T*> ptr;
        eastl::atomic<intptr_t> idx;
    };

    // make below member individual cache lines avoid false sharing
    char _cacheline_sapce0[ARC_CACHE_LINE_SIZE];
    union
    {
        FrontPair front_pair;
        eastl::atomic<SequencedFrontPtr> front;
    };

    char _cacheline_sapce1[ARC_CACHE_LINE_SIZE - sizeof(SequencedFrontPtr)];
    eastl::atomic<T*> back;
    char _cacheline_sapce2[ARC_CACHE_LINE_SIZE - sizeof(T*)];

    static_assert(sizeof(DT::Primitive::MPMC_Queue<T>::front_pair) == sizeof(DT::Primitive::MPMC_Queue<T>::front));

    static_assert(eastl::is_base_of_v<MPMC_Node, T>);
    static constexpr const int32_t ss = sizeof(eastl::atomic<intptr_t>);

   public:
    MPMC_Queue() noexcept;

    bool Empty() const noexcept;

    void PushBack(T* node) noexcept;

    void PushBack(T* first_node, T* last_node) noexcept;

    T* TryPop() noexcept;
};

template <typename T>
MPMC_Queue<T>::MPMC_Queue() noexcept
{
    front_pair.idx.store(1, eastl::memory_order_relaxed);
    back.store(nullptr, eastl::memory_order_seq_cst);
}

template <typename T>
bool MPMC_Queue<T>::Empty() const noexcept
{
    return back.load(eastl::memory_order_relaxed) == nullptr;
}

template <typename T>
void MPMC_Queue<T>::PushBack(T* node) noexcept
{
    // node->next = nullptr
    node->next.store(nullptr, eastl::memory_order_relaxed);
    if (T* prev = back.exchange(node, eastl::memory_order_release))
        {
            // auto prev = back
            // back = node
            // prev->next = node
            prev->next.store(node, eastl::memory_order_release);
        }
    else
        {
            // prev == nullptr first set the front
            front_pair.ptr.store(node, eastl::memory_order_relaxed);
            front_pair.idx.fetch_add(1, eastl::memory_order_release);
        }
}

template <typename T>
void MPMC_Queue<T>::PushBack(T* first_node, T* last_node) noexcept
{
    last_node->next.store(nullptr, eastl::memory_order_relaxed);
    if (T* prev = back.exchange(last_node, eastl::memory_order_release))
        {
            prev->next.store(first_node, eastl::memory_order_release);
        }
    else
        {
            front_pair.ptr.store(first_node, eastl::memory_order_relaxed);
            front_pair.idx.fetch_add(1, eastl::memory_order_release);
        }
}

template <typename T>
T* MPMC_Queue<T>::TryPop() noexcept
{
    T *node, *next;

    SequencedFrontPtr t_front, value;

    t_front = front.load(eastl::memory_order_acquire);

    do
        {
            // idx & 1 ~= 0 only when queue init or queue is empty
            if (t_front.idx & 1)
                {
                    return nullptr;
                }

            node = t_front.ptr;
            // check the next, if next == nullptr, front equal == back, the last node here
            if (!(next = static_cast<T*>(node->next.load(eastl::memory_order_relaxed))))
                {
                    goto BackNodeDCAS;
                }

            value.ptr = next;
            value.idx = t_front.idx + 2;

            // if no the last node, try get relpace front ,if replace success return node
            // if replace fail, other consumer modify the frong, retry update front
    } while (front.compare_exchange_strong(t_front, value, eastl::memory_order_acquire, eastl::memory_order_relaxed));

    return node;

BackNodeDCAS:
    // make front.idx & 1 ~= 0, so other consumer can't access the front anymore
    value.ptr = t_front.ptr;
    value.idx = t_front.idx | 1;

    if (!front.compare_exchange_strong(t_front, value, eastl::memory_order_acquire, eastl::memory_order_relaxed))
        {
            // if fail set the front, other consumer get the front, return nullptr
            return nullptr;
        }

    // success access front, try set the back to nullptr
    value.ptr = node;
    if (!back.compare_exchange_strong(value.ptr, nullptr, eastl::memory_order_acquire, eastl::memory_order_relaxed))
        {
            // fail the back is updated by other thread, restore access to 0
            front_pair.idx.fetch_and(~1, eastl::memory_order_release);
            return nullptr;
        }

    // all success return node
    return node;
}
}  // namespace DT::Primitive