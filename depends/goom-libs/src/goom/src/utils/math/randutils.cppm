module;

#include "goom/goom_logger.h"
#include "xoshiro.hpp"

#include <cmath>
#include <cstdint>
#include <iosfwd>
#include <istream>
#include <limits>
#include <ostream>
#include <type_traits>

export module Goom.Utils.Math.RandUtils;

import Goom.Lib.AssertUtils;

export namespace GOOM::UTILS::MATH::RAND
{

[[nodiscard]] auto GetRandSeed() noexcept -> uint64_t;
auto SetRandSeed(uint64_t seed) noexcept -> void;

extern const uint32_t G_RAND_MAX;

auto SaveRandState(std::ostream& file) -> void;
auto RestoreRandState(std::istream& file) -> void;

// Return random positive integer in the range n0 <= n < n1.
[[nodiscard]] auto GetRandInRange(uint32_t n0, uint32_t n1) noexcept -> uint32_t;

// Return random integer in the range 0 <= n < n1.
[[nodiscard]] auto GetNRand(uint32_t n1) noexcept -> uint32_t;

// Return random integer in the range 0 <= n < randMax.
[[nodiscard]] auto GetRand() noexcept -> uint32_t;

// Return random integer in the range n0 <= n < n1.
[[nodiscard]] auto GetRandInRange(int32_t n0, int32_t n1) noexcept -> int32_t;

// Return random float in the range x0 <= n <= x1.
[[nodiscard]] auto GetRandInRange(float x0, float x1) noexcept -> float;
[[nodiscard]] auto GetRandInRange(double x0, double x1) noexcept -> double;

template<typename T>
struct NumberRange
{
  T min;
  T max;
};
template<typename T>
[[nodiscard]] auto GetRandInRange(const NumberRange<T>& numberRange) noexcept -> T;

// Return prob(m/n)
[[nodiscard]] inline auto ProbabilityOfMInN(uint32_t m, uint32_t n) noexcept -> bool;
[[nodiscard]] inline auto ProbabilityOf(float prob) noexcept -> bool;

} // namespace GOOM::UTILS::MATH::RAND

namespace GOOM::UTILS::MATH::RAND
{

inline auto GetNRand(const uint32_t n1) noexcept -> uint32_t
{
  return GetRandInRange(0U, n1);
}

inline auto GetRand() noexcept -> uint32_t
{
  return GetRandInRange(0U, G_RAND_MAX);
}

template<typename T>
inline auto GetRandInRange(const NumberRange<T>& numberRange) noexcept -> T
{
  if (std::is_integral<T>())
  {
    return GetRandInRange(numberRange.min, numberRange.max + 1);
  }
  return GetRandInRange(numberRange.min, numberRange.max);
}

inline auto ProbabilityOfMInN(const uint32_t m, const uint32_t n) noexcept -> bool
{
  if (1 == m)
  {
    return 0 == GetNRand(n);
  }
  if (m == (n - 1))
  {
    return GetNRand(n) > 0;
  }
  return GetRandInRange(0.0F, 1.0F) <= (static_cast<float>(m) / static_cast<float>(n));
}

inline auto ProbabilityOf(const float prob) noexcept -> bool
{
  return GetRandInRange(0.0F, 1.0F) <= prob;
}

} // namespace GOOM::UTILS::MATH::RAND

namespace GOOM::UTILS::MATH::RAND
{

// NOTE: C++ std::uniform_int_distribution is too expensive (about double time) so we use
// Xoshiro and multiplication/shift technique. For timings, see tests/test_goomrand.cpp.

namespace
{

constexpr uint32_t G_RAND_MAX_CONST =
    (xoshiro256plus64::max() > std::numeric_limits<uint32_t>::max())
        ? std::numeric_limits<uint32_t>::max()
        : static_cast<uint32_t>(xoshiro256plus64::max());

// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables): Hard to get around!
uint64_t randSeed = 0UL;
thread_local xoshiro256plus64 xoshiroEng{GetRandSeed()}; // NOLINT(cert-err58-cpp)
// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables): Hard to get around!

inline auto RandXoshiroFunc(const uint32_t n0, const uint32_t n1) noexcept -> uint32_t
{
  const auto x     = static_cast<uint32_t>(xoshiroEng());
  const uint64_t m = (static_cast<uint64_t>(x) * static_cast<uint64_t>(n1 - n0)) >> 32U;
  return n0 + static_cast<uint32_t>(m);
}

#ifdef USE_ME
inline auto RandSplitMixFunc(const uint32_t n0, const uint32_t n1) noexcept -> uint32_t
{
  // thread_local SplitMix32 eng { GetRandSeed() };
  thread_local splitmix64 s_eng{GetRandSeed()};
  const auto x     = static_cast<uint32_t>(s_eng());
  const uint64_t m = (static_cast<uint64_t>(x) * static_cast<uint64_t>(n1 - n0)) >> 32U;
  return n0 + static_cast<uint32_t>(m);
}
#endif

} // namespace

const uint32_t G_RAND_MAX = G_RAND_MAX_CONST;

auto GetRandSeed() noexcept -> uint64_t
{
  return randSeed;
}

void SetRandSeed(const uint64_t seed) noexcept
{
  randSeed   = seed;
  xoshiroEng = randSeed;
  LogDebug("SetRandSeed: GetRandSeed = {}", GetRandSeed());
}

void SaveRandState(std::ostream& file)
{
  file << randSeed << "\n";
  file << xoshiroEng << "\n";
}

void RestoreRandState(std::istream& file)
{
  file >> randSeed;
  file >> xoshiroEng;
}

auto GetRandInRange(const uint32_t n0, const uint32_t n1) noexcept -> uint32_t
{
  Expects(n0 < n1);

  return RandXoshiroFunc(n0, n1);
}

auto GetRandInRange(const int32_t n0, const int32_t n1) noexcept -> int32_t
{
  Expects(n0 < n1);

  if ((n0 >= 0) && (n1 >= 0))
  {
    return static_cast<int32_t>(
        GetRandInRange(static_cast<uint32_t>(n0), static_cast<uint32_t>(n1)));
  }
  return n0 + static_cast<int32_t>(GetNRand(static_cast<uint32_t>(-n0 + n1)));
}

auto GetRandInRange(const float x0, const float x1) noexcept -> float
{
  Expects(x0 < x1);

  static const auto s_ENG_MAX = static_cast<float>(G_RAND_MAX);
  const auto t                = static_cast<float>(RandXoshiroFunc(0, G_RAND_MAX)) / s_ENG_MAX;
  return std::lerp(x0, x1, t);
  //  thread_local std::uniform_real_distribution<> dis(0, 1);
  //  return std::lerp(x0, x1, static_cast<float>(dis(eng)));
}

auto GetRandInRange(const double x0, const double x1) noexcept -> double
{
  Expects(x0 < x1);

  static const auto s_ENG_MAX = static_cast<double>(G_RAND_MAX);
  const auto t                = static_cast<double>(RandXoshiroFunc(0, G_RAND_MAX)) / s_ENG_MAX;
  return std::lerp(x0, x1, t);
  //  thread_local std::uniform_real_distribution<> dis(0, 1);
  //  return std::lerp(x0, x1, static_cast<float>(dis(eng)));
}

} // namespace GOOM::UTILS::MATH::RAND
