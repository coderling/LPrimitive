#pragma once

#include "Allocator.hpp"
#include "HeapAllocator.hpp"

namespace CDL::Primitive
{
using DefaultMemoryAllocator = TAllocator<AllocateStrategy::HeapAllocator, NullChunk, NoLock, UnTracked>;

DefaultMemoryAllocator& GetGlobalAllocator();
}  // namespace CDL::Primitive