module;

#include "goom/goom_logger.h"

#include <cmath>
#include <cstdint>

module Goom.VisualFx.ShaderFx:HueShifterLerper;

import Goom.Utils.Timer;
import Goom.Utils.Math.TValues;
import Goom.Utils.Math.GoomRand;
import Goom.Utils.Math.Misc;
import Goom.Lib.AssertUtils;
import Goom.PluginInfo;

using GOOM::UTILS::Timer;
using GOOM::UTILS::MATH::FULL_CIRCLE_RANGE;
using GOOM::UTILS::MATH::GoomRand;
using GOOM::UTILS::MATH::NumberRange;
using GOOM::UTILS::MATH::TValue;
using GOOM::UTILS::MATH::TWO_PI;

namespace GOOM::VISUAL_FX::SHADERS
{

class HueShiftLerper
{
public:
  struct Params
  {
    NumberRange<uint32_t> numLerpStepsRange{};
    NumberRange<uint32_t> lerpConstTimeRange{};
  };

  HueShiftLerper(const PluginInfo& goomInfo,
                 const GoomRand& goomRand,
                 const Params& params) noexcept;

  auto Update() noexcept -> void;
  auto ChangeValueRange() noexcept -> void;

  [[nodiscard]] auto GetLerpedValue() const noexcept -> float;

private:
  const PluginInfo* m_goomInfo;
  const GoomRand* m_goomRand;
  Params m_params;

  float m_srceHueShift    = 0.0F;
  float m_destHueShift    = 0.0F;
  float m_currentHueShift = 0.0F;

  TValue m_lerpT;
  auto RestartLerpWithNewDestHue() noexcept -> void;
  auto RestartLerp() noexcept -> void;
  auto SetNewDestHue() noexcept -> void;
  auto StopLerpAndSetHueShiftOff() noexcept -> void;

  Timer m_lerpConstTimer;

  [[nodiscard]] auto CanRestartLerp() const noexcept -> bool;
};

} // namespace GOOM::VISUAL_FX::SHADERS

namespace GOOM::VISUAL_FX::SHADERS
{

inline auto HueShiftLerper::ChangeValueRange() noexcept -> void
{
  m_lerpConstTimer.SetToFinished();

  RestartLerpWithNewDestHue();
}

inline auto HueShiftLerper::GetLerpedValue() const noexcept -> float
{
  return m_currentHueShift;
}

HueShiftLerper::HueShiftLerper(const PluginInfo& goomInfo,
                               const GoomRand& goomRand,
                               const Params& params) noexcept
  : m_goomInfo{&goomInfo},
    m_goomRand{&goomRand},
    m_params{params},
    m_lerpT{{.stepType=TValue::StepType::SINGLE_CYCLE, .numSteps=m_params.numLerpStepsRange.min}},
    m_lerpConstTimer{m_goomInfo->GetTime(), m_params.lerpConstTimeRange.min, false}
{
}

auto HueShiftLerper::Update() noexcept -> void
{
  if (not m_lerpConstTimer.Finished())
  {
    if (not CanRestartLerp())
    {
      return;
    }
    m_lerpConstTimer.SetToFinished();
  }

  Expects(m_lerpConstTimer.Finished());
  if (m_lerpConstTimer.JustFinished())
  {
    RestartLerpWithNewDestHue();
  }

  m_currentHueShift = std::lerp(m_srceHueShift, m_destHueShift, m_lerpT());

  m_lerpT.Increment();
  if (m_lerpT.IsStopped())
  {
    StopLerpAndSetHueShiftOff();
  }
}

auto HueShiftLerper::RestartLerpWithNewDestHue() noexcept -> void
{
  RestartLerp();
  SetNewDestHue();
}

inline auto HueShiftLerper::RestartLerp() noexcept -> void
{
  m_lerpT.SetNumSteps(m_goomRand->GetRandInRange(m_params.numLerpStepsRange));

  LogInfo("LerpT = {}. Reset lerpT steps to {}", m_lerpT(), m_lerpT.GetNumSteps());
}

inline auto HueShiftLerper::SetNewDestHue() noexcept -> void
{
  m_srceHueShift = GetLerpedValue();
  m_lerpT.Reset(0.0F);

  m_destHueShift =
      std::fmod(m_srceHueShift + m_goomRand->GetRandInRange<FULL_CIRCLE_RANGE>(), TWO_PI);
  LogInfo("Reset m_destHueShift = {}.", m_destHueShift);
}

inline auto HueShiftLerper::StopLerpAndSetHueShiftOff() noexcept -> void
{
  m_lerpConstTimer.SetTimeLimitAndResetToZero(
      m_goomRand->GetRandInRange(m_params.lerpConstTimeRange));
  LogInfo("LerpT = {}. Set off timer {}", m_lerpT(), m_lerpOffTimer.GetTimeLeft());

  m_srceHueShift = GetLerpedValue();
  m_destHueShift = m_srceHueShift;
  m_lerpT.Reset(0.0F);
}

inline auto HueShiftLerper::CanRestartLerp() const noexcept -> bool
{
  if (static constexpr float PROB_RESTART_LERP_AFTER_BIG_GOOM = 0.5F;
      m_goomRand->ProbabilityOf<PROB_RESTART_LERP_AFTER_BIG_GOOM>() and
      (0 == m_goomInfo->GetSoundEvents().GetTimeSinceLastBigGoom()))
  {
    LogInfo("Restarting lerp - GetTimeSinceLastBigGoom = {}",
            m_goomInfo.GetSoundEvents().GetTimeSinceLastBigGoom());
    return true;
  }

  if (static constexpr float PROB_RESTART_LERP_AFTER_GOOM = 0.01F;
      m_goomRand->ProbabilityOf<PROB_RESTART_LERP_AFTER_GOOM>() and
      (0 == m_goomInfo->GetSoundEvents().GetTimeSinceLastGoom()))
  {
    LogInfo("Restarting lerp - GetTimeSinceLastGoom = {}",
            m_goomInfo.GetSoundEvents().GetTimeSinceLastGoom());
    return true;
  }

  LogInfo("Not restarting lerp - GetTimeSinceLastGoom = {}, GetTimeSinceLastBigGoom = {}",
          m_goomInfo.GetSoundEvents().GetTimeSinceLastGoom(),
          m_goomInfo.GetSoundEvents().GetTimeSinceLastBigGoom());

  return false;
}

} // namespace GOOM::VISUAL_FX::SHADERS
