module;

#include <algorithm>
#include <cmath>
#include <concepts>
#include <cstdint>
#include <type_traits>
#include <vector>

export module Goom.Utils.Math.IncrementedValues;

import Goom.Utils.Math.Misc;
import Goom.Utils.Math.TValues;
import Goom.Lib.AssertUtils;
import Goom.Lib.GoomTypes;

// TODO(glk): How to avoid having to add these??
export template<typename T>
auto lerp(const T& val1, const T& val2, float t) noexcept -> T;
export template<typename T>
auto Clamped(const T& val, const T& val1, const T& val2) noexcept -> T;
export template<typename T>
auto GetMatching(const T& val, const T& val1, const T& val2) noexcept -> float;

template<typename T>
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
auto lerp(const T& val1, const T& val2, const float t) noexcept -> T
{
  return static_cast<T>(std::lerp(val1, val2, t));
}

template<typename T>
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
auto Clamped(const T& val, const T& val1, const T& val2) noexcept -> T
{
  return GOOM::UTILS::MATH::UnorderedClamp(val, val1, val2);
}

template<typename T>
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
auto GetMatching(const T& val, const T& val1, const T& val2) noexcept -> float
{
  using GOOM::UTILS::MATH::SMALL_FLOAT;

  if (std::fabs(static_cast<float>(val2) - static_cast<float>(val1)) < SMALL_FLOAT)
  {
    return 0.0F;
  }
  return ((static_cast<float>(val) - static_cast<float>(val1)) /
          (static_cast<float>(val2) - static_cast<float>(val1)));
}

export namespace GOOM::UTILS::MATH
{

template<typename T>
concept Lerpable = requires(const T& val1, const T& val2, float t) {
  { lerp(val1, val2, t) } -> std::same_as<T>;
};
template<typename T>
concept Matchable = requires(const T& val, const T& val1, const T& val2) {
  { GetMatching(val, val1, val2) } -> std::same_as<float>;
};
template<typename T>
concept Clampable = requires(const T& val, const T& val1, const T& val2) {
  { Clamped(val, val1, val2) } -> std::same_as<T>;
};

template<typename T>
class IncrementedValue
{
public:
  IncrementedValue(TValue::StepType stepType, uint32_t numSteps) noexcept;
  IncrementedValue(const T& value1,
                   const T& value2,
                   TValue::StepType stepType,
                   uint32_t numSteps) noexcept;
  IncrementedValue(const T& value1,
                   const T& value2,
                   TValue::StepType stepType,
                   float stepSize) noexcept;

  [[nodiscard]] auto GetValue1() const noexcept -> const T&;
  [[nodiscard]] auto GetValue2() const noexcept -> const T&;

  auto SetValue1(const T& value1) noexcept -> void;
  auto SetValue2(const T& value2) noexcept -> void;
  auto SetValues(const T& value1, const T& value2) noexcept -> void;
  auto SetNumSteps(uint32_t val) noexcept -> void;
  auto ReverseValues() noexcept -> void;

  [[nodiscard]] auto operator()() const noexcept -> const T&;
  auto Increment() noexcept -> void;

  [[nodiscard]] auto PeekNext() const noexcept -> T;

  [[nodiscard]] auto GetT() const noexcept -> const TValue&;
  auto ResetT(float t = 0.0) noexcept -> void;
  auto ResetCurrentValue(const T& newValue) noexcept -> void;

private:
  T m_value1;
  T m_value2;
  TValue m_t;
  T m_currentValue = m_value1;
  [[nodiscard]] auto GetValue(float t) const noexcept -> T;

  [[nodiscard]] static auto LerpValues(const T& val1, const T& val2, float t) noexcept -> T
    requires Lerpable<T>;
  [[nodiscard]] static auto Clamp(const T& val, const T& val1, const T& val2) noexcept -> T
    requires Clampable<T>;
  [[nodiscard]] static auto GetMatchingT(const T& val, const T& val1, const T& val2) noexcept
      -> float
    requires Matchable<T>;
};

} // namespace GOOM::UTILS::MATH

namespace GOOM::UTILS::MATH
{

template<typename T>
IncrementedValue<T>::IncrementedValue(const TValue::StepType stepType,
                                      const uint32_t numSteps) noexcept
  : m_value1{}, m_value2{}, m_t{{.stepType=stepType, .numSteps=numSteps}}
{
  Expects(numSteps > 0U);
}

template<typename T>
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
IncrementedValue<T>::IncrementedValue(const T& value1,
                                      const T& value2,
                                      const TValue::StepType stepType,
                                      const uint32_t numSteps) noexcept
  : m_value1{value1}, m_value2{value2}, m_t{{.stepType=stepType, .numSteps=numSteps}}
{
  Expects(numSteps > 0U);
}

template<typename T>
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
IncrementedValue<T>::IncrementedValue(const T& value1,
                                      const T& value2,
                                      const TValue::StepType stepType,
                                      const float stepSize) noexcept
  : m_value1{value1}, m_value2{value2}, m_t{TValue::StepSizeProperties{.stepSize=stepSize, .stepType=stepType}}
{
  Expects(stepSize > 0.0F);
}

template<typename T>
auto IncrementedValue<T>::GetValue1() const noexcept -> const T&
{
  return m_value1;
}

template<typename T>
auto IncrementedValue<T>::GetValue2() const noexcept -> const T&
{
  return m_value2;
}

template<typename T>
auto IncrementedValue<T>::SetValue1(const T& value1) noexcept -> void
{
  m_value1 = value1;
  ResetCurrentValue(m_currentValue);
}

template<typename T>
auto IncrementedValue<T>::SetValue2(const T& value2) noexcept -> void
{
  m_value2 = value2;
  ResetCurrentValue(m_currentValue);
}

template<typename T>
auto IncrementedValue<T>::SetValues(const T& value1, const T& value2) noexcept -> void
{
  m_value1 = value1;
  m_value2 = value2;
  ResetCurrentValue(m_currentValue);
}

template<typename T>
auto IncrementedValue<T>::ReverseValues() noexcept -> void
{
  std::swap(m_value1, m_value2);
  ResetCurrentValue(m_currentValue);
}

template<typename T>
auto IncrementedValue<T>::SetNumSteps(const uint32_t val) noexcept -> void
{
  m_t.SetNumSteps(val);
}

template<typename T>
auto IncrementedValue<T>::operator()() const noexcept -> const T&
{
  return m_currentValue;
}

template<typename T>
auto IncrementedValue<T>::Increment() noexcept -> void
{
  m_t.Increment();
  m_currentValue = GetValue(m_t());
}

template<typename T>
auto IncrementedValue<T>::PeekNext() const noexcept -> T
{
  auto tCopy = m_t;
  tCopy.Increment();
  return GetValue(tCopy());
}

template<typename T>
auto IncrementedValue<T>::GetValue(const float t) const noexcept -> T
{
  return LerpValues(m_value1, m_value2, t);
}

template<typename T>
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
auto IncrementedValue<T>::LerpValues(const T& val1, const T& val2, float t) noexcept -> T
  requires Lerpable<T>
{
  return lerp(val1, val2, t);
}

template<typename T>
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
auto IncrementedValue<T>::Clamp(const T& val, const T& val1, const T& val2) noexcept -> T
  requires Clampable<T>
{
  return Clamped(val, val1, val2);
}

template<typename T>
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
auto IncrementedValue<T>::GetMatchingT(const T& val, const T& val1, const T& val2) noexcept -> float
  requires Matchable<T>
{
  return GetMatching(val, val1, val2);
}

template<typename T>
auto IncrementedValue<T>::GetT() const noexcept -> const TValue&
{
  return m_t;
}

template<typename T>
auto IncrementedValue<T>::ResetT(const float t) noexcept -> void
{
  m_t.Reset(t);
  m_currentValue = GetValue(m_t());
}

template<typename T>
auto IncrementedValue<T>::ResetCurrentValue(const T& newValue) noexcept -> void
{
  const auto newClampedValue = Clamp(newValue, m_value1, m_value2);
  ResetT(GetMatchingT(newClampedValue, m_value1, m_value2));
}

} // namespace GOOM::UTILS::MATH
