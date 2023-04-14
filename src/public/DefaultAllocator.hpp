#pragma once
#include <memory_resource>

namespace DT::Primitive
{

struct IDTAllocator
{
    void* (*Allocate)(std::size_t bytes, std::size_t aligment);
    void* (*DeAllocate)(std::size_t bytes, std::size_t aligment);
};

IDTAllocator* GetDefaultAllocator();
}  // namespace DT::Primitive