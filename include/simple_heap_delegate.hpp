#pragma once
#include "detail.hpp"

#include <memory>
#include <type_traits>
#include <utility>

namespace sdaineka
{
template<typename>
class SimpleHeapDelegate;

template<typename TReturn, typename... TArgs>
class SimpleHeapDelegate<TReturn(TArgs...)>
{
public:
    template<typename... TBindArgs>
    using GlobalFuncPtr = TReturn (*)(TArgs..., detail::func_param_t<TBindArgs>...);
    template<typename TClass, typename... TBindArgs>
    using MemberFuncPtr = TReturn (TClass::*)(TArgs..., detail::func_param_t<TBindArgs>...);
    template<typename TClass, typename... TBindArgs>
    using MemberFuncPtrConst = TReturn (TClass::*)(TArgs..., detail::func_param_t<TBindArgs>...) const;

private:
    class StorageBase
    {
    public:
        virtual ~StorageBase()
        {
        }
        virtual TReturn operator()(TArgs... args) const = 0;
    };

    template<typename... TBindArgs>
    class GlobalFuncStorage : public StorageBase
    {
    public:
        GlobalFuncStorage(const GlobalFuncPtr<TBindArgs...> func, TBindArgs... bindArgs)
            : m_func(func)
            , m_bindArgs(std::move(bindArgs)...)
        {
        }

        TReturn operator()(TArgs... args) const override
        {
            return Invoke(std::forward<TArgs>(args)..., std::make_index_sequence<sizeof...(TBindArgs)>{});
        }

    private:
        template<std::size_t... I>
        TReturn Invoke(TArgs... args, std::index_sequence<I...>) const
        {
            return std::invoke(m_func, std::forward<TArgs>(args)..., std::get<I>(m_bindArgs)...);
        }

        GlobalFuncPtr<TBindArgs...> m_func;
        std::tuple<TBindArgs...> m_bindArgs;
    };

    template<typename TClass, typename... TBindArgs>
    class MemberDelegateStorage : public StorageBase
    {
    public:
        MemberDelegateStorage(TClass* cls, const MemberFuncPtr<TClass, TBindArgs...> func, TBindArgs... bindArgs)
            : m_cls(cls)
            , m_func(func)
            , m_bindArgs(std::move(bindArgs)...)
        {
        }

        TReturn operator()(TArgs... args) const override
        {
            return Invoke(std::forward<TArgs>(args)..., std::make_index_sequence<sizeof...(TBindArgs)>{});
        }

    private:
        template<std::size_t... I>
        TReturn Invoke(TArgs... args, std::index_sequence<I...>) const
        {
            return std::invoke(m_func, m_cls, std::forward<TArgs>(args)..., std::get<I>(m_bindArgs)...);
        }

        TClass* m_cls;
        MemberFuncPtr<TClass, TBindArgs...> m_func;
        std::tuple<TBindArgs...> m_bindArgs;
    };

    template<typename TClass, typename... TBindArgs>
    class MemberDelegateStorageConst : public StorageBase
    {
    public:
        MemberDelegateStorageConst(const TClass* cls, const MemberFuncPtrConst<TClass, TBindArgs...> func, TBindArgs... bindArgs)
            : m_cls(cls)
            , m_func(func)
            , m_bindArgs(std::move(bindArgs)...)
        {
        }

        TReturn operator()(TArgs... args) const override
        {
            return Invoke(std::forward<TArgs>(args)..., std::make_index_sequence<sizeof...(TBindArgs)>{});
        }

    private:
        template<std::size_t... I>
        TReturn Invoke(TArgs... args, std::index_sequence<I...>) const
        {
            return std::invoke(m_func, m_cls, std::forward<TArgs>(args)..., std::get<I>(m_bindArgs)...);
        }

        const TClass* m_cls;
        MemberFuncPtrConst<TClass, TBindArgs...> m_func;
        std::tuple<TBindArgs...> m_bindArgs;
    };

    template<typename TFunc, typename... TBindArgs>
    class LambdaDelegateStorage : public StorageBase
    {
    public:
        LambdaDelegateStorage(TFunc&& func, TBindArgs... bindArgs)
            : m_func(std::forward<TFunc>(func))
            , m_bindArgs(std::move(bindArgs)...)
        {
        }

        TReturn operator()(TArgs... args) const override
        {
            return Invoke(std::forward<TArgs>(args)..., std::make_index_sequence<sizeof...(TBindArgs)>{});
        }

    private:
        template<std::size_t... I>
        TReturn Invoke(TArgs... args, std::index_sequence<I...>) const
        {
            return std::invoke(m_func, std::forward<TArgs>(args)..., std::get<I>(m_bindArgs)...);
        }

        TFunc m_func;
        std::tuple<TBindArgs...> m_bindArgs;
    };

public:
    SimpleHeapDelegate() = default;

    SimpleHeapDelegate(const SimpleHeapDelegate&) = delete;
    SimpleHeapDelegate& operator=(const SimpleHeapDelegate&) = delete;
    SimpleHeapDelegate(SimpleHeapDelegate&&) noexcept = default;
    SimpleHeapDelegate& operator=(SimpleHeapDelegate&&) noexcept = default;

    template<typename... TBindArgs>
    static SimpleHeapDelegate CreateGlobal(GlobalFuncPtr<TBindArgs...> func, TBindArgs... bindArgs)
    {
        using StorageType = GlobalFuncStorage<TBindArgs...>;
        return SimpleHeapDelegate(new StorageType(func, std::move(bindArgs)...), sizeof(StorageType));
    }

    template<typename TClass, typename... TBindArgs>
    static SimpleHeapDelegate CreateMember(TClass* cls, MemberFuncPtr<TClass, TBindArgs...> func, TBindArgs... bindArgs)
    {
        using StorageType = MemberDelegateStorage<TClass, TBindArgs...>;
        return SimpleHeapDelegate(new StorageType(cls, func, std::move(bindArgs)...), sizeof(StorageType));
    }

    template<typename TClass, typename... TBindArgs>
    static SimpleHeapDelegate CreateMember(const TClass* cls, MemberFuncPtrConst<TClass, TBindArgs...> func, TBindArgs... bindArgs)
    {
        using StorageType = MemberDelegateStorageConst<TClass, TBindArgs...>;
        return SimpleHeapDelegate(new StorageType(cls, func, std::move(bindArgs)...), sizeof(StorageType));
    }

    template<typename TFunc, typename... TBindArgs>
    static SimpleHeapDelegate CreateLambda(TFunc&& func, TBindArgs... bindArgs)
    {
        using StorageType = LambdaDelegateStorage<TFunc, TBindArgs...>;
        return SimpleHeapDelegate(new StorageType(std::forward<TFunc>(func), std::move(bindArgs)...), sizeof(StorageType));
    }

    operator bool() const
    {
        return m_storage;
    }

    TReturn operator()(TArgs... args) const
    {
        return m_storage->operator()(std::forward<TArgs>(args)...);
    }

    std::size_t GetHeapSize() const
    {
        return m_storageSize;
    }

private:
    SimpleHeapDelegate(StorageBase* storage, std::size_t storageSize)
        : m_storage(storage)
        , m_storageSize(storageSize)
    {
    }

    std::unique_ptr<StorageBase> m_storage;
    std::size_t m_storageSize;
};
} // namespace sdaineka