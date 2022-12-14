#pragma once
#include <concepts>
#include "CommonDefines.hpp"
#include "IAllocator.hpp"
#include "IObject.hpp"
#include "ReferenceCounter.hpp"

namespace CDL::Primitive
{

template <typename Interface>
    requires std::derived_from<Interface, IObject>
class ReferenceCounterObject : public Interface
{
    template <typename OtherInterface>
    friend class CDL::Primitive::RefCountWeakPtr;
    template <typename OtherInterface>
    friend class CDL::Primitive::RefCountPtr;

    ReferenceCounter* p_refcounter;

   public:
    template <typename... InterfaceArgsType>
    ReferenceCounterObject(IReferenceCounter* _p_refcounter, InterfaceArgsType&&... args) noexcept
        : Interface(std::forward<InterfaceArgsType>(args)...),
          p_refcounter(Debug::StaticPointerCast<ReferenceCounter, IReferenceCounter>(_p_refcounter))
    {
    }

    ~ReferenceCounterObject() override {}

   protected:
    inline long AddReference() override final
    {
        L_ASSERT_EXPR(p_refcounter != nullptr);
        return p_refcounter->AddStrongRef();
    }

    inline long Release() override final
    {
        L_ASSERT_EXPR(p_refcounter != nullptr);
        return p_refcounter->ReleaseStrongRef();
    }

    inline IReferenceCounter* GetReferenceCounter() override final { return p_refcounter; }

   protected:
    void operator delete(void* ptr) { delete[] reinterpret_cast<uint8_t*>(ptr); }

    template <typename AllocatorType>
    void operator delete(void* ptr, AllocatorType* allocator, const char* description, const char* filename, const char* line)
    {
        allocator->Free(ptr);
    }

   private:
    friend class ReferenceCounter;
    template <typename ObjectType, typename AllocatorType>
    friend class MakeReferenceCounter;

    // private overload  delete and new, so only access by MakeReferenceCounter
    void* operator new(std::size_t size) { return new uint8_t[size]; }

    template <typename AllocatorType>
    void* operator new(std::size_t size, AllocatorType* allocator, const char* description, const char* filename, const int& line)
    {
        return allocator->Allocate(size);
    }
};

template <typename ObjectType, typename AllocatorType = IAllocator>
class MakeReferenceCounter
{
    AllocatorType* p_allocator = nullptr;

#ifdef L_DEVELOPMENT
    const char* description;
    const char* filename;
    const int line;
#endif
   public:
    MakeReferenceCounter(AllocatorType* _p_allocator, const char* _description, const char* _filename, const int& _line) noexcept
        : p_allocator(_p_allocator)
#ifdef L_DEVELOPMENT
          ,
          description(_description),
          filename(_filename),
          line(_line)
#endif
    {
    }

    NOCOPY_AND_NOMOVE(MakeReferenceCounter);

    template <typename... ObjectArgsType>
    ObjectType* operator()(ObjectArgsType&&... args)
    {
#ifndef L_DEVELOPMENT
        static constexpr const char* description = "<Unavailable in release build>";
        static constexpr const char* filename = "<Unavailable in release build>";
        static constexpr int line = -1;
#endif

        // handle expection
        ReferenceCounter* p_refcounter = new ReferenceCounter();
        ObjectType* p_obj = nullptr;
        try
            {
                if (this->p_allocator == nullptr)
                    {
                        p_obj = new ObjectType(p_refcounter, std::forward<ObjectArgsType>(args)...);
                    }
                else
                    {
                        p_obj = new (this->p_allocator, description, filename, line)
                            ObjectType(p_refcounter, std::forward<ObjectArgsType>(args)...);
                    }
                p_refcounter->Attach<AllocatorType>(p_obj, this->p_allocator, description, filename, line);
            }
        catch (...)
            {
                // when expection , delete reference
                // In addition, p_refcounter will be delete when refrence counter become 0 and after target object release
                if (p_refcounter != nullptr)
                    {
                        p_refcounter->DeleteSelf();
                        p_refcounter = nullptr;
                    }
            }
        return p_obj;
    }
};

#define MAKE_REF_OBJECT(ObjectType, allocator, description)                                                                                \
    CDL::Primitive::MakeReferenceCounter<ObjectType, typename std::remove_reference<decltype(allocator)>::type>(&(allocator), description, \
                                                                                                                __FILE__, __LINE__)
}  // namespace CDL::Primitive