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

template<typename T>
using func_param_t = std::conditional_t<std::is_pod_v<T> || std::is_pointer_v<T>, T, const T&>;
} // namespace detail
} // namespace sdaineka
