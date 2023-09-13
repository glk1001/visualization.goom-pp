#include "chroma_factor_lerper.h"

#include "goom/goom_config.h"
#include "goom/math20.h"
#include "goom_plugin_info.h"
#include "utils/math/goom_rand_base.h"

#include <cmath>

namespace GOOM::VISUAL_FX::SHADERS
{

ChromaFactorLerper::ChromaFactorLerper(const PluginInfo& goomInfo,
                                       const UTILS::MATH::IGoomRand& goomRand,
                                       // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
                                       const float minChromaFactor,
                                       const float maxChromaFactor) noexcept
  : m_goomInfo{&goomInfo},
    m_goomRand{&goomRand},
    m_minChromaFactor{minChromaFactor},
    m_maxChromaFactor{maxChromaFactor}
{
  Expects(minChromaFactor < maxChromaFactor);
  Expects(std::fabs(minChromaFactor - maxChromaFactor) >= MIN_CHROMA_RANGE);
}

auto ChromaFactorLerper::Update() noexcept -> void
{
  if (not m_lerpConstTimer.Finished())
  {
    return;
  }

  m_currentChromaFactor = STD20::lerp(m_srceChromaFactor, m_destChromaFactor, m_lerpT());

  m_lerpConstTimer.ResetToZero();
  m_lerpT.Increment();
}

auto ChromaFactorLerper::ChangeChromaFactorRange() noexcept -> void
{
  m_srceChromaFactor = m_currentChromaFactor;
  m_destChromaFactor = GetRandomDestChromaFactor();
  m_lerpT.Reset(0.0F);

  m_lerpT.SetNumSteps(m_goomRand->GetRandInRange(MIN_NUM_LERP_ON_STEPS, MAX_NUM_LERP_ON_STEPS));
  m_lerpConstTimer.SetTimeLimit(
      m_goomRand->GetRandInRange(MIN_LERP_CONST_TIME, MAX_LERP_CONST_TIME));
}

auto ChromaFactorLerper::GetRandomDestChromaFactor() const noexcept -> float
{
  static constexpr auto MAX_LOOPS = 10U;

  for (auto i = 0U; i < MAX_LOOPS; ++i)
  {
    const auto destChromaFactor = m_goomRand->GetRandInRange(m_minChromaFactor, m_maxChromaFactor);
    if (std::fabs(m_srceChromaFactor - destChromaFactor) >= MIN_CHROMA_RANGE)
    {
      return destChromaFactor;
    }
  }

  return m_goomRand->GetRandInRange(m_minChromaFactor, m_maxChromaFactor);
}

} // namespace GOOM::VISUAL_FX::SHADERS
