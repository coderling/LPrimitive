#pragma once

#include <stdint.h>
#include "Handle.h"
#include "VersionObjectPool.hpp"

namespace DT::Primitive
{
template <typename IDType, typename IDUnionType, size_t SIZE, uint32_t ALIGNMENT = 16>
class HandleAllocator final
{
    static_assert(SIZE > 0);

    using HandleType = IHandle<IDType, IDUnionType>;

    struct InnerData
    {
        void* target;

        InnerData() noexcept
            : target{nullptr}
        {
        }
    };

#ifdef CDL_HANDLE_TYPE_CHECK
    mutable EA::Thread::Mutex debug_lock;
    eastl::unordered_map<void*, const char*> handle2typename;
#endif

    Primitive::VersionObjectPool<InnerData, SIZE, IDType> pool;

   public:
    HandleAllocator(const char* name) noexcept
        : pool{name}
    {
    }
    template <typename T>
    IDUnionType Allocate(T* p_target = nullptr) noexcept
    {
        InnerData* h = pool.Allocate();
        L_ASSERT_EXPR(h != nullptr);
        auto id = pool.PointerToID(h);
        auto version = (IDType)pool.GetPtrVersion(h);
        HandleType handle{id, version};

        h->target = p_target;

#ifdef CDL_HANDLE_TYPE_CHECK
        debug_lock.Lock();
        handle2typename.emplace((void*)h, typeid(T).name());
        debug_lock.Unlock();
#endif
        return handle;
    }

    template <typename T>
    IDUnionType ReBind(IDUnionType& id, T* p_target) noexcept
    {
        L_ASSERT_EXPR(IsValid(id));
        if (!IsValid(id))
            {
                LOG_ERROR("try to bind object to handle that is invalid");
                return id;
            }

        HandleType handle{id};
        InnerData* ptr = (InnerData*)pool.IDToPointer(handle.id._id);
        L_ASSERT_EXPR(ptr != nullptr);

        ptr->target = p_target;

#ifdef CDL_HANDLE_TYPE_CHECK
        debug_lock.Lock();
        auto iter = handle2typename.find(ptr);
        if (iter != handle2typename.end())
            {
                L_ASSERT_EXPR(iter->second == typeid(T).name());
            }
        else
            {
                handle2typename[ptr] = typeid(T).name();
            }
        debug_lock.Unlock();
#endif
        return id;
    }

    void DeAllocate(IDUnionType& id) noexcept
    {
        HandleType handle{id};
        if (handle.IsNull())
            {
                return;
            }
        InnerData* ptr = (InnerData*)pool.IDToPointer(handle.id._id);
        pool.DeAllocate(ptr);

        handle.CID = HandleType::InvalidHandle;
    }

    template <typename T>
    inline eastl::enable_if_t<!eastl::is_pointer_v<T>, T*> HandleCast(const IDUnionType& id) noexcept
    {
        HandleType handle{id};
        InnerData* const ptr = (InnerData*)pool.IDToPointer(handle.id._id);
        if (ptr)

            {
#ifdef CDL_HANDLE_TYPE_CHECK
                // 要求
                debug_lock.Lock();
                auto iter = handle2typename.find(ptr);
                if (UTILS_UNLIKELY(iter == handle2typename.end()))
                    {
                        LOG_ERROR_EXCEPTION("InvalidCast: invliad handle");
                    }
                else if (UTILS_UNLIKELY(typeid(T).name() != iter->second))
                    {
                        LOG_ERROR_EXCEPTION_FORMAT("InvalidCast: to {}, but handle's actual type is {}", typeid(T).name(), iter->second);
                    }
                debug_lock.Unlock();
#endif
                if (!IsValid(id))
                    {
                        return nullptr;
                    }
                return (T*)ptr->target;
            }
        return nullptr;
    }

    bool IsValid(const IDUnionType& id) const noexcept
    {
        HandleType handle{id};
        if (handle.IsNull())
            {
                return false;
            }
        auto version = pool.GetPtrVersion(pool.IDToPointer(handle.id._id));
        return !pool.IsVersionRecyCle(version) && version == handle.id._version;
    }

    template <typename T>
    bool IsValidT(const IDUnionType& id) noexcept
    {
#if defined(CDL_DEVELOPMENT) && defined(CDL_HANDLE_TYPE_CHECK)
        if (!IsValid(id))
            {
                return false;
            }
        debug_lock.Lock();
        HandleType handle{id};
        void* const ptr = pool.IDToPointer(handle.id._id);
        auto iter = handle2typename.find(ptr);
        bool valid = iter != handle2typename.end() && iter->second == typeid(T).name();
        debug_lock.Unlock();

        return valid;
#else
        return true;
#endif
    }
};
}  // namespace DT::Primitive