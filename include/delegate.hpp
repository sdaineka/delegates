#pragma once
#include <memory>
#include <type_traits>
#include <utility>

namespace sdaineka
{
namespace v1
{
template<typename>
class Delegate;

template<typename TReturn, typename... TArgs>
class Delegate<TReturn(TArgs...)>
{
public:
    template<typename... TBindArgs>
    using GlobalFuncPtr = TReturn (*)(TArgs..., TBindArgs...);
    template<typename TClass, typename... TBindArgs>
    using MemberFuncPtr = TReturn (TClass::*)(TArgs..., TBindArgs...);
    template<typename TClass, typename... TBindArgs>
    using MemberFuncPtrConst = TReturn (TClass::*)(TArgs..., TBindArgs...) const;

private:
    class StorageBase
    {
    public:
        virtual ~StorageBase()
        {
        }
        virtual TReturn operator()(TArgs&&... args) const = 0;
    };

    template<typename... TBindArgs>
    class GlobalFuncStorage : public StorageBase
    {
    public:
        GlobalFuncStorage(const GlobalFuncPtr<TBindArgs...> func, TBindArgs&&... bindArgs)
            : m_func(func)
            , m_bindArgs(std::forward<TBindArgs>(bindArgs)...)
        {
        }

        TReturn operator()(TArgs&&... args) const override
        {
            return Invoke(std::forward<TArgs>(args)..., std::make_index_sequence<sizeof...(TBindArgs)>{});
        }

    private:
        template<std::size_t... I>
        TReturn Invoke(TArgs&&... args, std::index_sequence<I...>) const
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
        MemberDelegateStorage(TClass* cls, const MemberFuncPtr<TClass, TBindArgs...> func, TBindArgs&&... bindArgs)
            : m_cls(cls)
            , m_func(func)
            , m_bindArgs(std::forward<TBindArgs>(bindArgs)...)
        {
        }

        TReturn operator()(TArgs&&... args) const override
        {
            return Invoke(std::forward<TArgs>(args)..., std::make_index_sequence<sizeof...(TBindArgs)>{});
        }

    private:
        template<std::size_t... I>
        TReturn Invoke(TArgs&&... args, std::index_sequence<I...>) const
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
        MemberDelegateStorageConst(const TClass* cls, const MemberFuncPtrConst<TClass, TBindArgs...> func, TBindArgs&&... bindArgs)
            : m_cls(cls)
            , m_func(func)
            , m_bindArgs(std::forward<TBindArgs>(bindArgs)...)
        {
        }

        TReturn operator()(TArgs&&... args) const override
        {
            return Invoke(std::forward<TArgs>(args)..., std::make_index_sequence<sizeof...(TBindArgs)>{});
        }

    private:
        template<std::size_t... I>
        TReturn Invoke(TArgs&&... args, std::index_sequence<I...>) const
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
        LambdaDelegateStorage(TFunc&& func, TBindArgs&&... bindArgs)
            : m_func(std::forward<TFunc>(func))
            , m_bindArgs(std::forward<TBindArgs>(bindArgs)...)
        {
        }

        TReturn operator()(TArgs&&... args) const override
        {
            return Invoke(std::forward<TArgs>(args)..., std::make_index_sequence<sizeof...(TBindArgs)>{});
        }

    private:
        template<std::size_t... I>
        TReturn Invoke(TArgs&&... args, std::index_sequence<I...>) const
        {
            return std::invoke(m_func, std::forward<TArgs>(args)..., std::get<I>(m_bindArgs)...);
        }

        TFunc m_func;
        std::tuple<TBindArgs...> m_bindArgs;
    };

public:
    Delegate(const Delegate&) = delete;
    Delegate& operator=(const Delegate&) = delete;
    Delegate(Delegate&&) noexcept = default;
    Delegate& operator=(Delegate&&) noexcept = default;

    template<typename... TBindArgs>
    static Delegate CreateGlobal(GlobalFuncPtr<TBindArgs...> func, TBindArgs&&... bindArgs)
    {
        return Delegate(new GlobalFuncStorage<TBindArgs...>(func, std::forward<TBindArgs>(bindArgs)...));
    }

    template<typename TClass, typename... TBindArgs>
    static Delegate CreateMember(TClass* cls, MemberFuncPtr<TClass, TBindArgs...> func, TBindArgs&&... bindArgs)
    {
        return Delegate(new MemberDelegateStorage<TClass, TBindArgs...>(cls, func, std::forward<TBindArgs>(bindArgs)...));
    }

    template<typename TClass, typename... TBindArgs>
    static Delegate CreateMember(const TClass* cls, MemberFuncPtrConst<TClass, TBindArgs...> func, TBindArgs&&... bindArgs)
    {
        return Delegate(new MemberDelegateStorageConst<TClass, TBindArgs...>(cls, func, std::forward<TBindArgs>(bindArgs)...));
    }

    template<typename TFunc, typename... TBindArgs>
    static Delegate CreateLambda(TFunc&& func, TBindArgs&&... bindArgs)
    {
        return Delegate(new LambdaDelegateStorage<TFunc, TBindArgs...>(std::forward<TFunc>(func), std::forward<TBindArgs>(bindArgs)...));
    }

    TReturn operator()(TArgs&&... args) const
    {
        return m_storage->operator()(std::forward<TArgs>(args)...);
    }

private:
    Delegate(StorageBase* storage)
        : m_storage(storage) {};

    std::unique_ptr<StorageBase> m_storage;
};
} // namespace v1

namespace v2
{
template<typename TReturn, typename... TArgs>
struct DelegateStackSize
{
    static constexpr std::size_t size() { return 24; }
};

template<typename>
class Delegate;

template<typename TReturn, typename... TArgs>
class Delegate<TReturn(TArgs...)>
{
public:
    template<typename... TBindArgs>
    using GlobalFuncPtr = TReturn (*)(TArgs..., TBindArgs...);
    template<typename TClass, typename... TBindArgs>
    using MemberFuncPtr = TReturn (TClass::*)(TArgs..., TBindArgs...);
    template<typename TClass, typename... TBindArgs>
    using MemberFuncPtrConst = TReturn (TClass::*)(TArgs..., TBindArgs...) const;

public:
    Delegate(const Delegate&) = delete;
    Delegate& operator=(const Delegate&) = delete;
    Delegate(Delegate&&) noexcept = default;
    Delegate& operator=(Delegate&&) noexcept = default;

    template<typename... TBindArgs>
    static Delegate CreateGlobal(GlobalFuncPtr<TBindArgs...> func, TBindArgs&&... bindArgs)
    {
        return Delegate(GlobalFuncTag{}, func, std::forward<TBindArgs>(bindArgs)...);
    }

    template<typename TClass, typename... TBindArgs>
    static Delegate CreateMember(TClass* cls, MemberFuncPtr<TClass, TBindArgs...> func, TBindArgs&&... bindArgs)
    {
        return Delegate(MemberFuncTag{}, cls, func, std::forward<TBindArgs>(bindArgs)...);
    }

    template<typename TClass, typename... TBindArgs>
    static Delegate CreateMember(const TClass* cls, MemberFuncPtrConst<TClass, TBindArgs...> func, TBindArgs&&... bindArgs)
    {
        return Delegate(MemberFuncTag{}, cls, func, std::forward<TBindArgs>(bindArgs)...);
    }

    template<typename TFunc, typename... TBindArgs>
    static Delegate CreateLambda(TFunc&& func, TBindArgs&&... bindArgs)
    {
        return Delegate(LambdaFuncTag{}, std::forward<TFunc>(func), std::forward<TBindArgs>(bindArgs)...);
    }

    TReturn operator()(TArgs&&... args) const
    {
        return m_invoker(GetData(), std::forward<TArgs>(args)...);
    }

    std::size_t GetHeapSize() const
    {
        return m_heapSize;
    }

    static constexpr std::size_t GetStackSize() { return DelegateStackSize<TReturn, TArgs...>::size(); }

private:
    struct GlobalFuncTag
    {
    };

    struct MemberFuncTag
    {
    };

    struct LambdaFuncTag
    {
    };

    template<typename... TBindArgs>
    Delegate(GlobalFuncTag, GlobalFuncPtr<TBindArgs...> func, TBindArgs&&... bindArgs)
        : m_invoker(InvokeGlobalFunc<TBindArgs...>)
    {
        // Small Buffer Optimization
        static constexpr std::size_t kSize = sizeof(GlobalFuncPtr<TBindArgs...>) + sizeof(std::tuple<TBindArgs...>);
        if constexpr (kSize > GetStackSize())
        {
            m_heapSize = kSize;
            m_heapData = std::make_unique<std::byte[]>(m_heapSize);
        }

        new (GetData()) GlobalFuncPtr<TBindArgs...>(func);
        new (GetData(sizeof(GlobalFuncPtr<TBindArgs...>))) std::tuple<TBindArgs...>(std::forward<TBindArgs>(bindArgs)...);
    }

    template<typename TClass, typename... TBindArgs>
    Delegate(MemberFuncTag, TClass* cls, MemberFuncPtr<TClass, TBindArgs...> func, TBindArgs&&... bindArgs)
        : m_invoker(InvokeMemberFunc<TClass, TBindArgs...>)
    {
        // Small Buffer Optimization
        static constexpr std::size_t kSize =
            sizeof(TClass*) + sizeof(MemberFuncPtr<TClass, TBindArgs...>) + sizeof(std::tuple<TBindArgs...>);
        if constexpr (kSize > GetStackSize())
        {
            m_heapSize = kSize;
            m_heapData = std::make_unique<std::byte[]>(m_heapSize);
        }

        new (GetData()) TClass*(cls);
        new (GetData(sizeof(TClass*))) MemberFuncPtr<TClass, TBindArgs...>(func);
        new (GetData(sizeof(TClass*) + sizeof(MemberFuncPtr<TClass, TBindArgs...>)))
            std::tuple<TBindArgs...>(std::forward<TBindArgs>(bindArgs)...);
    }

    template<typename TClass, typename... TBindArgs>
    Delegate(MemberFuncTag, const TClass* cls, MemberFuncPtrConst<TClass, TBindArgs...> func, TBindArgs&&... bindArgs)
        : m_invoker(InvokeMemberFuncConst<TClass, TBindArgs...>)
    {
        // Small Buffer Optimization
        static constexpr std::size_t kSize =
            sizeof(TClass*) + sizeof(MemberFuncPtrConst<TClass, TBindArgs...>) + sizeof(std::tuple<TBindArgs...>);
        if constexpr (kSize > GetStackSize())
        {
            m_heapSize = kSize;
            m_heapData = std::make_unique<std::byte[]>(m_heapSize);
        }

        new (GetData()) const TClass*(cls);
        new (GetData(sizeof(TClass*))) MemberFuncPtrConst<TClass, TBindArgs...>(func);
        new (GetData(sizeof(TClass*) + sizeof(MemberFuncPtrConst<TClass, TBindArgs...>)))
            std::tuple<TBindArgs...>(std::forward<TBindArgs>(bindArgs)...);
    }

    template<typename TFunc, typename... TBindArgs>
    Delegate(LambdaFuncTag, TFunc&& func, TBindArgs&&... bindArgs)
        : m_invoker(InvokeLambda<std::decay_t<TFunc>, TBindArgs...>)
    {
        // Small Buffer Optimization
        static constexpr std::size_t kSize = sizeof(TFunc) + sizeof(std::tuple<TBindArgs...>);
        if constexpr (kSize > GetStackSize())
        {
            m_heapSize = kSize;
            m_heapData = std::make_unique<std::byte[]>(m_heapSize);
        }

        new (GetData()) std::decay_t<TFunc>(std::forward<TFunc>(func));
        new (GetData(sizeof(TFunc))) std::tuple<TBindArgs...>(std::forward<TBindArgs>(bindArgs)...);
    }

private:
    using InvokeFunc = TReturn (*)(std::byte* /*data*/, TArgs&&... /*args*/);
    InvokeFunc m_invoker;

    template<typename... TBindArgs>
    static TReturn InvokeGlobalFunc(std::byte* data, TArgs&&... args)
    {
        auto& func = *reinterpret_cast<GlobalFuncPtr<TBindArgs...>*>(data);
        std::tuple<TBindArgs...>& bindArgs = *reinterpret_cast<std::tuple<TBindArgs...>*>(data + sizeof(GlobalFuncPtr<TBindArgs...>));
        return InvokeGlobalFuncInternal(func, std::forward<TArgs>(args)..., bindArgs, std::make_index_sequence<sizeof...(TBindArgs)>{});
    }

    template<typename... TBindArgs, std::size_t... I>
    static TReturn InvokeGlobalFuncInternal(GlobalFuncPtr<TBindArgs...>& func, TArgs&&... args, std::tuple<TBindArgs...>& bindArgs,
                                            std::index_sequence<I...>)
    {
        return std::invoke(func, std::forward<TArgs>(args)..., std::get<I>(bindArgs)...);
    }

    template<typename TClass, typename... TBindArgs>
    static TReturn InvokeMemberFunc(std::byte* data, TArgs&&... args)
    {
        auto* cls = *reinterpret_cast<TClass**>(data);
        auto& func = *reinterpret_cast<MemberFuncPtr<TClass, TBindArgs...>*>(data + sizeof(TClass*));
        std::tuple<TBindArgs...>& bindArgs =
            *reinterpret_cast<std::tuple<TBindArgs...>*>(data + sizeof(TClass*) + sizeof(MemberFuncPtr<TClass, TBindArgs...>));
        return InvokeMemberFuncInternal(*cls, func, std::forward<TArgs>(args)..., bindArgs,
                                        std::make_index_sequence<sizeof...(TBindArgs)>{});
    }

    template<typename TClass, typename... TBindArgs, std::size_t... I>
    static TReturn InvokeMemberFuncInternal(TClass& cls, MemberFuncPtr<TClass, TBindArgs...>& func, TArgs&&... args,
                                            std::tuple<TBindArgs...>& bindArgs, std::index_sequence<I...>)
    {
        return std::invoke(func, cls, std::forward<TArgs>(args)..., std::get<I>(bindArgs)...);
    }

    template<typename TClass, typename... TBindArgs>
    static TReturn InvokeMemberFuncConst(std::byte* data, TArgs&&... args)
    {
        const auto* cls = *reinterpret_cast<const TClass**>(data);
        auto& func = *reinterpret_cast<MemberFuncPtrConst<TClass, TBindArgs...>*>(data + sizeof(TClass*));
        std::tuple<TBindArgs...>& bindArgs =
            *reinterpret_cast<std::tuple<TBindArgs...>*>(data + sizeof(TClass*) + sizeof(MemberFuncPtrConst<TClass, TBindArgs...>));
        return InvokeMemberFuncConstInternal(*cls, func, std::forward<TArgs>(args)..., bindArgs,
                                             std::make_index_sequence<sizeof...(TBindArgs)>{});
    }

    template<typename TClass, typename... TBindArgs, std::size_t... I>
    static TReturn InvokeMemberFuncConstInternal(const TClass& cls, MemberFuncPtrConst<TClass, TBindArgs...>& func, TArgs&&... args,
                                                 std::tuple<TBindArgs...>& bindArgs, std::index_sequence<I...>)
    {
        return std::invoke(func, cls, std::forward<TArgs>(args)..., std::get<I>(bindArgs)...);
    }

    template<typename TFunc, typename... TBindArgs>
    static TReturn InvokeLambda(std::byte* data, TArgs&&... args)
    {
        TFunc& func = *reinterpret_cast<TFunc*>(data);
        std::tuple<TBindArgs...>& bindArgs = *reinterpret_cast<std::tuple<TBindArgs...>*>(data + sizeof(TFunc));
        return InvokeLambdaInternal(func, std::forward<TArgs>(args)..., bindArgs, std::make_index_sequence<sizeof...(TBindArgs)>{});
    }

    template<typename TFunc, typename... TBindArgs, std::size_t... I>
    static TReturn InvokeLambdaInternal(TFunc& func, TArgs&&... args, std::tuple<TBindArgs...>& bindArgs, std::index_sequence<I...>)
    {
        return std::invoke(func, std::forward<TArgs>(args)..., std::get<I>(bindArgs)...);
    }

    std::byte* GetData(std::size_t offset = 0) const
    {
        return m_heapSize > 0 ? (m_heapData.get() + offset) : const_cast<std::byte*>(&m_stackData[offset]);
    }

    std::byte m_stackData[GetStackSize()];
    std::size_t m_heapSize = 0;
    std::unique_ptr<std::byte[]> m_heapData;
};
} // namespace v2

using v1::Delegate;

} // namespace sdaineka