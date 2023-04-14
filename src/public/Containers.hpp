#pragma once
#include <deque>
#include <list>
#include <map>
#include <memory_resource>
#include <queue>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace DT::Primitive
{
template <typename T>
using String = std::pmr::string;

template <typename T>
using Vector = std::pmr::vector<T>;

template <typename T>
using List = std::pmr::list<T>;

template <typename T>
using Queue = std::queue<T, std::pmr::deque<T>>;

template <typename T>
using Deque = std::pmr::deque<T>;

template <typename T>
using Set = std::pmr::set<T>;

template <typename T>
using USet = std::pmr::unordered_set<T>;

template <typename KT, typename VT>
using Map = std::pmr::map<KT, VT>;

template <typename KT, typename VT>
using MMap = std::pmr::multimap<KT, VT>;

template <typename KT, typename VT>
using UMap = std::pmr::unordered_map<KT, VT>;

template <typename KT, typename VT>
using UMMap = std::pmr::unordered_multimap<KT, VT>;

}  // namespace DT::Primitive