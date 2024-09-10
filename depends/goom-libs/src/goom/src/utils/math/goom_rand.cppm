module;

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <map>
#include <numeric>
#include <type_traits>
#include <utility>
#include <vector>

export module Goom.Utils.Math.GoomRand;

import Goom.Utils.EnumUtils;
import Goom.Utils.Math.Misc;
import Goom.Utils.Math.Rand.RandUtils;
import Goom.Lib.AssertUtils;

export namespace GOOM::UTILS::MATH
{

inline constexpr auto GOOM_RAND_MAX = RAND::GOOM_RAND_MAX;

[[nodiscard]] auto GetRandSeed() noexcept -> uint64_t;
auto SetRandSeed(uint64_t seed) noexcept -> void;

// NOLINTBEGIN(misc-non-private-member-variables-in-classes): Need class as non-type template param.
template<typename T>
class NumberRange
{
public:
  constexpr NumberRange() = default;
  constexpr NumberRange(T minT, T maxT) noexcept;

  T min;
  T max;
  T range;
  T rangePlus1;
};
// NOLINTEND(misc-non-private-member-variables-in-classes)

template<typename T>
NumberRange(T, T) -> NumberRange<T>;

template<typename T>
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
constexpr NumberRange<T>::NumberRange(const T minT, const T maxT) noexcept
  : min{minT}, max{maxT}, range{maxT - minT}, rangePlus1{range + 1}
{
  Expects(minT <= maxT);
}

inline constexpr auto UNIT_RANGE        = NumberRange{0.0F, 1.0F};
inline constexpr auto FULL_CIRCLE_RANGE = NumberRange{0.0F, TWO_PI};
inline constexpr auto HALF_CIRCLE_RANGE = NumberRange{0.0F, PI};

template<typename T>
[[nodiscard]] constexpr auto GetMidpoint(const NumberRange<T>& numberRange) noexcept;

class GoomRand
{
public:
  // Return a random number in the range [numberRange.min, numberRange.max].
  template<NumberRange numberRange> // NOLINT(readability-identifier-naming)
  [[nodiscard]] auto GetRandInRange() const noexcept -> decltype(numberRange.min);
  template<typename T>
  [[nodiscard]] auto GetRandInRange(const NumberRange<T>& numberRange) const noexcept -> T;

  template<std::ranges::random_access_range Range>
  auto Shuffle(Range& range) const noexcept -> void;

  template<float x> // NOLINT: readability-identifier-naming
  [[nodiscard]] auto ProbabilityOf() const noexcept -> bool;
  [[nodiscard]] auto ProbabilityOf(float x) const noexcept -> bool;

  // Return a random integer in the range [0, n).
  [[nodiscard]] auto GetNRand(uint32_t n) const noexcept -> uint32_t;

private:
  // Return a random integer number in the range [n0, n0 + nRangePlus1).
  [[nodiscard]] static auto GetRandInRange(uint32_t n0, uint32_t nRangePlus1) noexcept -> uint32_t;
  [[nodiscard]] static auto GetRandInRange(int32_t n0, int32_t nRangePlus1) noexcept -> int32_t;

  // Return a random real number in the range [x0, x0+xRange].
  [[nodiscard]] static auto GetRandInRange(float x0, float xRange) noexcept -> float;
  [[nodiscard]] static auto GetRandInRange(double x0, double xRange) noexcept -> double;
};

template<EnumType E>
class Weights
{
public:
  struct KeyValue
  {
    E key{};
    float weight{};
  };
  using EventWeightPairs = std::vector<KeyValue>;

  Weights() noexcept = default;
  Weights(const GoomRand& goomRand, const EventWeightPairs& weights) noexcept;

  auto SetWeightToZero(const E& enumVal) noexcept -> void;

  [[nodiscard]] auto GetWeightArray() const noexcept -> const std::array<float, NUM<E>>&;
  [[nodiscard]] auto GetEventWeightPairs() const noexcept -> EventWeightPairs;

  [[nodiscard]] auto GetNumElements() const noexcept -> size_t;
  [[nodiscard]] auto GetNumSetWeights() const noexcept -> size_t;
  [[nodiscard]] auto GetWeight(const E& enumVal) const noexcept -> float;
  [[nodiscard]] auto GetSumOfWeights() const noexcept -> float;
  [[nodiscard]] auto GetSumOfWeightsUpTo(const E& lastVal) const noexcept -> float;

  [[nodiscard]] auto GetRandomWeighted() const noexcept -> E;
  [[nodiscard]] auto GetRandomWeighted(const E& given) const noexcept -> E;
  [[nodiscard]] auto GetRandomWeightedUpTo(const E& upToThisEvent) const noexcept -> E;

private:
  const GoomRand* m_goomRand = nullptr;
  using WeightArray          = std::array<float, NUM<E>>;
  struct WeightData
  {
    // Array indexes correspond to E values.
    WeightArray weightArray{0.0F};
    WeightArray progressiveWeightSumArray{0.0F};
    size_t numSetWeights = 0;
  };
  WeightData m_weightData{};

  [[nodiscard]] auto GetRandomWeighted(float sumOfWeightsUpTo) const noexcept -> E;
  [[nodiscard]] static auto GetWeightData(const EventWeightPairs& eventWeightPairs) noexcept
      -> WeightData;
};

template<typename E>
auto GetWeightedSample(const Weights<E>& weights, uint32_t n, std::vector<E>& sample) noexcept
    -> void;

template<EnumType E>
class ConditionalWeights
{
public:
  struct KeyValue
  {
    E key{};
    std::map<E, float> weightMultipliers{};
  };
  using EventWeightMultiplierPairs = std::vector<KeyValue>;

  ConditionalWeights(const GoomRand& goomRand,
                     const typename Weights<E>::EventWeightPairs& eventWeightPairs,
                     bool disallowEventsSameAsGiven = true) noexcept;
  ConditionalWeights(const GoomRand& goomRand,
                     const typename Weights<E>::EventWeightPairs& weights,
                     const EventWeightMultiplierPairs& weightMultiplierPairs,
                     bool disallowEventsSameAsGiven = true) noexcept;

  struct ConditionalEvents
  {
    E previousEvent;
    E event;
  };
  [[nodiscard]] auto GetNumSetWeights() const noexcept -> size_t;
  [[nodiscard]] auto GetWeight(const ConditionalEvents& events) const noexcept -> float;
  [[nodiscard]] auto GetSumOfWeights(const E& given) const noexcept -> float;

  [[nodiscard]] auto GetRandomWeighted(const E& given) const noexcept -> E;

private:
  Weights<E> m_unconditionalWeights;
  std::map<E, Weights<E>> m_conditionalWeights;
  bool m_disallowEventsSameAsGiven;
  [[nodiscard]] static auto GetConditionalWeightMap(
      const GoomRand& goomRand,
      const typename Weights<E>::EventWeightPairs& eventWeightPairs,
      const EventWeightMultiplierPairs& weightMultiplierPairs) noexcept -> std::map<E, Weights<E>>;
};

} // namespace GOOM::UTILS::MATH

namespace GOOM::UTILS::MATH
{

inline auto GetRandSeed() noexcept -> uint64_t
{
  return RAND::GetRandSeed();
}

inline auto SetRandSeed(const uint64_t seed) noexcept -> void
{
  RAND::SetRandSeed(seed);
}

template<typename T>
constexpr auto GetMidpoint(const NumberRange<T>& numberRange) noexcept
{
  return std::midpoint(numberRange.min, numberRange.max);
}

// NOLINTBEGIN(readability-convert-member-functions-to-static)
template<std::ranges::random_access_range Range>
auto GoomRand::Shuffle(Range& range) const noexcept -> void
{
  auto first = std::ranges::begin(range);
  auto last  = std::ranges::end(range);

  for (auto i = last - first - 1; i > 0; --i)
  {
    using std::swap;
    swap(first[i], first[GetRandInRange(0U, static_cast<uint32_t>(i) + 1)]);
  }
}

template<typename E>
auto GetWeightedSample(const Weights<E>& weights, uint32_t n, std::vector<E>& sample) noexcept
    -> void
{
  Expects(n <= weights.GetNumSetWeights());

  sample.resize(n);
  auto newWeights = weights;
  for (auto i = 0U; i < n; ++i)
  {
    const auto nextEnum = newWeights.GetRandomWeighted();
    sample.at(i)        = nextEnum;
    newWeights.SetWeightToZero(nextEnum);
  }
}

template<float x> // NOLINT(readability-identifier-naming)
auto GoomRand::ProbabilityOf() const noexcept -> bool
{
  static_assert(0.0F <= x);
  static_assert(x <= 1.0F);
  return GetRandInRange<UNIT_RANGE>() <= x;
}

auto GoomRand::ProbabilityOf(const float x) const noexcept -> bool
{
  return GetRandInRange<UNIT_RANGE>() <= x;
}

template<NumberRange numberRange> // NOLINT(readability-identifier-naming)
[[nodiscard]] auto GoomRand::GetRandInRange() const noexcept -> decltype(numberRange.min)
{
  if constexpr (not std::is_integral<decltype(numberRange.min)>())
  {
    // Get a random floating point number.
    if constexpr (numberRange.range == static_cast<decltype(numberRange.min)>(0.0F))
    {
      return numberRange.min;
    }
    else
    {
      return GetRandInRange(numberRange.min, numberRange.range);
    }
  }
  else
  {
    // Get random integer number.
    if constexpr (0 == numberRange.range)
    {
      return numberRange.min;
    }
    else
    {
      return GetRandInRange(numberRange.min, numberRange.rangePlus1);
    }
  }
}

template<typename T>
auto GoomRand::GetRandInRange(const NumberRange<T>& numberRange) const noexcept -> T
{
  // NOLINTBEGIN(clang-analyzer-core.CallAndMessage): False positive.
  if constexpr (not std::is_integral<T>())
  {
    // Get random floating point number.
    return GetRandInRange(numberRange.min, numberRange.range);
  }
  else
  {
    // Get random integer number.
    return GetRandInRange(numberRange.min, numberRange.rangePlus1);
  }
  // NOLINTEND(clang-analyzer-core.CallAndMessage)
}

inline auto GoomRand::GetNRand(const uint32_t n) const noexcept -> uint32_t
{
  return RAND::GetNRand(n);
}
// NOLINTEND(readability-convert-member-functions-to-static)

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
inline auto GoomRand::GetRandInRange(const uint32_t n0, const uint32_t nRangePlus1) noexcept
    -> uint32_t
{
  return RAND::GetRandInRange(n0, nRangePlus1);
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
inline auto GoomRand::GetRandInRange(const int32_t n0, const int32_t nRangePlus1) noexcept
    -> int32_t
{
  return RAND::GetRandInRange(n0, nRangePlus1);
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
inline auto GoomRand::GetRandInRange(const float x0, const float xRange) noexcept -> float
{
  return RAND::GetRandInRange(x0, xRange);
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
inline auto GoomRand::GetRandInRange(const double x0, const double xRange) noexcept -> double
{
  return RAND::GetRandInRange(x0, xRange);
}

template<EnumType E>
Weights<E>::Weights(const GoomRand& goomRand, const EventWeightPairs& weights) noexcept
  : m_goomRand{&goomRand}, m_weightData{GetWeightData(weights)}
{
  Expects(NUM<E> == m_weightData.weightArray.size());
  Expects(GetSumOfWeights() > SMALL_FLOAT);
}

template<EnumType E>
auto Weights<E>::SetWeightToZero(const E& enumVal) noexcept -> void
{
  const auto index = static_cast<size_t>(enumVal);
  if (m_weightData.weightArray.at(index) < SMALL_FLOAT)
  {
    return;
  }

  Expects(m_weightData.numSetWeights > 0);
  m_weightData.weightArray.at(index) = 0.0;
  --m_weightData.numSetWeights;

  m_weightData.progressiveWeightSumArray.at(0) = m_weightData.weightArray.at(0);
  for (auto i = 1U; i < m_weightData.weightArray.size(); ++i)
  {
    m_weightData.progressiveWeightSumArray.at(i) =
        m_weightData.progressiveWeightSumArray.at(i - 1) + m_weightData.weightArray.at(i);
  }
}

template<EnumType E>
auto Weights<E>::GetWeightData(const EventWeightPairs& eventWeightPairs) noexcept -> WeightData
{
  WeightData weightData{};
  for (const auto& [e, w] : eventWeightPairs)
  {
    weightData.weightArray.at(static_cast<size_t>(e)) = w;
    ++weightData.numSetWeights;
  }

  weightData.progressiveWeightSumArray.at(0) = weightData.weightArray.at(0);
  for (auto i = 1U; i < weightData.weightArray.size(); ++i)
  {
    weightData.progressiveWeightSumArray.at(i) =
        weightData.progressiveWeightSumArray.at(i - 1) + weightData.weightArray.at(i);
  }

  return weightData;
}

template<EnumType E>
auto Weights<E>::GetWeightArray() const noexcept -> const std::array<float, NUM<E>>&
{
  return m_weightData.weightArray;
}

template<EnumType E>
auto Weights<E>::GetEventWeightPairs() const noexcept -> EventWeightPairs
{
  auto eventWeightPairs = EventWeightPairs{};

  for (const auto& [e, w] : m_weightData.weightArray)
  {
    eventWeightPairs.emplace_back(e, w);
  }

  return eventWeightPairs;
}

template<EnumType E>
auto Weights<E>::GetNumElements() const noexcept -> size_t
{
  return m_weightData.weightArray.size();
}

template<EnumType E>
auto Weights<E>::GetNumSetWeights() const noexcept -> size_t
{
  return m_weightData.numSetWeights;
}

template<EnumType E>
auto Weights<E>::GetWeight(const E& enumVal) const noexcept -> float
{
  for (auto i = 0U; i < m_weightData.weightArray.size(); ++i)
  {
    if (static_cast<E>(i) == enumVal)
    {
      return m_weightData.weightArray[i];
    }
  }

  return 0.0F;
}

template<EnumType E>
auto Weights<E>::GetSumOfWeights() const noexcept -> float
{
  return m_weightData.progressiveWeightSumArray.back();
}

template<EnumType E>
auto Weights<E>::GetRandomWeighted() const noexcept -> E
{
  return GetRandomWeighted(GetSumOfWeights());
}

template<EnumType E>
auto Weights<E>::GetRandomWeightedUpTo(const E& upToThisEvent) const noexcept -> E
{
  return GetRandomWeighted(GetSumOfWeightsUpTo(upToThisEvent));
}

template<EnumType E>
auto Weights<E>::GetRandomWeighted(const float sumOfWeightsUpTo) const noexcept -> E
{
  Expects(sumOfWeightsUpTo > SMALL_FLOAT);
  const auto randVal = m_goomRand->GetRandInRange(NumberRange{0.0F, sumOfWeightsUpTo});

  for (auto i = 0U; i < m_weightData.weightArray.size(); ++i)
  {
    if (randVal <= m_weightData.progressiveWeightSumArray[i])
    {
      // NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange): Seems broken
      return static_cast<E>(i);
    }
  }

  std::unreachable();
}

template<EnumType E>
auto Weights<E>::GetSumOfWeightsUpTo(const E& lastVal) const noexcept -> float
{
  return m_weightData.progressiveWeightSumArray.at(static_cast<size_t>(lastVal));
}

template<EnumType E>
auto Weights<E>::GetRandomWeighted(const E& given) const noexcept -> E
{
  auto newWeights = *this;
  newWeights.SetWeightToZero(given);

  return newWeights.GetRandomWeighted();
}

template<EnumType E>
ConditionalWeights<E>::ConditionalWeights(
    const GoomRand& goomRand,
    const typename Weights<E>::EventWeightPairs& eventWeightPairs,
    const bool disallowEventsSameAsGiven) noexcept
  : m_unconditionalWeights{goomRand, eventWeightPairs},
    m_conditionalWeights{},
    m_disallowEventsSameAsGiven{disallowEventsSameAsGiven}
{
}

template<EnumType E>
ConditionalWeights<E>::ConditionalWeights(const GoomRand& goomRand,
                                          const typename Weights<E>::EventWeightPairs& weights,
                                          const EventWeightMultiplierPairs& weightMultiplierPairs,
                                          const bool disallowEventsSameAsGiven) noexcept
  : m_unconditionalWeights{goomRand, weights},
    m_conditionalWeights{GetConditionalWeightMap(goomRand, weights, weightMultiplierPairs)},
    m_disallowEventsSameAsGiven{disallowEventsSameAsGiven}
{
}

template<EnumType E>
auto ConditionalWeights<E>::GetNumSetWeights() const noexcept -> size_t
{
  return m_unconditionalWeights.GetNumSetWeights();
}

template<EnumType E>
auto ConditionalWeights<E>::GetWeight(const ConditionalEvents& events) const noexcept -> float
{
  const auto iter = m_conditionalWeights.find(events.previousEvent);
  if (iter == cend(m_conditionalWeights))
  {
    return m_unconditionalWeights.GetWeight(events.event);
  }
  return iter->second.GetWeight(events.event);
}

template<EnumType E>
auto ConditionalWeights<E>::GetSumOfWeights(const E& given) const noexcept -> float
{
  const auto iter = m_conditionalWeights.find(given);
  if (iter == cend(m_conditionalWeights))
  {
    return m_unconditionalWeights.GetSumOfWeights();
  }
  return iter->second.GetSumOfWeights();
}

template<EnumType E>
auto ConditionalWeights<E>::GetRandomWeighted(const E& given) const noexcept -> E
{
  const auto iter = m_conditionalWeights.find(given);
  if (iter == cend(m_conditionalWeights))
  {
    if (m_disallowEventsSameAsGiven)
    {
      return m_unconditionalWeights.GetRandomWeighted(given);
    }
    return m_unconditionalWeights.GetRandomWeighted();
  }

  const auto& weights = iter->second;
  return weights.GetRandomWeighted();
}

template<EnumType E>
auto ConditionalWeights<E>::GetConditionalWeightMap(
    const GoomRand& goomRand,
    const typename Weights<E>::EventWeightPairs& eventWeightPairs,
    const EventWeightMultiplierPairs& weightMultiplierPairs) noexcept -> std::map<E, Weights<E>>
{
  auto conditionalWeights = std::map<E, Weights<E>>{};
  for (const auto& [given, multiplierPairs] : weightMultiplierPairs)
  {
    typename Weights<E>::EventWeightPairs newEventWeightPairs = eventWeightPairs;
    for (auto& newEventWeightPair : newEventWeightPairs)
    {
      if (multiplierPairs.find(newEventWeightPair.key) == cend(multiplierPairs))
      {
        continue;
      }
      newEventWeightPair.weight *= multiplierPairs.at(newEventWeightPair.key);
    }

    conditionalWeights.emplace(given, Weights<E>{goomRand, newEventWeightPairs});
  }

  return conditionalWeights;
}

} // namespace GOOM::UTILS::MATH
