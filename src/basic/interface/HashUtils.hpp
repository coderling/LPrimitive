#pragma once
#include <functional>

namespace Toy::Engine
{
// boost hash_combine
template <typename T>
inline void HashCombine(std::size_t& hash, const T& v)
{
    hash ^= std::hash<T>{}(v) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
}

template <typename FirstArgsType, typename... RestArgsType>
inline void HashCombine(std::size_t& hash_va, const FirstArgsType& arg, const RestArgsType&... rest_args)
{
    HashCombine(hash_va, arg);
    HashCombine(hash_va, rest_args...);
}

template <typename... ArgsType>
inline std::size_t ComputeHash(const ArgsType&... args)
{
    std::size_t hash_va = 0;
    HashCombine(hash_va, args...);

    return hash_va;
}

template <typename Iter>
inline std::size_t HansRange(Iter begin, Iter end)
{
    std::size_t hash = 0;
    for (; begin != end; ++begin)
        {
            HashCombine(hash, *begin);
        }
    return hash;
}

}  // namespace Toy::Engine