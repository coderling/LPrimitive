#pragma once
#include <assert.h>
#include <stdint.h>
#include <type_traits>

namespace CDL::Primitive
{
template <typename IDType = uint32_t, typename UnionIDType = uint64_t>
struct IHandle
{
    using HIDType = IDType;
    using HUnionIDType = UnionIDType;
    static_assert(sizeof(IDType) * 2 == sizeof(UnionIDType), "error idtype");

   public:
    struct ID
    {
        IDType _version;
        IDType _id;
    };
    static constexpr IDType InvalidHandleID = ~((IDType)0);
    static constexpr IDType InvalidHandle = ~((UnionIDType)0);
    union
    {
        ID id;
        UnionIDType CID;
    };

    bool IsNull() const noexcept { return id._id == InvalidHandleID || id._version == InvalidHandleID; }

    constexpr IHandle() noexcept
        : id{InvalidHandleID, InvalidHandleID}
    {
    }

    explicit IHandle(const IDType& _id, const IDType _version) noexcept
        : id{_version, _id}
    {
        assert(!IsNull());
    }

    explicit IHandle(const IDType& _id) noexcept
        : id{InvalidHandleID, _id}
    {
        assert(!IsNull());
    }

    // is handle valid
    explicit operator bool() const noexcept { return !IsNull(); }

    // auto cast
    operator UnionIDType() const noexcept { return CID; }

    bool operator==(const IHandle& rhs) const noexcept { return CID == rhs.CID; }
    bool operator!=(const IHandle& rhs) const noexcept { return CID != rhs.CID; }
};

}  // namespace CDL::Primitive