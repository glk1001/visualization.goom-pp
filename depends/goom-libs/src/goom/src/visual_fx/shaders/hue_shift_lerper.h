#pragma once

#include "goom_plugin_info.h"
#include "utils/math/goom_rand_base.h"
#include "utils/t_values.h"
#include "utils/timer.h"

#include <cstdint>

namespace GOOM::VISUAL_FX::SHADERS
{

class HueShiftLerper
{
public:
  struct LerpData
  {
    uint32_t minNumLerpOnSteps{};
    uint32_t maxNumLerpOnSteps{};
    uint32_t minLerpOffTime{};
    uint32_t maxLerpOffTime{};
  };

  HueShiftLerper(const PluginInfo& goomInfo,
                 const UTILS::MATH::IGoomRand& goomRand,
                 const LerpData& lerpData) noexcept;

  auto Update() noexcept -> void;
  auto ChangeHue() noexcept -> void;

  [[nodiscard]] auto GetHueShift() const noexcept -> float;

private:
  const PluginInfo* m_goomInfo;
  const UTILS::MATH::IGoomRand* m_goomRand;
  LerpData m_lerpData;

  float m_srceHueShift    = 0.0F;
  float m_destHueShift    = 0.0F;
  float m_currentHueShift = 0.0F;

  UTILS::TValue m_lerpT{
      {UTILS::TValue::StepType::SINGLE_CYCLE, m_lerpData.minNumLerpOnSteps}
  };
  auto RestartLerpWithNewDestHue() noexcept -> void;
  auto RestartLerp() noexcept -> void;
  auto SetNewDestHue() noexcept -> void;
  auto StopLerpAndSetHueShiftOff() noexcept -> void;

  UTILS::Timer m_lerpOffTimer{m_goomInfo->GetTime(), m_lerpData.minLerpOffTime, false};

  [[nodiscard]] auto CanRestartLerp() const noexcept -> bool;
};

inline auto HueShiftLerper::ChangeHue() noexcept -> void
{
  m_lerpOffTimer.SetToFinished();

  RestartLerpWithNewDestHue();
}

inline auto HueShiftLerper::GetHueShift() const noexcept -> float
{
  return m_currentHueShift;
}

} // namespace GOOM::VISUAL_FX::SHADERS
