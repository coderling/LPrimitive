#pragma once

#include <atomic>
#include <mutex>
#include "Atomics.hpp"
#include "DebugUtility.hpp"
#include "IObject.hpp"
#include "IReferenceCounter.hpp"
#include "SpinLock.hpp"

namespace CDL::Primitive
{
class ReferenceCounter final : public IReferenceCounter
{
    struct ObjectHandleBase;
    enum EObjectState : int;

    AtomicInt32 count_strongref;
    AtomicInt32 count_weakref;
    ObjectHandleBase* p_object_handle;
    std::atomic<EObjectState> object_state;
    Spinlock lock;

    enum EObjectState : int
    {
        DEAD = -1,
        NOT_INIT = 0,
        ALIVE = 1,
    };

    struct ObjectHandleBase
    {
        virtual ~ObjectHandleBase() {}
        virtual void DestroyObject() = 0;
        virtual void QueryInterface(const IUUID& iid, IObject** pp_object) = 0;
    };

    template <typename AllocatorType>
    struct ObjectHandle final : ObjectHandleBase
    {
        ObjectHandle(IObject* _p_object, AllocatorType* _p_allocator, const char* _description, const char* _filename, const int& _line)
            : p_object{_p_object},
              p_allocator{_p_allocator}
#ifdef L_DEVELOPMENT
              ,
              description{_description},
              filename{_filename},
              line{_line}
#endif
        {
        }
        void DestroyObject() noexcept override
        {
            if (p_allocator != nullptr)
                {
#ifdef L_DEVELOPMENT
                    LOG_INFO("destroy object: ", filename, ":", line, " description (", description, ")");
#endif
                    p_object->Destroy();
                    p_object->~IObject();
                    p_allocator->Free(p_object);
                }
            else
                {
                    LOG_INFO("destroy object");
                    delete p_object;
                }
        }
        void QueryInterface(const IUUID& iid, IObject** pp_interface) override { p_object->QueryInterface(iid, pp_interface); }

        ~ObjectHandle() noexcept override
        {
            p_object = nullptr;
            p_allocator = nullptr;
        }

       private:
        IObject* p_object;
        AllocatorType* p_allocator;

#ifdef L_DEVELOPMENT
        const char* description;
        const char* filename;
        const int line;
#endif
    };

   private:
    template <typename ObjectType, typename AllocatorType>
    friend class MakeReferenceCounter;
    ReferenceCounter() noexcept
        : count_strongref{0},
          count_weakref{0},
          p_object_handle{nullptr},
          object_state(EObjectState::NOT_INIT)
    {
    }

    // only new by MakeReferenceCounter, and delete here
    void DeleteSelf() { delete this; }

    template <typename AllocatorType>
    inline void Attach(IObject* p_object, AllocatorType* p_allocator, const char* _description, const char* _filename, const int& _line)
    {
        L_ASSERT_EXPR(object_state == EObjectState::NOT_INIT, "already attached by anthoer object!!!");
        p_object_handle = new ObjectHandle(p_object, p_allocator, _description, _filename, _line);
        object_state.store(EObjectState::ALIVE);
    }

   public:
    inline long AddStrongRef() override
    {
        L_ASSERT_EXPR(EObjectState::ALIVE == object_state,
                      "Attempinting to increment strong reference for a object which is not initialied or destoryed!!!");
        return Atomics::Increment(count_strongref);
    }

    inline long ReleaseStrongRef() override
    {
        L_ASSERT_EXPR(EObjectState::ALIVE == object_state,
                      "Attempinting to increment strong reference for a object which is not initialied or destoryed!!!");

        const auto ref_count = Atomics::Decrement(count_strongref);

        L_ASSERT_EXPR(ref_count >= 0, "error decrement strong reference make strong referencs < 0 !!!!");
        if (ref_count == 0)
            {
                // start destroy
                // ???GetObject()?????????????????????????????????GetObject()????????????

                // get lock
                std::unique_lock<Spinlock> u_lock(lock);

                // ??????GetObject()????????????????????????????????????????????????count_strongref +1 -1???????????????????????????
                // ??????ReleaseStrongRef() ????????????????????????GetObject() ??????????????????Destroy()??????????????????
                L_ASSERT_EXPR(count_strongref == 0 && object_state == EObjectState::ALIVE,
                              "while enter Destroy , reference must be equal 0 and object is alive!!!");

                if (count_strongref == 0 && object_state == EObjectState::ALIVE)
                    {
                        // first mask as Destroyed
                        object_state.store(EObjectState::DEAD);

                        // check the weak count, if > 0, can not delete self
                        bool can_delete_self = count_weakref == 0;

                        // must unlock first, before ~ReferenceCounter() avoid twice u_lock.~() cause crash.
                        u_lock.unlock();
                        p_object_handle->DestroyObject();

                        // no reference, delete self.
                        if (can_delete_self)
                            {
                                DeleteSelf();
                            }
                    }
                else
                    {
                        // do nothing
                        // auto release lock
                    }
            }
        return ref_count;
    }
    inline long GetNumOfStrongRef() const override { return count_strongref; }

    inline long AddWeakRef() override
    {
        L_ASSERT_EXPR(EObjectState::ALIVE == object_state,
                      "Attempinting to increment weak reference for a object which is not initialied or destoryed!!!");
        return Atomics::Increment(count_weakref);
    }

    inline long ReleaseWeakRef() override
    {
        // ???????????????Decrement????????????Lock??????????????????ReleaseStrongRef()??????????????????count_weakref==0??????????????????????????????decr
        // count_weakref???????????????????????????????????????????????????????????????decrement count_weakref == 0 => Release
        // ?????????
        /* thread1 : ReleaseWeakRef         |   thread2: AddWeakRef      |               thread2: ReleaseStrongRef
                                            |                            |           2. wait and get lock
             1. decr count_weakref == 0     |                            |
                                            |                            |           3. check count_weakRef == 0
             2. wait lock                   |    1. AddWeakRef() > 0     |
                                            |                            |           4. delete self !!! error delete here
                                            |                            |
                                            |                            |           5. release lock
        */

        std::unique_lock<Spinlock> u_lock(lock);
        L_ASSERT_EXPR(EObjectState::ALIVE == object_state,
                      "Attempinting to increment weak reference for a object which is not initialied or destoryed!!!");

        const auto ref_count = Atomics::Decrement(count_weakref);

        L_ASSERT_EXPR(ref_count >= 0, "error decrement weak reference make weak referencs < 0 !!!!");

        //
        if (ref_count == 0 && object_state == EObjectState::DEAD)
            {
                // if ref_count == 0 and object_state == EObjectState::ALIVE
                // ReleaseStrongRef() will take care of delete self
                L_ASSERT_EXPR(count_strongref == 0);

                // must unlock first, before ~ReferenceCounter() avoid twice u_lock.~() cause crash.
                u_lock.unlock();
                DeleteSelf();
            }
        else
            {
                // auto release lock
            }

        return ref_count;
    }

    inline long GetNumOfWeakRef() const override { return count_weakref; }

    inline void GetObject(void** pp_object) override
    {
        // return ref and add strongref
        if (object_state != EObjectState::ALIVE)
            {
                return;
            }

        // ??????1???
        /* thread1 : GetObject                          |               thread2: ReleaseStrongRef
                                                        |           1. decr count: count_strongRef == 0: ???????????????????????????decr count == 0
                                                                        ???????????????GetObject() ??????????????????
            1. check object alive
            2. wait and get  lock                       |           2. wait lock
            3. count_strongRef > 0                      |           [1. decr count: count_strongRef == 0]
            4. Inc count_strongRef : QueryInterface     |           [2. wait lock]
            5. return ref                               |
            6. release lock                             |
                                                        |           7. get lock
                                                        |           8. check count_strongRef > 0
                                                        |           9. release lock and do not destroy object
        */

        // ??????2???
        /* thread1 : GetObject                      |               thread2: ReleaseStrongRef
                                                    |           1. decr count: count_strongRef == 0
            2. wait lock                            |           2. wait and get lock
                                                    |           3. check count_strongRef == 0
                                                    |           4. mask object destroy and destroy object
                                                    |           5. destroy self counter
                                                    |           6. release lock
            3. get lock                             |
            4. check object not alive and no count  |
            5. do not return ref                    |
            6. release lock                         |
        */

        // get lock
        std::lock_guard<Spinlock> guard(lock);
        // ?????????????????????????????????????????????ReleaseStrongRef()????????????????????????Destroy?????????
        // ??????3???
        /* thread1 : GetObject                          |               thread2: ReleaseStrongRef
                                                        |
            1. check object alive                       |
            2. wait and get  lock                       |
            3. inc count_strongRef                      |
                                                        |                [1. decr count: count_strongRef > 0, return ]
                                                                         ??????GetObject???????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
            4. Inc count_strongRef : QueryInterface     |
            5. return ref                               |
            6. release lock                             |
        */
        const auto ref_count = Atomics::Increment(count_strongref);
        if (object_state == EObjectState::ALIVE && ref_count > 1)
            {
                // return ref
                p_object_handle->QueryInterface(UUID_UNKNOWN, reinterpret_cast<IObject**>(pp_object));
            }
        Atomics::Decrement(count_strongref);
        // guard.~() releaes lock
    }

    ~ReferenceCounter() noexcept
    {
        L_ASSERT_EXPR(count_strongref == 0 && count_weakref == 0, "Has references to object when the object destoryed!!!");
        if (p_object_handle != nullptr)
            {
                delete p_object_handle;
                p_object_handle = nullptr;
            }
    }
};

}  // namespace CDL::Primitive