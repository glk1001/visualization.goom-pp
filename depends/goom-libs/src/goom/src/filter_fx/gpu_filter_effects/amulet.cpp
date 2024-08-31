module Goom.FilterFx.GpuFilterEffects.Amulet;

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

static constexpr auto AMPLITUDE_RANGE = NumberRange{0.1F, 1.51F};
static constexpr auto BASE_RANGE      = NumberRange{0.1F, 0.3F};
static constexpr auto FREQUENCY_RANGE = NumberRange{0.01F, 0.1F};

static constexpr auto VIEWPORT_BOUNDS = RandomViewport::Bounds{
    .minSideLength       = 0.1F,
    .probUseCentredSides = 1.0F,
    .rect                = {},
    .sides               = {.minMaxWidth  = {.minValue = 2.0F, .maxValue = 10.0F},
                            .minMaxHeight = {.minValue = 2.0F, .maxValue = 10.0F}}
};

static constexpr auto PROB_XY_AMPLITUDES_EQUAL  = 0.98F;
static constexpr auto PROB_XY_BASES_EQUAL       = 0.98F;
static constexpr auto PROB_XY_FREQUENCIES_EQUAL = 0.98F;
static constexpr auto PROB_NO_VIEWPORT          = 0.5F;

Amulet::Amulet(const GoomRand& goomRand) noexcept
  : m_goomRand{&goomRand},
    m_randomViewport{goomRand, VIEWPORT_BOUNDS},
    m_gpuParams{GetRandomParams()}
{
  m_randomViewport.SetProbNoViewport(PROB_NO_VIEWPORT);
}

auto Amulet::GetRandomParams() const noexcept -> GpuParams
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

  const auto xFreq = m_goomRand->GetRandInRange<FREQUENCY_RANGE>();
  const auto yFreq = m_goomRand->ProbabilityOf<PROB_XY_FREQUENCIES_EQUAL>()
                         ? xFreq
                         : m_goomRand->GetRandInRange<FREQUENCY_RANGE>();

  return GpuParams{
      viewport,
      {xAmplitude, yAmplitude},
      {     xBase,      yBase},
      {     xFreq,      yFreq},
  };
}

auto Amulet::GetGpuZoomFilterEffectNameValueParams() const noexcept -> NameValuePairs
{
  return {};
}

Amulet::GpuParams::GpuParams(const Viewport& viewport,
                             const Amplitude& amplitude,
                             const FilterBase& filterBase,
                             const FrequencyFactor& frequencyFactor) noexcept
  : m_viewport{viewport},
    m_amplitude{amplitude},
    m_filterBase{filterBase},
    m_frequencyFactor{frequencyFactor}
{
}

auto Amulet::GpuParams::OutputGpuParams(const SetterFuncs& setterFuncs) const noexcept -> void
{
  setterFuncs.setFloat("u_amuletXAmplitude", m_amplitude.x);
  setterFuncs.setFloat("u_amuletYAmplitude", m_amplitude.y);
  setterFuncs.setFloat("u_amuletXBase", m_filterBase.x);
  setterFuncs.setFloat("u_amuletYBase", m_filterBase.y);
  setterFuncs.setFloat("u_amuletXFreq", m_frequencyFactor.x);
  setterFuncs.setFloat("u_amuletYFreq", m_frequencyFactor.y);
}

} // namespace GOOM::FILTER_FX::GPU_FILTER_EFFECTS
