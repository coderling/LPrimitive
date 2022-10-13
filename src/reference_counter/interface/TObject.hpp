#pragma once
#include "ReferenceCounterObject.hpp"

namespace LPrimitive
{
template <typename Interface>
requires std::derived_from<Interface, IObject>
class TObject : public ReferenceCounterObject<Interface>
{
   public:
    static constexpr const IUUID IObject_UUID = UUID_UNKNOWN;
    TObject(IReferenceCounter* _p_refcounter) noexcept
        : ReferenceCounterObject<Interface>(_p_refcounter)
    {
    }
    ~TObject() override {}

    L_RESULT QueryInterface(const IUUID& iid, IObject** pp_interface) override
    {
        if (pp_interface == nullptr)
            {
                L_DEV_CHECK_EXPR(false, "Fail to QueryInterface (", iid.name, ")pp_interface = nullptr!");
                return L_RESULT::TR_ERROR;
            }

        if (iid == UUID_UNKNOWN)
            {
                *pp_interface = this;
                (this)->AddReference();
                return L_RESULT::TR_OK;
            }
        L_DEV_CHECK_EXPR(false, "Fail to QueryInterface ", iid.name, "!");
        return L_RESULT::TR_ERROR;
    }
};

#define IMPLEMENT_QUERYINTERFACE_LOGIC(IID_VALUE, BASE_CLASS_TYPE)                                                                         \
    {                                                                                                                                      \
        if (pp_interface == nullptr)                                                                                                       \
            {                                                                                                                              \
                L_DEV_CHECK_EXPR(false, "Fail to QueryInterface (", iid.name, ")pp_interface = nullptr!");                                 \
                return L_RESULT::TR_ERROR;                                                                                                 \
            }                                                                                                                              \
                                                                                                                                           \
        if (iid == IID_VALUE)                                                                                                              \
            {                                                                                                                              \
                *pp_interface = this;                                                                                                      \
                (this)->AddReference();                                                                                                    \
                return L_RESULT::TR_OK;                                                                                                    \
            }                                                                                                                              \
        else                                                                                                                               \
            {                                                                                                                              \
                return BASE_CLASS_TYPE::QueryInterface(iid, pp_interface);                                                                 \
            }                                                                                                                              \
        L_DEV_CHECK_EXPR(false, "Fail to QueryInterface ", iid.name, "!");                                                                 \
        return L_RESULT::TR_ERROR;                                                                                                         \
    }

#define IMPLEMENT_QUERYINTERFACE(CLASS_TYPE, BASE_CLASS_TYPE, INTERFACE_TYPE)                                                              \
    L_RESULT CLASS_TYPE::QueryInterface(const IUUID& iid, IObject** pp_interface)                                                          \
        IMPLEMENT_QUERYINTERFACE_LOGIC(INTERFACE_TYPE::INTERFACE_TYPE##_UUID, BASE_CLASS_TYPE)

#define IMPLEMENT_QUERYINTERFACE_LOCALLY(BASE_CLASS_TYPE, INTERFACE_TYPE)                                                                  \
    IMPLEMENT_QUERYINTERFACE_STATEMENT()                                                                                                   \
    IMPLEMENT_QUERYINTERFACE_LOGIC(INTERFACE_TYPE::INTERFACE_TYPE##_UUID, BASE_CLASS_TYPE)

#define IMPLEMENT_QUERYINTERFACE_STATEMENT() L_RESULT QueryInterface(const IUUID& iid, IObject** pp_interface) override

#define IMPLEMENT_CONSTRUCT_STATEMENT(CLASS_TYPE, ...)                                                                                     \
    template <typename ObjectType, typename AllocatorType>                                                                                 \
    friend class LPrimitive::MakeReferenceCounter;                                                                                         \
    explicit CLASS_TYPE(IReferenceCounter* p_refcounter __VA_OPT__(, )##__VA_ARGS__) noexcept

#define IMPLEMENT_CONSTRUCT_DEFINE_HEAD(CLASS_TYPE, ...)                                                                                   \
    CLASS_TYPE::CLASS_TYPE(IReferenceCounter* p_refcounter __VA_OPT__(, )##__VA_ARGS__) noexcept

#define IMPLEMENT_CONSTRUCT_LOCALLY(CLASS_TYPE, ...)                                                                                       \
    template <typename ObjectType, typename AllocatorType>                                                                                 \
    friend class LPrimitive::MakeReferenceCounter;                                                                                         \
    explicit CLASS_TYPE(IReferenceCounter* p_refcounter __VA_OPT__(, )##__VA_ARGS__) noexcept

#define IMPLEMENT_CONSTRUCT_INIT_LIST(TBASE, ...)                                                                                          \
        :TBASE(p_refcounter __VA_OPT__(,) ##__VA_ARGS__)

}  // namespace LPrimitive