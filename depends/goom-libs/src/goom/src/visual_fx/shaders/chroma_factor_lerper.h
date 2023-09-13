#pragma once

#include "goom_plugin_info.h"
#include "utils/math/goom_rand_base.h"
#include "utils/t_values.h"
#include "utils/timer.h"

#include <cstdint>

namespace GOOM::VISUAL_FX::SHADERS
{

class ChromaFactorLerper
{
public:
  ChromaFactorLerper(const PluginInfo& goomInfo,
                     const UTILS::MATH::IGoomRand& goomRand,
                     float minChromaFactor,
                     float maxChromaFactor) noexcept;

  auto Update() noexcept -> void;
  auto ChangeChromaFactorRange() noexcept -> void;

  [[nodiscard]] auto GetChromaFactor() const noexcept -> float;

private:
  [[maybe_unused]] const PluginInfo* m_goomInfo;
  const UTILS::MATH::IGoomRand* m_goomRand;

  static constexpr auto MIN_CHROMA_RANGE = 0.1F;
  float m_minChromaFactor;
  float m_maxChromaFactor;
  float m_srceChromaFactor    = m_goomRand->GetRandInRange(m_minChromaFactor, m_maxChromaFactor);
  float m_destChromaFactor    = GetRandomDestChromaFactor();
  float m_currentChromaFactor = m_srceChromaFactor;
  [[nodiscard]] auto GetRandomDestChromaFactor() const noexcept -> float;

  static constexpr uint32_t MIN_NUM_LERP_ON_STEPS     = 50U;
  static constexpr uint32_t MAX_NUM_LERP_ON_STEPS     = 500U;
  static constexpr uint32_t DEFAULT_NUM_LERP_ON_STEPS = MIN_NUM_LERP_ON_STEPS;
  UTILS::TValue m_lerpT{
      {UTILS::TValue::StepType::CONTINUOUS_REVERSIBLE, DEFAULT_NUM_LERP_ON_STEPS}
  };

  static constexpr uint32_t MIN_LERP_CONST_TIME     = 50U;
  static constexpr uint32_t MAX_LERP_CONST_TIME     = 100U;
  static constexpr uint32_t DEFAULT_LERP_CONST_TIME = MIN_LERP_CONST_TIME;
  UTILS::Timer m_lerpConstTimer{m_goomInfo->GetTime(), DEFAULT_LERP_CONST_TIME, false};
};

inline auto ChromaFactorLerper::GetChromaFactor() const noexcept -> float
{
  return m_currentChromaFactor;
}

} // namespace GOOM::VISUAL_FX::SHADERS
