module;

#include <algorithm>
#include <array>
#include <cstddef>
#include <string>

export module Goom.Lib.GoomPaths;

// TODO(glk): fix this
#ifdef __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctad-maybe-unsupported"
#endif

// NOLINTBEGIN(cppcoreguidelines-explicit-constructor-and-conversion,
//             google-explicit-constructor)
template<size_t N>
struct CompileTimeString
{
  consteval CompileTimeString(const char (&arr)[N]) noexcept
  {
    std::copy_n(arr, N, buffer.begin());
  }

  template<size_t LhsSize, size_t RhsSize>
    requires(((LhsSize + RhsSize) - 1) == N)
  consteval CompileTimeString(const CompileTimeString<LhsSize>& lhs,
                              const CompileTimeString<RhsSize>& rhs) noexcept
  {
    auto concatStr = lhs.to_string();
    concatStr.append(rhs.to_string());
    std::copy_n(concatStr.c_str(), N, buffer.begin());
  }

  [[nodiscard]] constexpr auto to_string() const noexcept -> std::string
  {
    return std::string{buffer.data()};
  }
  [[nodiscard]] constexpr auto c_str() const noexcept -> const char* { return buffer.data(); }
  [[nodiscard]] constexpr operator std::string() const noexcept
  {
    return std::string{buffer.data()};
  }
  [[nodiscard]] consteval operator const char*() const noexcept { return buffer.data(); }

  std::array<char, N> buffer{};
  size_t size = N;
};
// NOLINTEND(cppcoreguidelines-explicit-constructor-and-conversion,
//           google-explicit-constructor)

template<size_t LhsSize, size_t RhsSize>
CompileTimeString(CompileTimeString<LhsSize>, CompileTimeString<RhsSize>)
    -> CompileTimeString<(LhsSize + RhsSize) - 1>;

template<CompileTimeString Lhs, CompileTimeString Rhs>
consteval auto static_concat() noexcept -> CompileTimeString<(Lhs.size + Rhs.size) - 1>
{
  return CompileTimeString<Lhs.size + Rhs.size - 1>{Lhs, Rhs};
}

template<CompileTimeString Lhs, CompileTimeString Rhs, CompileTimeString... Others>
  requires(sizeof...(Others) != 0)
consteval auto static_concat() noexcept -> decltype(auto)
{
  return static_concat<CompileTimeString{Lhs, Rhs}, Others...>();
}

export namespace GOOM
{
template<CompileTimeString Cts>
consteval auto operator""_cts()
{
  return Cts;
}

template<size_t N>
auto operator+(const std::string& str1, const CompileTimeString<N>& str2)
{
  return str1 + str2.to_string();
}

[[nodiscard]] consteval auto GetPathSep() noexcept -> decltype(auto)
{
#ifdef _WIN32PC
  return "\\"_cts;
#else
  return "/"_cts;
#endif
}

template<CompileTimeString Base>
[[nodiscard]] consteval auto join_paths() noexcept -> decltype(auto)
{
  return Base;
}

template<CompileTimeString Base, CompileTimeString... Others>
  requires(sizeof...(Others) != 0)
[[nodiscard]] consteval auto join_paths() noexcept -> decltype(auto)
{
  return static_concat<static_concat<Base, GetPathSep()>(), join_paths<Others...>()>();
}

[[nodiscard]] constexpr auto join_paths(const std::string& base) noexcept -> std::string
{
  return base;
}

template<typename... Types>
  requires(sizeof...(Types) != 0)
[[nodiscard]] constexpr auto join_paths(const std::string& base, Types... paths) noexcept
    -> std::string
{
  return base + GetPathSep() + join_paths(paths...);
}

} // namespace GOOM

#ifdef __clang__
#pragma GCC diagnostic pop
#endif
