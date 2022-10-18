#pragma once
#include "IDataBlob.hpp"
#include "RefCountPtr.hpp"
#include "TObject.hpp"

namespace CDL::Primitive
{
class DataBlob final : public TObject<IDataBlob>
{
    using TBase = TObject<IDataBlob>;

    std::vector<uint8_t> data_buffer;

   public:
    static RefCountPtr<DataBlob> Create(const std::size_t& size = 0, const void* p_data = nullptr);

    ~DataBlob() noexcept override;

    void OnDestroy() override;

    IMPLEMENT_QUERYINTERFACE_STATEMENT();

    std::size_t GetDataSize() const override;

    void* GetDataPointer() override;

    const void* GetDataPointer() const override;

    void Resize(const std::size_t& size) override;

    template <typename DataType>
    const DataType* GetDataPointer() const noexcept
    {
        return reinterpret_cast<DataType*>(GetDataPointer());
    }

    template <typename DataType>
    DataType* GetDataPointer() noexcept
    {
        // this 转化为const 指针，调用const版本的GetDataPointer() 然后结果去除const
        return const_cast<DataType*>((static_cast<const DataBlob*>(this))->GetDataPointer<DataType>());
    }

   private:
    IMPLEMENT_CONSTRUCT_STATEMENT(DataBlob, std::size_t size = 0, const void* p_data = nullptr);
};
}  // namespace CDL::Primitive