#pragma once

#include "xoshiro.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <istream>
#include <ostream>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


#if __cplusplus <= 201402L
namespace GOOM
{
namespace UTILS
{
namespace RAND
{
#else
namespace GOOM::UTILS::RAND
{
#endif

[[nodiscard]] auto GetRandSeed() -> uint64_t;
void SetRandSeed(uint64_t seed);
extern const uint32_t g_randMax;
[[nodiscard]] auto GetXoshiroEng() -> xoshiro256plus64;

void SaveRandState(std::ostream& f);
void RestoreRandState(std::istream& f);

// Return random sign integer, either -1 or +1.
[[nodiscard]] inline auto GetRandSignInt() -> int;
// Return random sign float, either -1.0 or +1.0.
[[nodiscard]] inline auto GetRandSignFlt() -> float;

// Return random positive integer in the range n0 <= n < n1.
[[nodiscard]] auto GetRandInRange(uint32_t n0, uint32_t n1) -> uint32_t;
// Return random integer in the range 0 <= n < n1.
[[nodiscard]] auto GetNRand(uint32_t n1) -> uint32_t;
// Return random integer in the range 0 <= n < randMax.
[[nodiscard]] auto GetRand() -> uint32_t;
// Return random integer in the range n0 <= n < n1.
[[nodiscard]] auto GetRandInRange(int32_t n0, int32_t n1) -> int32_t;
// Return random float in the range x0 <= n <= x1.
[[nodiscard]] auto GetRandInRange(float x0, float x1) -> float;
template<typename T>
struct NumberRange
{
  T min;
  T max;
};
template<typename T>
[[nodiscard]] auto GetRandInRange(const NumberRange<T>& numberRange) -> T;

template<class RandomIt>
void Shuffle(RandomIt first, RandomIt last);

// Return prob(m/n)
[[nodiscard]] inline auto ProbabilityOfMInN(uint32_t m, uint32_t n) -> bool;
[[nodiscard]] inline auto ProbabilityOf(float p) -> bool;

inline auto GetRandSignInt() -> int
{
  return GetRandInRange(0U, 100U) < 50 ? -1 : +1;
}

inline auto GetRandSignFlt() -> float
{
  return GetRandInRange(0U, 100U) < 50 ? -1.0F : +1.0F;
}

inline auto GetNRand(const uint32_t n1) -> uint32_t
{
  return GetRandInRange(0U, n1);
}

inline auto GetRand() -> uint32_t
{
  return GetRandInRange(0U, g_randMax);
}

template<typename T>
inline auto GetRandInRange(const NumberRange<T>& numberRange) -> T
{
  if (std::is_integral<T>())
  {
    return GetRandInRange(numberRange.min, numberRange.max + 1);
  }
  return GetRandInRange(numberRange.min, numberRange.max);
}

template<class RandomIt>
inline void Shuffle(RandomIt first, RandomIt last)
{
  return std::shuffle(first, last, GetXoshiroEng());
}

inline auto ProbabilityOfMInN(const uint32_t m, const uint32_t n) -> bool
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

inline auto ProbabilityOf(const float p) -> bool
{
  return GetRandInRange(0.0F, 1.0F) <= p;
}

#if __cplusplus <= 201402L
} // namespace RAND
} // namespace UTILS
} // namespace GOOM
#else
} // namespace GOOM::UTILS::RAND
#endif