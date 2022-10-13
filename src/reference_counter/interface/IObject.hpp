#pragma once
#include "CommonDefines.hpp"
#include "InterfaceUUID.hpp"

namespace LPrimitive
{
template <typename OtherInterface>
class RefCountWeakPtr;
template <typename OtherInterface>
class RefCountPtr;
class ReferenceCounter;
}  // namespace LPrimitive

namespace LPrimitive
{
class IReferenceCounter;

class IObject
{
   public:
    virtual L_RESULT QueryInterface(const IUUID& iid, IObject** pp_interface) = 0;
    void Destroy() { OnDestroy(); }
    virtual ~IObject() noexcept {}

   protected:
    template <typename OtherInterface>
    friend class LPrimitive::RefCountWeakPtr;
    template <typename OtherInterface>
    friend class LPrimitive::RefCountPtr;
    friend class LPrimitive::ReferenceCounter;
    virtual long AddReference() = 0;
    virtual long Release() = 0;
    virtual IReferenceCounter* GetReferenceCounter() = 0;
    virtual void OnDestroy() = 0;
};
}  // namespace LPrimitive