module Goom.FilterFx.GpuFilterEffects.Vortex;

import Goom.FilterFx.FilterUtils.Utils;
import Goom.FilterFx.NormalizedCoords;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRand;

namespace GOOM::FILTER_FX::GPU_FILTER_EFFECTS
{

using FILTER_UTILS::RandomViewport;
using UTILS::NameValuePairs;
using UTILS::MATH::GoomRand;
using UTILS::MATH::NumberRange;

static constexpr auto AMPLITUDE_RANGE       = NumberRange{1.0F, 3.0F};
static constexpr auto BASE_RANGE            = NumberRange{0.00F, 0.03F};
static constexpr auto FREQUENCY_RANGE       = NumberRange{0.005F, 0.030F};
static constexpr auto POSITION_FACTOR_RANGE = NumberRange{0.1F, 0.2F};
static constexpr auto R_FACTOR_RANGE        = NumberRange{5.0F, 15.0F};

static constexpr auto VIEWPORT_BOUNDS = RandomViewport::Bounds{
    .minSideLength       = 0.1F,
    .probUseCentredSides = 1.0F,
    .rect                = {},
    .sides               = {.minMaxWidth  = {.minValue = 2.0F, .maxValue = 10.0F},
                            .minMaxHeight = {.minValue = 2.0F, .maxValue = 10.0F}}
};

static constexpr auto PROB_XY_AMPLITUDES_EQUAL = 0.98F;
static constexpr auto PROB_XY_BASES_EQUAL      = 0.98F;
static constexpr auto PROB_NO_VIEWPORT         = 0.5F;

Vortex::Vortex(const GoomRand& goomRand) noexcept
  : m_goomRand{&goomRand},
    m_randomViewport{goomRand, VIEWPORT_BOUNDS},
    m_gpuParams{GetRandomParams()}
{
  m_randomViewport.SetProbNoViewport(PROB_NO_VIEWPORT);
}

auto Vortex::GetRandomParams() const noexcept -> GpuParams
{
  const auto viewport = m_randomViewport.GetRandomViewport();

  const auto xAmplitude = m_goomRand->GetRandInRange<AMPLITUDE_RANGE>();
  const auto yAmplitude = m_goomRand->ProbabilityOf<PROB_XY_AMPLITUDES_EQUAL>()
                              ? xAmplitude
                              : m_goomRand->GetRandInRange<AMPLITUDE_RANGE>();

  const auto xBase = m_goomRand->GetRandInRange<BASE_RANGE>();
  const auto yBase = m_goomRand->ProbabilityOf<PROB_XY_BASES_EQUAL>()
                         ? xBase
                         : m_goomRand->GetRandInRange<BASE_RANGE>();

  const auto frequencyFactor = m_goomRand->GetRandInRange<FREQUENCY_RANGE>();

  const auto positionFactor = m_goomRand->GetRandInRange<POSITION_FACTOR_RANGE>();
  const auto rFactor        = m_goomRand->GetRandInRange<R_FACTOR_RANGE>();

  return GpuParams{
      viewport,
      {xAmplitude, yAmplitude},
      {     xBase,      yBase},
      frequencyFactor,
      positionFactor,
      rFactor,
  };
}

auto Vortex::GetGpuZoomFilterEffectNameValueParams() const noexcept -> NameValuePairs
{
  return {};
}

Vortex::GpuParams::GpuParams(const Viewport& viewport,
                             const Amplitude& amplitude,
                             const FilterBase& filterBase,
                             const float frequencyFactor,
                             const float positionFactor,
                             const float rFactor) noexcept
  : m_viewport{viewport},
    m_amplitude{amplitude},
    m_filterBase{filterBase},
    m_frequencyFactor{frequencyFactor},
    m_positionFactor{positionFactor},
    m_rFactor{rFactor}
{
}

auto Vortex::GpuParams::OutputGpuParams(const SetterFuncs& setterFuncs) const noexcept -> void
{
  setterFuncs.setFloat("u_vortexXAmplitude", m_amplitude.x);
  setterFuncs.setFloat("u_vortexYAmplitude", m_amplitude.y);
  setterFuncs.setFloat("u_vortexXBase", m_filterBase.x);
  setterFuncs.setFloat("u_vortexYBase", m_filterBase.y);
  setterFuncs.setFloat("u_vortexFreq", m_frequencyFactor);
  setterFuncs.setFloat("u_vortexPositionFactor", m_positionFactor);
  setterFuncs.setFloat("u_vortexRFactor", m_rFactor);
}

} // namespace GOOM::FILTER_FX::GPU_FILTER_EFFECTS
