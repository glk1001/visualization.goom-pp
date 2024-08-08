module;

#include <array>
#include <cstddef>
#include <utility>

export module Goom.Utils.ArrayUtils;

export namespace GOOM::UTILS
{

template<typename T, std::size_t N>
constexpr auto CreateArray(const T& value) -> std::array<T, N>;

template<typename T, std::size_t N>
constexpr auto Contains(const std::array<T, N>& array, const T& value) noexcept -> bool;

} // namespace GOOM::UTILS

namespace GOOM::UTILS
{
namespace DETAIL
{

template<typename T, std::size_t... Is>
constexpr auto CreateArrayImpl(const T value, [[maybe_unused]] std::index_sequence<Is...> args)
    -> std::array<T, sizeof...(Is)>
{
  // Cast 'Is' to void to remove the warning: unused value
  return {{(static_cast<void>(Is), value)...}};
}

} // namespace DETAIL

template<typename T, std::size_t N>
constexpr auto CreateArray(const T& value) -> std::array<T, N>
{
  return DETAIL::CreateArrayImpl(value, std::make_index_sequence<N>());
}

template<typename T, std::size_t N>
constexpr auto Contains(const std::array<T, N>& array, const T& value) noexcept -> bool
{
  for (const auto& element : array)
  {
    if (element == value)
    {
      return true;
    }
  }

  return false;
}

} // namespace GOOM::UTILS
