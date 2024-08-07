module;

#include <cmath>
#include <cstdint>

module Goom.FilterFx.FilterEffects.AdjustmentEffects.YOnly;

import Goom.FilterFx.CommonTypes;
import Goom.FilterFx.NormalizedCoords;
import Goom.Utils.EnumUtils;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRand;
import Goom.Lib.AssertUtils;

namespace GOOM::FILTER_FX::FILTER_EFFECTS
{

using UTILS::NameValuePairs;
using UTILS::NUM;
using UTILS::MATH::GoomRand;
using UTILS::MATH::NumberRange;

static constexpr auto X_DEFAULT_EFFECT = YOnly::YOnlyEffect::X_SIN_Y_SIN;
static constexpr auto Y_DEFAULT_EFFECT = YOnly::YOnlyEffect::NONE;

static constexpr auto X_DEFAULT_FREQ_FACTOR = 1.0F;
static constexpr auto Y_DEFAULT_FREQ_FACTOR = 10.0F;
static constexpr auto FREQ_FACTOR_RANGE     = FrequencyFactorRange{
        {-50.0F, +50.01F},
        {-50.0F, +50.01F},
};

static constexpr auto X_DEFAULT_AMPLITUDE = 10.0F;
static constexpr auto Y_DEFAULT_AMPLITUDE = 10.0F;
static constexpr auto AMPLITUDE_RANGE     = AmplitudeRange{
        {0.010F, 1.011F},
        {0.010F, 1.011F},
};

static constexpr auto PROB_Y_ONLY_STRICT   = 0.9F;
static constexpr auto PROB_FREQ_EQUAL      = 0.9F;
static constexpr auto PROB_AMPLITUDE_EQUAL = 0.9F;

YOnly::YOnly(const GoomRand& goomRand) noexcept
  : m_goomRand{&goomRand},
    m_params{
        {X_DEFAULT_EFFECT,      Y_DEFAULT_EFFECT},
        {X_DEFAULT_FREQ_FACTOR, Y_DEFAULT_FREQ_FACTOR},
        {X_DEFAULT_AMPLITUDE,   Y_DEFAULT_AMPLITUDE}
    }
{
}

auto YOnly::SetRandomParams() noexcept -> void
{
  static constexpr auto Y_ONLY_EFFECT_RANGE =
      NumberRange{static_cast<uint32_t>(YOnlyEffect::NONE) + 1, NUM<YOnlyEffect> - 1};
  const auto xEffect = static_cast<YOnlyEffect>(m_goomRand->GetRandInRange<Y_ONLY_EFFECT_RANGE>());
  const auto yEffect =
      m_goomRand->ProbabilityOf<PROB_Y_ONLY_STRICT>()
          ? YOnlyEffect::NONE
          : static_cast<YOnlyEffect>(m_goomRand->GetRandInRange<Y_ONLY_EFFECT_RANGE>());

  const auto xFreqFactor = m_goomRand->GetRandInRange<FREQ_FACTOR_RANGE.xRange>();
  const auto yFreqFactor = m_goomRand->ProbabilityOf<PROB_FREQ_EQUAL>()
                               ? xFreqFactor
                               : m_goomRand->GetRandInRange<FREQ_FACTOR_RANGE.yRange>();

  const auto xAmplitude = m_goomRand->GetRandInRange<AMPLITUDE_RANGE.xRange>();
  const auto yAmplitude = m_goomRand->ProbabilityOf<PROB_AMPLITUDE_EQUAL>()
                              ? xAmplitude
                              : m_goomRand->GetRandInRange<AMPLITUDE_RANGE.yRange>();

  SetParams({
      {    xEffect,     yEffect},
      {xFreqFactor, yFreqFactor},
      { xAmplitude,  yAmplitude}
  });
}

auto YOnly::GetYOnlyZoomAdjustmentMultiplier(const YOnlyEffect effect,
                                             const NormalizedCoords& coords) const noexcept -> float
{
  switch (effect)
  {
    case YOnlyEffect::X_SIN_Y_SIN:
      return std::sin(m_params.frequencyFactor.x * coords.GetX()) *
             std::sin(m_params.frequencyFactor.y * coords.GetY());
    case YOnlyEffect::X_SIN_Y_COS:
      return std::sin(m_params.frequencyFactor.x * coords.GetX()) *
             std::cos(m_params.frequencyFactor.y * coords.GetY());
    case YOnlyEffect::X_COS_Y_SIN:
      return std::cos(m_params.frequencyFactor.x * coords.GetX()) *
             std::sin(m_params.frequencyFactor.y * coords.GetY());
    case YOnlyEffect::X_COS_Y_COS:
      return std::cos(m_params.frequencyFactor.x * coords.GetX()) *
             std::cos(m_params.frequencyFactor.y * coords.GetY());
    default:
      FailFast();
  }
}

auto YOnly::GetZoomAdjustmentEffectNameValueParams() const noexcept -> NameValuePairs
{
  return NameValuePairs();
}

} // namespace GOOM::FILTER_FX::FILTER_EFFECTS
