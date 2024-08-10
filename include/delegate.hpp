#pragma once
#include "delegate_common.hpp"

#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

namespace sdaineka
{
namespace detail
{
template<std::size_t StackSize>
struct DelegateStackStorage
{
    DelegateStackStorage() = default;

    std::byte data[StackSize];
};

struct DelegateHeapStorage
{
    DelegateHeapStorage(std::size_t size)
        : size(size)
        , data(std::make_unique<std::byte[]>(size))
    {
    }

    std::unique_ptr<std::byte[]> data;
    std::size_t size;
};
} // namespace detail

template<typename>
struct DelegateStorageStackSize
{
    static constexpr std::size_t size()
    {
        return 24;
    }
};

template<typename>
class Delegate;

template<typename TReturn, typename... TArgs>
class Delegate<TReturn(TArgs...)>
{
public:
    template<typename... TBindArgs>
    using GlobalFuncPtr = TReturn (*)(TArgs..., delegate_bind_arg_t<TBindArgs>...);
    template<typename TClass, typename... TBindArgs>
    using MemberFuncPtr = TReturn (TClass::*)(TArgs..., delegate_bind_arg_t<TBindArgs>...);
    template<typename TClass, typename... TBindArgs>
    using MemberFuncPtrConst = TReturn (TClass::*)(TArgs..., delegate_bind_arg_t<TBindArgs>...) const;

public:
    Delegate() = default;

    Delegate(const Delegate&) = delete;
    Delegate& operator=(const Delegate&) = delete;
    Delegate(Delegate&&) noexcept = default;
    Delegate& operator=(Delegate&&) noexcept = default;

    template<typename... TBindArgs>
    static Delegate CreateGlobal(GlobalFuncPtr<TBindArgs...> func, TBindArgs... bindArgs)
    {
        return Delegate(GlobalFuncTag{}, func, std::move(bindArgs)...);
    }

    template<typename TClass, typename... TBindArgs>
    static Delegate CreateMember(TClass* cls, MemberFuncPtr<TClass, TBindArgs...> func, TBindArgs... bindArgs)
    {
        return Delegate(MemberFuncTag{}, cls, func, std::move(bindArgs)...);
    }

    template<typename TClass, typename... TBindArgs>
    static Delegate CreateMember(const TClass* cls, MemberFuncPtrConst<TClass, TBindArgs...> func, TBindArgs... bindArgs)
    {
        return Delegate(MemberFuncTag{}, cls, func, std::move(bindArgs)...);
    }

    template<typename TFunc, typename... TBindArgs>
    static Delegate CreateLambda(TFunc&& func, TBindArgs... bindArgs)
    {
        return Delegate(LambdaFuncTag{}, std::forward<TFunc>(func), std::move(bindArgs)...);
    }

    TReturn operator()(TArgs... args) const
    {
        return m_invoker(GetData(), std::forward<TArgs>(args)...);
    }

    operator bool() const
    {
        return m_invoker != nullptr;
    }

    std::size_t GetHeapSize() const
    {
        if (const auto* heapData = std::get_if<HeapStorage>(&m_storage))
        {
            return heapData->size;
        }

        return 0;
    }

    static constexpr std::size_t GetStorageStackSize()
    {
        return DelegateStorageStackSize<TReturn(TArgs...)>::size();
    }

private:
    struct GlobalFuncTag
    {};
    struct MemberFuncTag
    {};
    struct LambdaFuncTag
    {};

    template<typename TSavedArgsTuple>
    void Construct(TSavedArgsTuple&& v)
    {
        if constexpr (sizeof(TSavedArgsTuple) > GetStorageStackSize())
        {
            m_storage.template emplace<HeapStorage>(sizeof(TSavedArgsTuple));
        }

        new (GetData()) TSavedArgsTuple(std::forward<TSavedArgsTuple>(v));
    }

    template<typename... TBindArgs>
    Delegate(GlobalFuncTag, GlobalFuncPtr<TBindArgs...> func, TBindArgs... bindArgs)
        : m_invoker(Invoke<1, sizeof...(TBindArgs), std::tuple<GlobalFuncPtr<TBindArgs...>, TBindArgs...>>)
    {
        Construct(std::make_tuple(func, std::move(bindArgs)...));
    }

    template<typename TClass, typename... TBindArgs>
    Delegate(MemberFuncTag, TClass* cls, MemberFuncPtr<TClass, TBindArgs...> func, TBindArgs... bindArgs)
        : m_invoker(Invoke<2, sizeof...(TBindArgs), std::tuple<MemberFuncPtr<TClass, TBindArgs...>, TClass*, TBindArgs...>>)
    {
        Construct(std::make_tuple(func, cls, std::move(bindArgs)...));
    }

    template<typename TClass, typename... TBindArgs>
    Delegate(MemberFuncTag, const TClass* cls, MemberFuncPtrConst<TClass, TBindArgs...> func, TBindArgs... bindArgs)
        : m_invoker(Invoke<2, sizeof...(TBindArgs), std::tuple<MemberFuncPtrConst<TClass, TBindArgs...>, const TClass*, TBindArgs...>>)
    {
        Construct(std::make_tuple(func, cls, std::move(bindArgs)...));
    }

    template<typename TFunc, typename... TBindArgs>
    Delegate(LambdaFuncTag, TFunc&& func, TBindArgs... bindArgs)
        : m_invoker(Invoke<1, sizeof...(TBindArgs), std::tuple<TFunc, TBindArgs...>>)
    {
        Construct(std::make_tuple(std::forward<TFunc>(func), std::move(bindArgs)...));
    }

private:
    using StackStorage = detail::DelegateStackStorage<GetStorageStackSize()>;
    using HeapStorage = detail::DelegateHeapStorage;

    using InvokeFunc = TReturn (*)(std::byte* /*data*/, TArgs... /*args*/);
    InvokeFunc m_invoker = nullptr;

    template<std::size_t FuncArgsSize, std::size_t BindArgsSize, typename TSavedArgsTuple>
    static TReturn Invoke(std::byte* data, TArgs... args)
    {
        auto& savedArgsTuple = *reinterpret_cast<TSavedArgsTuple*>(data);
        return InvokeInternal(savedArgsTuple, std::make_index_sequence<FuncArgsSize>(),
                              detail::make_index_sequence<FuncArgsSize, BindArgsSize>(), std::forward<TArgs>(args)...);
    }

    template<typename TSavedArgsTuple, std::size_t... FuncIs, std::size_t... BindIs>
    static TReturn InvokeInternal(TSavedArgsTuple& savedArgsTuple, std::index_sequence<FuncIs...>, std::index_sequence<BindIs...>,
                                  TArgs... args)
    {
        return std::invoke(std::get<FuncIs>(savedArgsTuple)..., std::forward<TArgs>(args)..., std::get<BindIs>(savedArgsTuple)...);
    }

    std::byte* GetData(std::size_t offset = 0) const
    {
        if (auto* stack = std::get_if<StackStorage>(&m_storage))
        {
            return const_cast<std::byte*>(&stack->data[offset]);
        }

        return std::get<HeapStorage>(m_storage).data.get() + offset;
    }

    std::variant<StackStorage, HeapStorage> m_storage;
};
} // namespace sdaineka