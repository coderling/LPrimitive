#include "DataBlob.hpp"
#include <EASTL/memory.h>
#include <iostream>
#include "DefaultMemoryAllocator.hpp"
#include "Logger.hpp"

namespace CDL::Primitive
{
RefCountPtr<DataBlob> DataBlob::Create(const size_t& size, const void* p_data)
{
    auto raw_ptr = MAKE_REF_OBJECT(DataBlob, GetGlobalAllocator(), "DataBlob")(size, p_data);
    return RefCountPtr<DataBlob>(raw_ptr);
}

IMPLEMENT_CONSTRUCT_DEFINE_HEAD(DataBlob, size_t size, const void* p_data)
IMPLEMENT_CONSTRUCT_INIT_LIST(TBase), data_buffer(size)
{
    Resize(size);
    if (size > 0)
        {
            if (p_data != nullptr)
                {
                    memcpy(data_buffer.data(), p_data, size);
                }
        }
}

DataBlob::~DataBlob() noexcept {}

IMPLEMENT_QUERYINTERFACE(DataBlob, TBase, IDataBlob)

size_t DataBlob::GetDataSize() const { return data_buffer.size(); }

void* DataBlob::GetDataPointer()
{
    // 调用const 版本
    return const_cast<void*>((static_cast<const DataBlob*>(this))->GetDataPointer());
}

const void* DataBlob::GetDataPointer() const { return data_buffer.data(); }

void DataBlob::Resize(const size_t& size) { data_buffer.resize(size); }

void DataBlob::OnDestroy() {}

}  // namespace CDL::Primitive