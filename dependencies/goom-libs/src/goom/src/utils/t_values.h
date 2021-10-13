#pragma once

#include "mathutils.h"

#include <cstdint>

#if __cplusplus <= 201402L
namespace GOOM
{
namespace UTILS
{
#else
namespace GOOM::UTILS
{
#endif


class TValue
{
public:
  static constexpr float T_EPSILON = 1.0e-07F;
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

  [[nodiscard]] auto GetStepType() const -> StepType;
  [[nodiscard]] auto GetStepSize() const -> float;
  void SetStepSize(float val);

  auto operator()() const -> float;
  [[nodiscard]] auto GetCurrentStep() const -> float;
  void Increment();
  [[nodiscard]] auto IsStopped() const -> bool;
  void Reset(float t = 0.0);

private:
  const StepType m_stepType;
  float m_stepSize;
  float m_currentStep;
  float m_t;
  const std::vector<DelayPoint> m_delayPoints;
  std::vector<DelayPoint> m_currentDelayPoints;
  bool m_startedDelay = false;
  bool m_justFinishedDelay = false;
  uint32_t m_delayPointCount = 0;
  [[nodiscard]] auto IsInDelayZone() -> bool;
  [[nodiscard]] auto IsInThisDelayZone(const DelayPoint& delayPoint) const -> bool;
  [[nodiscard]] auto WeAreStartingDelayPoint() -> bool;
  void ValidateDelayPoints();
  void SingleCycleIncrement();
  void ContinuousRepeatableIncrement();
  void ContinuousReversibleIncrement();
  void CheckContinuousReversibleBoundary();
  void HandleBoundary(float continueValue, float stepSign);
};

inline auto TValue::GetStepType() const -> StepType
{
  return m_stepType;
}

inline auto TValue::GetStepSize() const -> float
{
  return m_stepSize;
}

inline auto TValue::operator()() const -> float
{
  return m_t;
}

inline auto TValue::GetCurrentStep() const -> float
{
  return m_currentStep;
}

inline auto TValue::IsStopped() const -> bool
{
  if (m_stepType != StepType::SINGLE_CYCLE)
  {
    return false;
  }
  return m_t >= MAX_T_VALUE;
}

inline void TValue::Reset(const float t)
{
  m_t = t;
  m_currentStep = m_stepSize;
  m_startedDelay = false;
  m_justFinishedDelay = false;
}

inline auto TValue::IsInThisDelayZone(const DelayPoint& delayPoint) const -> bool
{
  return ((delayPoint.t0 - m_stepSize + T_EPSILON) < m_t)
         && (m_t < (delayPoint.t0 + m_stepSize - T_EPSILON));
}

#if __cplusplus <= 201402L
} // namespace UTILS
} // namespace GOOM
#else
} // namespace GOOM::UTILS
#endif
