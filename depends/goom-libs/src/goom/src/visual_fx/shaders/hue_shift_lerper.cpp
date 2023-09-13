//#undef NO_LOGGING

#include "hue_shift_lerper.h"

#include "goom/goom_config.h"
#include "goom/goom_logger.h"
#include "goom/math20.h"
#include "goom_plugin_info.h"
#include "utils/math/goom_rand_base.h"
#include "utils/math/misc.h"

#include <cmath>

namespace GOOM::VISUAL_FX::SHADERS
{

using UTILS::MATH::TWO_PI;

HueShiftLerper::HueShiftLerper(const PluginInfo& goomInfo,
                               const UTILS::MATH::IGoomRand& goomRand,
                               const LerpData& lerpData) noexcept
  : m_goomInfo{&goomInfo}, m_goomRand{&goomRand}, m_lerpData{lerpData}
{
}

auto HueShiftLerper::Update() noexcept -> void
{
  if (not m_lerpOffTimer.Finished())
  {
    if (not CanRestartLerp())
    {
      return;
    }
    m_lerpOffTimer.SetToFinished();
  }

  Expects(m_lerpOffTimer.Finished());
  if (m_lerpOffTimer.JustFinished())
  {
    RestartLerpWithNewDestHue();
  }

  m_currentHueShift = STD20::lerp(m_srceHueShift, m_destHueShift, m_lerpT());

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
  m_lerpT.SetNumSteps(
      m_goomRand->GetRandInRange(m_lerpData.minNumLerpOnSteps, m_lerpData.maxNumLerpOnSteps + 1));

  LogInfo("LerpT = {}. Reset lerpT steps to {}", m_lerpT(), m_lerpT.GetNumSteps());
}

inline auto HueShiftLerper::SetNewDestHue() noexcept -> void
{
  m_srceHueShift = GetHueShift();
  m_lerpT.Reset(0.0F);

  static constexpr auto MAX_CHANGE = TWO_PI;
  m_destHueShift = std::fmod(m_srceHueShift + m_goomRand->GetRandInRange(0.0F, MAX_CHANGE), TWO_PI);
  LogInfo("Reset m_destHueShift = {}.", m_destHueShift);
}

inline auto HueShiftLerper::StopLerpAndSetHueShiftOff() noexcept -> void
{
  m_lerpOffTimer.SetTimeLimit(
      m_goomRand->GetRandInRange(m_lerpData.minLerpOffTime, m_lerpData.maxLerpOffTime + 1));
  LogInfo("LerpT = {}. Set off timer {}", m_lerpT(), m_lerpOffTimer.GetTimeLeft());

  m_srceHueShift = GetHueShift();
  m_destHueShift = m_srceHueShift;
  m_lerpT.Reset(0.0F);
}

inline auto HueShiftLerper::CanRestartLerp() const noexcept -> bool
{
  if (static constexpr float PROB_RESTART_LERP_AFTER_BIG_GOOM = 0.5F;
      m_goomRand->ProbabilityOf(PROB_RESTART_LERP_AFTER_BIG_GOOM) and
      (0 == m_goomInfo->GetSoundEvents().GetTimeSinceLastBigGoom()))
  {
    LogInfo("Restarting lerp - GetTimeSinceLastBigGoom = {}",
            m_goomInfo.GetSoundEvents().GetTimeSinceLastBigGoom());
    return true;
  }

  if (static constexpr float PROB_RESTART_LERP_AFTER_GOOM = 0.01F;
      m_goomRand->ProbabilityOf(PROB_RESTART_LERP_AFTER_GOOM) and
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
