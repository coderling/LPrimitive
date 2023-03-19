#pragma once
#include "Common.hpp"

namespace CDL::Primitive
{
template <typename T>
class alignas(sizeof(intptr_t) * 2) MPSC_Queue final
{
    static_assert(eastl::is_base_of_v<MPSC_Node, T>);

    // make below member individual cache lines avoid false sharing
    char _cacheline_sapce0[ARC_CACHE_LINE_SIZE];
    union
    {
        eastl::atomic<T*> front;
        eastl::atomic<intptr_t> front_intptr;  //
    };
    char _cacheline_sapce1[ARC_CACHE_LINE_SIZE - sizeof(T*)];
    eastl::atomic<T*> back;
    char _cacheline_sapce2[ARC_CACHE_LINE_SIZE - sizeof(T*)];

   public:
    MPSC_Queue() noexcept;

    bool Empty() const noexcept;

    void PushBack(T* node) noexcept;

    void PushBack(T* first_node, T* last_node) noexcept;

    T* TryPopFront() noexcept;
};

template <typename T>
MPSC_Queue<T>::MPSC_Queue() noexcept
{
    front_intptr.store(0, eastl::memory_order_relaxed);
    back.store(nullptr, eastl::memory_order_seq_cst);
}

template <typename T>
bool MPSC_Queue<T>::Empty() const noexcept
{
    return back.load(eastl::memory_order_relaxed) == nullptr;
}

template <typename T>
void MPSC_Queue<T>::PushBack(T* node) noexcept
{
    node->next.store(nullptr, eastl::memory_order_relaxed);
    /*
    : queu is empty init
            thread 1            |   thread 2        |  thread 3 ....
            push node1
            prev = nullptr
                                  push node2
                                  prev = node1
                                                        push node3
                                                        prev node2
            front.store(node1)   prev->next=node2       prev->next=node3

    result: node1->node2->node3
    */
    // try update back
    if (T* prev = back.exchange(node, eastl::memory_order_release))
        {
            // origin back is not null
            // auto prev = back
            // back = node
            // prev->next = node
            prev->next.store(node, eastl::memory_order_release);
        }
    else
        {
            // origin back is  null, push first element
            // just
            front.store(node, eastl::memory_order_release);
        }
}

template <typename T>
void MPSC_Queue<T>::PushBack(T* first_node, T* last_node) noexcept
{
    last_node->next.store(nullptr, eastl::memory_order_relaxed);

    if (T* prev = back.exchange(last_node, eastl::memory_order_release))
        {
            prev->next.store(first_node, eastl::memory_order_release);
        }

    else
        {
            front.store(first_node, eastl::memory_order_release);
        }
}

template <typename T>
T* MPSC_Queue<T>::TryPopFront() noexcept
{
    // only one consumer at the same time
    T *node, *next, *expected;
    // a valid address the first bit must 0, we can use first bit to mark we access the queue
    intptr_t _front = front_intptr.fetch_or(1, eastl::memory_order_acquire);
    if ((_front & 1) | !(_front >> 1))
        {
            // if the old _front first bit == 1 that mean other consumer had get access
            // if queue is empty _front >> 1 == 0
            return 0;
        }

    node = (T*)_front;
    next = static_cast<T*>(node->next.load(eastl::memory_order_relaxed));

    if (!next)
        {
            front.store(nullptr, eastl::memory_order_release);

            expected = node;
            if (!back.compare_exchange_strong(expected, nullptr, eastl::memory_order_acquire, eastl::memory_order_relaxed))
                {
                    /*
                    init only node1 in queue, push incomplete
                    case 1:
                            thread 1                |  thread 2
                        push node2
                        prev=node1
                        back=node2
                        front=node1
                                                        pop front
                                                        node=node1
                                                        next=nullptr
                        (front=nullptr)                 front=nullptr
                                                        back=node2
                                                        node!=back
                                                        front=node=node1
                                                        return nullptr
                        pre->next=node2
                    */

                    front.store(node, eastl::memory_order_release);

                    return nullptr;
                }

            /*
            init only node1 in queue, push incomplete
            case 2:
                    thread 1                |  thread 2
                push node2
                back=front=node1
                before push exchage
                                                pop front
                                                node=node1
                                                next=nullptr
                (front=nullptr)                 front=nullptr
                                                back=node1
                                                node == back
                                                back= nullptr
                                                return node

                try exchange
                prev=front=nullptr
                front=node2
            */
            return node;
        }
    /*
    init queue size > 1, no conflict with push
    just return and update front
    */
    front.store(next, eastl::memory_order_release);

    return node;
}

}  // namespace CDL::Primitive