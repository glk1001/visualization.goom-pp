#pragma once

#include "goom_plugin_info.h"
#include "utils/math/goom_rand_base.h"
#include "utils/t_values.h"
#include "utils/timer.h"

#include <cstdint>

namespace GOOM::VISUAL_FX::SHADERS
{

class ColorMultiplierLerper
{
public:
  ColorMultiplierLerper(const PluginInfo& goomInfo,
                        const UTILS::MATH::IGoomRand& goomRand,
                        float minColorMultiplier,
                        float maxColorMultiplier) noexcept;

  auto Update() noexcept -> void;
  auto ChangeMultiplierRange() noexcept -> void;

  [[nodiscard]] auto GetColorMultiplier() const noexcept -> float;

private:
  const PluginInfo* m_goomInfo;
  const UTILS::MATH::IGoomRand* m_goomRand;

  static constexpr auto MIN_RANGE = 0.025F;
  float m_minColorMultiplier;
  float m_maxColorMultiplier;
  float m_srceColorMultiplier =
      m_goomRand->GetRandInRange(m_minColorMultiplier, m_maxColorMultiplier);
  float m_destColorMultiplier = GetDestColorMultiplier();
  [[nodiscard]] auto GetDestColorMultiplier() const noexcept -> float;
  float m_currentColorMultiplier = m_srceColorMultiplier;

  static constexpr uint32_t MIN_NUM_LERP_ON_STEPS     = 50U;
  static constexpr uint32_t MAX_NUM_LERP_ON_STEPS     = 500U;
  static constexpr uint32_t DEFAULT_NUM_LERP_ON_STEPS = MIN_NUM_LERP_ON_STEPS;
  UTILS::TValue m_lerpT{
      {UTILS::TValue::StepType::CONTINUOUS_REVERSIBLE, DEFAULT_NUM_LERP_ON_STEPS}
  };

  static constexpr uint32_t MIN_LERP_CONST_TIME     = 10U;
  static constexpr uint32_t MAX_LERP_CONST_TIME     = 50U;
  static constexpr uint32_t DEFAULT_LERP_CONST_TIME = MIN_LERP_CONST_TIME;
  UTILS::Timer m_lerpConstTimer{m_goomInfo->GetTime(), DEFAULT_LERP_CONST_TIME, false};
};

inline auto ColorMultiplierLerper::GetColorMultiplier() const noexcept -> float
{
  return m_currentColorMultiplier;
}

} // namespace GOOM::VISUAL_FX::SHADERS
