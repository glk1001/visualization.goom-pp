#pragma once

#include "enumutils.h"
#include "mathutils.h"

#include <cstdint>
#include <format>
#include <map>
#include <stdexcept>
#if __cplusplus <= 201402L
#include <tuple>
#endif
#include <type_traits>
#include <vector>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace UTILS
{
#else
namespace GOOM::UTILS
{
#endif

class IGoomRand
{
public:
  IGoomRand() noexcept = default;
  IGoomRand(const IGoomRand&) noexcept = delete;
  IGoomRand(IGoomRand&&) noexcept = delete;
  auto operator=(const IGoomRand&) -> IGoomRand& = delete;
  auto operator=(IGoomRand&&) -> IGoomRand& = delete;
  virtual ~IGoomRand() noexcept = default;

  // Return random integer in the range 0 <= n < n1.
  [[nodiscard]] virtual auto GetNRand(uint32_t n1) const -> uint32_t = 0;
  // Return random integer in the range 0 <= n < randMax.
  [[nodiscard]] virtual auto GetRand() const -> uint32_t = 0;

  // Return random number in the range n0 <= n < n1.
  [[nodiscard]] virtual auto GetRandInRange(uint32_t n0, uint32_t n1) const -> uint32_t = 0;
  [[nodiscard]] virtual auto GetRandInRange(int32_t n0, int32_t n1) const -> int32_t = 0;
  [[nodiscard]] virtual auto GetRandInRange(float n0, float n1) const -> float = 0;
  template<typename T>
  struct NumberRange
  {
    T min;
    T max;
  };
  template<typename T>
  [[nodiscard]] auto GetRandInRange(const NumberRange<T>& numberRange) const -> T;

  template<class RandomIt>
  void Shuffle(RandomIt first, RandomIt last) const;

  // Return prob(m/n)
  [[nodiscard]] virtual auto ProbabilityOfMInN(uint32_t m, uint32_t n) const -> bool = 0;
  [[nodiscard]] virtual auto ProbabilityOf(float x) const -> bool = 0;
};

template<class E>
class Weights
{
public:
  using EventWeightPairs = std::vector<std::pair<E, float>>;

  Weights(const IGoomRand& goomRand, const EventWeightPairs& weights);

  [[nodiscard]] auto GetNumElements() const -> size_t;
  [[nodiscard]] auto GetWeight(const E& enumClass) const -> float;
  [[nodiscard]] auto GetSumOfWeights() const -> float;

  [[nodiscard]] auto GetRandomWeighted() const -> E;

private:
  template<class F>
  friend class ConditionalWeights;
  [[nodiscard]] auto GetRandomWeighted(const E& given) const -> E;
  const IGoomRand& m_goomRand;
  using WeightArray = std::array<float, NUM<E>>;
  const WeightArray m_weights;
  const float m_sumOfWeights;
  [[nodiscard]] static auto GetWeights(const EventWeightPairs& eventWeightPairs) -> WeightArray;
  [[nodiscard]] static auto GetSumOfWeights(const EventWeightPairs& eventWeightPairs) -> float;
};

template<class E>
class ConditionalWeights
{
public:
  // Array of pairs: 'given' enum and corresponding array of weight multipliers.
  using EventWeightMultiplierPairs = std::vector<std::pair<E, std::map<E, float>>>;

  ConditionalWeights(const IGoomRand& goomRand,
                     const typename Weights<E>::EventWeightPairs& eventWeightPairs,
                     bool disallowEventsSameAsGiven = true);
  ConditionalWeights(const IGoomRand& goomRand,
                     const typename Weights<E>::EventWeightPairs& weights,
                     const EventWeightMultiplierPairs& weightMultiplierPairs,
                     bool disallowEventsSameAsGiven = true);

  [[nodiscard]] auto GetWeight(const E& given, const E& enumClass) const -> float;
  [[nodiscard]] auto GetSumOfWeights(const E& given) const -> float;

  [[nodiscard]] auto GetRandomWeighted(const E& given) const -> E;

private:
  const Weights<E> m_unconditionalWeights;
  const std::map<E, Weights<E>> m_conditionalWeights;
  const bool m_disallowEventsSameAsGiven;
  [[nodiscard]] static auto GetConditionalWeightMap(
      const IGoomRand& goomRand,
      const typename Weights<E>::EventWeightPairs& eventWeightPairs,
      const EventWeightMultiplierPairs& weightMultiplierPairs) -> std::map<E, Weights<E>>;
};

template<typename T>
inline auto IGoomRand::GetRandInRange(const NumberRange<T>& numberRange) const -> T
{
  if (std::is_integral<T>())
  {
    return GetRandInRange(numberRange.min, numberRange.max + 1);
  }
  return GetRandInRange(numberRange.min, numberRange.max);
}

template<class RandomIt>
inline void IGoomRand::Shuffle(RandomIt first, RandomIt last) const
{
  using DiffType = typename std::iterator_traits<RandomIt>::difference_type;

  const DiffType n = last - first;
  for (DiffType i = n - 1; i > 0; --i)
  {
    std::swap(first[i], first[GetRandInRange(0, static_cast<int32_t>(i + 1))]);
  }
}

template<class E>
inline Weights<E>::Weights(const IGoomRand& goomRand, const EventWeightPairs& weights)
  : m_goomRand{goomRand}, m_weights{GetWeights(weights)}, m_sumOfWeights{GetSumOfWeights(weights)}
{
  if (m_sumOfWeights < SMALL_FLOAT)
  {
    throw std::logic_error("Sum of weights is zero.");
  }
}

template<class E>
inline auto Weights<E>::GetWeights(const EventWeightPairs& eventWeightPairs) -> WeightArray
{
  WeightArray weightArray{0};
#if __cplusplus <= 201402L
  for (const auto& wgt : eventWeightPairs)
  {
    const auto& e = wgt.first;
    const auto& w = wgt.second;
#else
  for (const auto& [e, w] : eventWeightPairs)
  {
#endif
    weightArray.at(static_cast<size_t>(e)) = w;
  }

  return weightArray;
}

template<class E>
inline auto Weights<E>::GetSumOfWeights(const EventWeightPairs& eventWeightPairs) -> float
{
  float sumOfWeights = 0.0F;
#if __cplusplus <= 201402L
  for (const auto& wgt : eventWeightPairs)
  {
    const auto& w = wgt.second;
#else
  for (const auto& [e, w] : eventWeightPairs)
  {
#endif
    sumOfWeights += w;
  }
  return sumOfWeights - SMALL_FLOAT;
}

template<class E>
inline auto Weights<E>::GetNumElements() const -> size_t
{
  return m_weights.size();
}

template<class E>
inline auto Weights<E>::GetWeight(const E& enumClass) const -> float
{
  for (size_t i = 0; i < m_weights.size(); ++i)
  {
    if (static_cast<E>(i) == enumClass)
    {
      return m_weights[i];
    }
  }

  return 0.0F;
}

template<class E>
inline auto Weights<E>::GetSumOfWeights() const -> float
{
  return m_sumOfWeights;
}

template<class E>
inline auto Weights<E>::GetRandomWeighted() const -> E
{
  return GetRandomWeighted(E::_NUM);
}

template<class E>
inline auto Weights<E>::GetRandomWeighted(const E& given) const -> E
{
  const float sumOfWeights = (given == E::_NUM)
                                 ? m_sumOfWeights
                                 : (m_sumOfWeights - m_weights[static_cast<size_t>(given)]);

  float randVal = m_goomRand.GetRandInRange(0.0F, sumOfWeights);

  for (size_t i = 0; i < m_weights.size(); ++i)
  {
    if (static_cast<E>(i) == given)
    {
      continue;
    }
    if (randVal < m_weights[i])
    {
      return static_cast<E>(i);
    }
    randVal -= m_weights[i];
  }

  throw std::logic_error(std20::format("Should not get here. randVal = {}.", randVal));
}

template<class E>
inline ConditionalWeights<E>::ConditionalWeights(
    const IGoomRand& goomRand,
    const typename Weights<E>::EventWeightPairs& eventWeightPairs,
    const bool disallowEventsSameAsGiven)
  : m_unconditionalWeights{goomRand, eventWeightPairs},
    m_conditionalWeights{},
    m_disallowEventsSameAsGiven{disallowEventsSameAsGiven}
{
}

template<class E>
inline ConditionalWeights<E>::ConditionalWeights(
    const IGoomRand& goomRand,
    const typename Weights<E>::EventWeightPairs& weights,
    const EventWeightMultiplierPairs& weightMultiplierPairs,
    const bool disallowEventsSameAsGiven)
  : m_unconditionalWeights{goomRand, weights},
    m_conditionalWeights{GetConditionalWeightMap(goomRand, weights, weightMultiplierPairs)},
    m_disallowEventsSameAsGiven{disallowEventsSameAsGiven}
{
}

template<class E>
inline auto ConditionalWeights<E>::GetWeight(const E& given, const E& enumClass) const -> float
{
  const auto iter = m_conditionalWeights.find(given);
  if (iter == cend(m_conditionalWeights))
  {
    return m_unconditionalWeights.GetWeight(enumClass);
  }
  return iter->second.GetWeight(enumClass);
}

template<class E>
inline auto ConditionalWeights<E>::GetSumOfWeights(const E& given) const -> float
{
  const auto iter = m_conditionalWeights.find(given);
  if (iter == cend(m_conditionalWeights))
  {
    return m_unconditionalWeights.GetSumOfWeights();
  }
  return iter->second.GetSumOfWeights();
}

template<class E>
inline auto ConditionalWeights<E>::GetRandomWeighted(const E& given) const -> E
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

  const Weights<E>& weights = iter->second;
  return weights.GetRandomWeighted();
}

template<class E>
auto ConditionalWeights<E>::GetConditionalWeightMap(
    const IGoomRand& goomRand,
    const typename Weights<E>::EventWeightPairs& eventWeightPairs,
    const EventWeightMultiplierPairs& weightMultiplierPairs) -> std::map<E, Weights<E>>
{
  std::map<E, Weights<E>> conditionalWeights{};
  for (const auto& weightMultiplierPair : weightMultiplierPairs)
  {
    const E& given = weightMultiplierPair.first;
    const std::map<E, float>& multiplierPairs = weightMultiplierPair.second;

    typename Weights<E>::EventWeightPairs newEventWeightPairs = eventWeightPairs;
    for (auto& newEventWeightPair : newEventWeightPairs)
    {
      if (multiplierPairs.find(newEventWeightPair.first) == cend(multiplierPairs))
      {
        continue;
      }
      newEventWeightPair.second *= multiplierPairs.at(newEventWeightPair.first);
    }

    conditionalWeights.emplace(given, Weights<E>{goomRand, newEventWeightPairs});
  }

  return conditionalWeights;
}

#if __cplusplus <= 201402L
} // namespace UTILS
} // namespace GOOM
#else
} // namespace GOOM::UTILS
#endif
