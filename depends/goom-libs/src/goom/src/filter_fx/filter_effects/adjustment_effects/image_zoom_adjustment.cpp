module;

#include <string>

module Goom.FilterFx.FilterEffects.AdjustmentEffects.ImageZoomAdjustment;

import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRand;

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

using GOOM::UTILS::NameValuePairs;
using GOOM::UTILS::MATH::GoomRand;
using UTILS::MATH::NumberRange;

static constexpr auto AMPLITUDE_RANGE    = NumberRange{0.0025F, 0.1000F};
static constexpr auto COLOR_CUTOFF_RANGE = NumberRange{0.1F, 0.9F};
static constexpr auto ZOOM_FACTOR_RANGE  = NumberRange{0.1F, 0.5F};

static constexpr auto PROB_XY_COLOR_CUTOFFS_EQUAL = 0.5F;
static constexpr auto PROB_AMPLITUDES_EQUAL       = 0.5F;
static constexpr auto PROB_ZOOM_FACTORS_EQUAL     = 0.5F;

ImageZoomAdjustment::ImageZoomAdjustment(const std::string& resourcesDirectory,
                                         const GoomRand& goomRand)
  : m_goomRand{&goomRand}, m_imageDisplacementList{resourcesDirectory, *m_goomRand}
{
  if (!resourcesDirectory.empty())
  {
    DoSetRandomParams();
  }
}

auto ImageZoomAdjustment::SetRandomParams() noexcept -> void
{
  DoSetRandomParams();
}

inline auto ImageZoomAdjustment::DoSetRandomParams() noexcept -> void
{
  m_imageDisplacementList.SetRandomImageDisplacement();

  const auto xAmplitude   = m_goomRand->GetRandInRange<AMPLITUDE_RANGE>();
  const auto xColorCutoff = m_goomRand->GetRandInRange<COLOR_CUTOFF_RANGE>();
  const auto xZoomFactor  = m_goomRand->GetRandInRange<ZOOM_FACTOR_RANGE>();

  m_imageDisplacementList.SetParams({
      .amplitude    = {.x = xAmplitude,
                       .y = m_goomRand->ProbabilityOf<PROB_AMPLITUDES_EQUAL>()
                                ? xAmplitude
                                : m_goomRand->GetRandInRange<AMPLITUDE_RANGE>()},
      .xColorCutoff = xColorCutoff,
      .yColorCutoff = m_goomRand->ProbabilityOf<PROB_XY_COLOR_CUTOFFS_EQUAL>()
                          ? xColorCutoff
                          : m_goomRand->GetRandInRange<COLOR_CUTOFF_RANGE>(),
      .xZoomFactor  = xZoomFactor,
      .yZoomFactor  = m_goomRand->ProbabilityOf<PROB_ZOOM_FACTORS_EQUAL>()
                          ? xZoomFactor
                          : m_goomRand->GetRandInRange<ZOOM_FACTOR_RANGE>()
  });
}

auto ImageZoomAdjustment::GetZoomAdjustmentEffectNameValueParams() const noexcept -> NameValuePairs
{
  static constexpr auto* IMAGE_PARAM_GROUP = "ImageZoomAdjustment";
  return m_imageDisplacementList.GetNameValueParams(IMAGE_PARAM_GROUP);
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
