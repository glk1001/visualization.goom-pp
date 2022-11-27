#pragma once

#include "goom_config.h"
#include "math/misc.h"

#include <cstdint>
#include <vector>

namespace GOOM::UTILS
{

class TValue
{
public:
  static constexpr float T_EPSILON   = 1.0e-07F;
  static constexpr float MAX_T_VALUE = 1.0F + T_EPSILON;
  struct DelayPoint
  {
    float t0;
    uint32_t delayTime;
  };
  enum class StepType
  {
    CONTINUOUS_REPEATABLE,
    CONTINUOUS_REVERSIBLE,
    SINGLE_CYCLE,
  };

  TValue(StepType stepType, float stepSize, float startingT = 0.0F) noexcept;
  TValue(StepType stepType,
         float stepSize,
         const std::vector<DelayPoint>& delayPoints,
         float startingT = 0.0F) noexcept;
  TValue(StepType stepType, uint32_t numSteps, float startingT = 0.0F) noexcept;
  TValue(StepType stepType,
         uint32_t numSteps,
         const std::vector<DelayPoint>& delayPoints,
         float startingT = 0.0F) noexcept;
  TValue(const TValue&) noexcept = default;
  TValue(TValue&&) noexcept      = default;
  ~TValue() noexcept;
  auto operator=(const TValue&) noexcept -> TValue& = delete;
  auto operator=(TValue&&) noexcept -> TValue&      = delete;

  [[nodiscard]] auto GetStepType() const noexcept -> StepType;

  [[nodiscard]] auto GetStepSize() const noexcept -> float;
  auto SetStepSize(float val) noexcept -> void;

  [[nodiscard]] auto GetNumSteps() const noexcept -> uint32_t;
  auto SetNumSteps(uint32_t val) noexcept -> void;

  auto operator()() const noexcept -> float;

  enum class Boundaries
  {
    START,
    INSIDE,
    END
  };
  [[nodiscard]] auto HasJustHitStartBoundary() const noexcept -> bool;
  [[nodiscard]] auto HasJustHitEndBoundary() const noexcept -> bool;

  [[nodiscard]] auto DelayJustFinishing() const noexcept -> bool;
  [[nodiscard]] auto IsDelayed() const noexcept -> bool;

  [[nodiscard]] auto GetCurrentStep() const noexcept -> float;
  auto Increment() noexcept -> void;
  [[nodiscard]] auto IsStopped() const noexcept -> bool;
  auto Reset(float t = 0.0) noexcept -> void;

private:
  const StepType m_stepType;
  float m_stepSize;
  float m_currentStep{m_stepSize};
  float m_t;
  Boundaries m_currentPosition{(m_stepType == StepType::SINGLE_CYCLE) ? Boundaries::INSIDE
                                                                      : Boundaries::START};
  const std::vector<DelayPoint> m_delayPoints;
  std::vector<DelayPoint> m_currentDelayPoints{m_delayPoints};
  bool m_startedDelay        = false;
  bool m_justFinishedDelay   = false;
  uint32_t m_delayPointCount = 0;
  [[nodiscard]] auto IsInDelayZone() noexcept -> bool;
  [[nodiscard]] auto IsInThisDelayZone(const DelayPoint& delayPoint) const noexcept -> bool;
  [[nodiscard]] auto WeAreStartingDelayPoint() noexcept -> bool;
  auto ValidateDelayPoints() const noexcept -> void;

  auto SingleCycleIncrement() noexcept -> void;
  auto ContinuousRepeatableIncrement() noexcept -> void;
  auto ContinuousReversibleIncrement() noexcept -> void;
  auto CheckContinuousReversibleBoundary() noexcept -> void;
  auto HandleBoundary(float continueValue, float stepSign) noexcept -> void;
};

template<typename T>
class IncrementedValue
{
public:
  IncrementedValue(const T& value1,
                   const T& value2,
                   TValue::StepType stepType,
                   uint32_t numSteps) noexcept;

  auto SetValue1(const T& value1) noexcept -> void;
  auto SetValue2(const T& value2) noexcept -> void;
  auto SetValues(const T& value1, float value2) noexcept -> void;

  [[nodiscard]] auto operator()() const noexcept -> const T&;
  auto Increment() noexcept -> void;

  [[nodiscard]] auto PeekNext() const noexcept -> T;

  [[nodiscard]] auto GetT() const noexcept -> const TValue&;
  auto ResetT(float t = 0.0) noexcept -> void;

private:
  T m_value1;
  T m_value2;
  TValue m_t;
  T m_currentValue = m_value1;
  [[nodiscard]] auto GetValue(float t) const noexcept -> T;
};

inline auto TValue::GetStepType() const noexcept -> StepType
{
  return m_stepType;
}

inline auto TValue::GetStepSize() const noexcept -> float
{
  return m_stepSize;
}

inline auto TValue::operator()() const noexcept -> float
{
  return m_t;
}

inline auto TValue::HasJustHitStartBoundary() const noexcept -> bool
{
  return m_currentPosition == Boundaries::START;
}

inline auto TValue::HasJustHitEndBoundary() const noexcept -> bool
{
  return m_currentPosition == Boundaries::END;
}

inline auto TValue::DelayJustFinishing() const noexcept -> bool
{
  return 1 == m_delayPointCount;
}

inline auto TValue::IsDelayed() const noexcept -> bool
{
  return m_delayPointCount > 0;
}

inline auto TValue::GetCurrentStep() const noexcept -> float
{
  return m_currentStep;
}

inline auto TValue::IsStopped() const noexcept -> bool
{
  if (m_stepType != StepType::SINGLE_CYCLE)
  {
    return false;
  }
  return m_t >= MAX_T_VALUE;
}

inline auto TValue::Reset(const float t) noexcept -> void
{
  m_t                 = t;
  m_currentStep       = m_stepSize;
  m_startedDelay      = false;
  m_justFinishedDelay = false;
}

inline auto TValue::IsInThisDelayZone(const DelayPoint& delayPoint) const noexcept -> bool
{
  return (((delayPoint.t0 - m_stepSize) + T_EPSILON) < m_t) &&
         (m_t < ((delayPoint.t0 + m_stepSize) - T_EPSILON));
}

template<typename T>
inline IncrementedValue<T>::IncrementedValue(const T& value1,
                                             const T& value2,
                                             const TValue::StepType stepType,
                                             const uint32_t numSteps) noexcept
  : m_value1{value1}, m_value2{value2}, m_t{stepType, numSteps}
{
  Expects(numSteps > 0U);
}

template<typename T>
inline auto IncrementedValue<T>::SetValue1(const T& value1) noexcept -> void
{
  m_value1 = value1;
}

template<typename T>
inline auto IncrementedValue<T>::SetValue2(const T& value2) noexcept -> void
{
  m_value2 = value2;
}

template<typename T>
inline auto IncrementedValue<T>::SetValues(const T& value1, const float value2) noexcept -> void
{
  m_value1 = value1;
  m_value2 = value2;
}

template<typename T>
inline auto IncrementedValue<T>::operator()() const noexcept -> const T&
{
  return m_currentValue;
}

template<typename T>
inline auto IncrementedValue<T>::Increment() noexcept -> void
{
  m_t.Increment();
  m_currentValue = GetValue(m_t());
}

template<typename T>
inline auto IncrementedValue<T>::PeekNext() const noexcept -> T
{
  auto tCopy = m_t;
  tCopy.Increment();
  return GetValue(tCopy());
}

template<typename T>
inline auto IncrementedValue<T>::GetValue(const float t) const noexcept -> T
{
  return STD20::lerp(m_value1, m_value2, t);
}

template<typename T>
inline auto IncrementedValue<T>::GetT() const noexcept -> const TValue&
{
  return m_t;
}

template<typename T>
inline auto IncrementedValue<T>::ResetT(const float t) noexcept -> void
{
  m_t.Reset(t);
  m_currentValue = GetValue(m_t());
}

} // namespace GOOM::UTILS
