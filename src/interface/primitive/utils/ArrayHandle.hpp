#pragma once

#include <stdint.h>
#include "../allocator/AllocatorInterfaces.hpp"
#include "Common.hpp"

namespace CDL::Primitive
{
struct ArrayHeader
{
    size_t length;
    size_t capacity;
};

#define MIN_CAPACITY 4

struct ArrayHandler
{
    template <typename T>
    static inline ArrayHeader* ArrayHead(T* arr)
    {
        assert(arr != nullptr);
        auto h = (ArrayHeader*)((char*)arr - sizeof(ArrayHeader));
        return h;
    }

    template <typename T>
    static inline void ArrayAlloc(T** parr, uint32_t request_capacity)
    {
        auto arr = *parr;
        auto* header = arr ? ArrayHead(arr) : nullptr;
        size_t capacity = header ? header->capacity : 0;
        capacity = capacity * 2 > capacity + request_capacity ? capacity * 2 : capacity + request_capacity;

        capacity = capacity < (MIN_CAPACITY) ? (MIN_CAPACITY) : capacity;
        void* ptr = (Primitive::GetGlobalAllocator().Allocate(capacity * sizeof(T) + sizeof(ArrayHeader)));
        T* n_arr = (T*)((char*)ptr + sizeof(ArrayHeader));
        if (header)
            {
                const auto& len = header->length;
                memcpy(n_arr, arr, header->capacity * sizeof(T));
                Primitive::GetGlobalAllocator().Free((void*)header, sizeof(ArrayHeader) + header->capacity * sizeof(T));
                auto* n_header = ArrayHead(n_arr);
                n_header->length = len;
                n_header->capacity = capacity;
            }
        else
            {
                auto* n_header = ArrayHead(n_arr);
                n_header->length = 0;
                n_header->capacity = capacity;
            }
        *parr = n_arr;
    }

    template <typename T>
    static inline void ArrayFree(T** parr)
    {
        auto arr = *parr;
        auto* header = arr ? ArrayHead(arr) : nullptr;
        if (header) Primitive::GetGlobalAllocator().Free((void*)header, sizeof(ArrayHeader) + header->capacity * sizeof(T));
        *parr = nullptr;
    }

    template <typename T>
    static inline void ArrayPush(T** parr, T va)
    {
        auto arr = *parr;
        auto len = ArrayHead(arr)->length++;
        arr[len] = va;
    }

    template <typename T, typename... ARGS>
    static inline T& ArrayEmplaceBack(T** parr, ARGS&&... args)
    {
        auto arr = *parr;
        auto p_t = new (arr + ArrayHead(arr)->length) T{eastl::forward<ARGS>(args)...};
        ++ArrayHead(arr)->length;
        return *p_t;
    }

    template <typename T>
    static inline void ArrayDel(T** parr, size_t index)
    {
        ArrayDel(parr, index, 1);
    }

    template <typename T>
    static inline void ArrayDel(T** parr, size_t index, size_t count)
    {
        auto arr = *parr;
        L_ASSERT_EXPR(index < ArrayHead(arr)->length);
        const auto& len = ArrayHead(arr)->length;
        const auto& need_move = len - (index + count);
        for (size_t i = index; i < len; ++i)
            {
                Destruct(arr[i]);
            }

        if (need_move > 0)
            {
                memmove(arr + index, arr + (index + count), sizeof(T) * need_move);
                ArrayHead(arr)->length -= count;
            }
        else
            {
                ArrayHead(arr)->length = index;
            }
    }

    template <typename T>
    static inline void ArrayCheckCapacity(T** parr, uint32_t n)
    {
        auto arr = *parr;
        int64_t c = arr ? (int64_t)(ArrayHead(arr)->length + n) - (int64_t)ArrayHead(arr)->capacity : n;
        if (c > 0)
            {
                ArrayAlloc(parr, c);
            }
    }

    template <typename T>
    static inline void ArrayCheckPush(T** parr, T va)
    {
        ArrayCheckCapacity(parr, 1);
        ArrayPush(parr, va);
    }

    template <typename T, typename... ARGS>
    static inline T& ArrayCheckEmplaceBack(T** parr, ARGS&&... args)
    {
        ArrayCheckCapacity(parr, 1);
        return ArrayEmplaceBack(parr, eastl::forward<ARGS>(args)...);
    }

    template <typename T>
    static inline void ArraySetLength(T** parr, uint32_t len)
    {
        auto arr = *parr;
        if (!arr)
            {
                ArrayAlloc(parr, len);
                ArrayHead(*parr)->length = len;
            }
        else
            {
                ArrayHeader* header = ArrayHead(arr);
                if (header->capacity < len)
                    {
                        ArrayAlloc(parr, len - header->capacity);
                    }
                else if (len < header->length)
                    {
                        for (uint32_t index = len; len < header->length; ++index)
                            {
                                Destruct(arr[index]);
                            }
                    }
                header->length = len;
            }
    }

#define ARRAY_PUSH(arr, va) (CDL::Primitive::ArrayHandler::ArrayCheckPush(&(arr), (va)))

#define ARRAY_EMPLACE_BACK(arr, ...) (CDL::Primitive::ArrayHandler::ArrayCheckEmplaceBack(&(arr)__VA_OPT__(, )##__VA_ARGS__))

#define ARRAY_DELN(arr, index, count) (CDL::Primitive::ArrayHandler::ArrayDel(&(arr), (index), (count)))

#define ARRAY_DEL(arr, index) (CDL::Primitive::ArrayHandler::ArrayDel(&(arr), (index)))

#define ARRAY_FREE(arr) (CDL::Primitive::ArrayHandler::ArrayFree(&(arr)))

#define ARRAY_LENGTH(arr) ((arr) ? CDL::Primitive::ArrayHandler::ArrayHead((arr))->length : 0)

#define ARRAY_CAPACITY(arr) ((arr) ?: CDL::Primitive::ArrayHandler::ArrayHead((arr))->capacity : 0)

#define ARRAY_SETLENGTH(arr, length) (CDL::Primitive::ArrayHandler::ArraySetLength(&(arr), length))
};

}  // namespace CDL::Primitive