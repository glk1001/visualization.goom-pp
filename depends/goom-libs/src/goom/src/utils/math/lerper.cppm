module;

#include <cmath>
#include <concepts>
#include <cstdint>

export module Goom.Utils.Math.Lerper;

import Goom.Utils.Math.TValues;
import Goom.Lib.GoomTypes;
import Goom.Lib.Point2d;

// TODO(glk): How to avoid having to add these??
template<typename T>
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
auto Lerp(const T& val1, const T& val2, const float t) noexcept -> T
{
  return static_cast<T>(std::lerp(val1, val2, t));
}

template<>
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
auto Lerp(const GOOM::Point2dFlt& val1, const GOOM::Point2dFlt& val2, const float t) noexcept
    -> GOOM::Point2dFlt
{
  return static_cast<GOOM::Point2dFlt>(GOOM::lerp(val1, val2, t));
}

export namespace GOOM::UTILS::MATH
{

template<typename T>
concept Lerpable = requires(const T& val1, const T& val2, float t) {
  { Lerp(val1, val2, t) } -> std::same_as<T>;
};

template<Lerpable T>
class Lerper
{
public:
  enum class LerperType : UnderlyingEnumType
  {
    SINGLE,
    CONTINUOUS,
  };
  Lerper() noexcept = default;
  Lerper(uint32_t numSteps,
         const T& value1,
         const T& value2,
         LerperType lerperType = LerperType::SINGLE) noexcept;

  auto ResetValues(const T& value1, const T& value2) noexcept -> void;
  auto SetNumSteps(uint32_t val) noexcept -> void;

  [[nodiscard]] auto operator()() const noexcept -> const T&;
  auto Increment() noexcept -> void;

  [[nodiscard]] auto GetT() const noexcept -> float;
  auto ResetT(float t = 0.0) noexcept -> void;

private:
  TValue m_t{
      TValue::NumStepsProperties{.stepType = TValue::StepType::SINGLE_CYCLE, .numSteps = 1U}
  };
  T m_value1{};
  T m_value2{};
  T m_currentValue = m_value1;

  [[nodiscard]] static auto GetStepType(LerperType lerperType) noexcept -> TValue::StepType;
};

} // namespace GOOM::UTILS::MATH

namespace GOOM::UTILS::MATH
{

template <Lerpable T>
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
Lerper<T>::Lerper(const uint32_t numSteps,
                  const T& value1,
                  const T& value2,
                  const LerperType lerperType) noexcept
  : m_t{TValue::NumStepsProperties{.stepType = GetStepType(lerperType), .numSteps = numSteps}},
    m_value1{value1},
    m_value2{value2}
{
}

template<Lerpable T>
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
auto Lerper<T>::ResetValues(const T& value1, const T& value2) noexcept -> void
{
  m_value1 = value1;
  m_value2 = value2;
  ResetT(0.0F);
}

template<Lerpable T>
auto Lerper<T>::SetNumSteps(const uint32_t val) noexcept -> void
{
  m_t.SetNumSteps(val);
}

template<Lerpable T>
auto Lerper<T>::operator()() const noexcept -> const T&
{
  return m_currentValue;
}

template<Lerpable T>
auto Lerper<T>::Increment() noexcept -> void
{
  m_t.Increment();
  m_currentValue = Lerp(m_value1, m_value2, m_t());
}

template<Lerpable T>
auto Lerper<T>::GetT() const noexcept -> float
{
  return m_t();
}

template<Lerpable T>
auto Lerper<T>::ResetT(const float t) noexcept -> void
{
  m_t.Reset(t);
  m_currentValue = Lerp(m_value1, m_value2, m_t());
}

template<Lerpable T>
auto Lerper<T>::GetStepType(LerperType lerperType) noexcept -> TValue::StepType
{
  switch (lerperType)
  {
    case LerperType::SINGLE:
      return TValue::StepType::SINGLE_CYCLE;
    case LerperType::CONTINUOUS:
      return TValue::StepType::CONTINUOUS_REVERSIBLE;
  }
}

} // namespace GOOM::UTILS::MATH
