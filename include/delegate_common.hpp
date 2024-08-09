#pragma once
#include <type_traits>
#include <utility>

namespace sdaineka
{
namespace detail
{
template<std::size_t Offset, std::size_t... Is>
constexpr std::index_sequence<(Offset + Is)...> add_offset(std::index_sequence<Is...>)
{
    return {};
}

template<std::size_t Offset, std::size_t N>
constexpr auto make_index_sequence()
{
    return add_offset<Offset>(std::make_index_sequence<N>());
}
} // namespace detail

template<typename T>
struct DelegateBindArg
{
    using type = std::conditional_t<(sizeof(T) <= sizeof(void*)), T, const T&>;
};

template<typename T>
using delegate_bind_arg_t = typename DelegateBindArg<T>::type;
} // namespace sdaineka
