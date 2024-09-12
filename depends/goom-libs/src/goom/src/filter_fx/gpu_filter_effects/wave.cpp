module Goom.FilterFx.GpuFilterEffects.Wave;

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

static constexpr auto AMPLITUDE_RANGE       = NumberRange{0.1F, 1.51F};
static constexpr auto BASE_RANGE            = NumberRange{0.1F, 0.3F};
static constexpr auto CYCLE_FREQUENCY_RANGE = NumberRange{1.0F, 3.0F};
static constexpr auto FREQ_FACTOR_RANGE     = NumberRange{1.0F, 50.0F};
static constexpr auto REDUCER_COEFF_RANGE   = NumberRange{0.95F, 1.5F};
static constexpr auto SQ_DIST_POWER_RANGE   = NumberRange{0.15F, 1.1F};

static constexpr auto VIEWPORT_BOUNDS = RandomViewport::Bounds{
    .minSideLength       = 0.1F,
    .probUseCentredSides = 1.0F,
    .rect                = {},
    .sides               = {.minMaxWidth  = {.minValue = 2.0F, .maxValue = 10.0F},
                            .minMaxHeight = {.minValue = 2.0F, .maxValue = 10.0F}}
};

static constexpr auto PROB_XY_AMPLITUDES_EQUAL        = 0.98F;
static constexpr auto PROB_XY_BASES_EQUAL             = 0.98F;
static constexpr auto PROB_XY_CYCLE_FREQUENCIES_EQUAL = 0.98F;
static constexpr auto PROB_XY_FREQUENCIES_EQUAL       = 0.98F;
static constexpr auto PROB_NO_VIEWPORT                = 0.5F;

Wave::Wave(const GoomRand& goomRand) noexcept
  : m_goomRand{&goomRand},
    m_randomViewport{goomRand, VIEWPORT_BOUNDS},
    m_gpuParams{GetRandomParams()}
{
  m_randomViewport.SetProbNoViewport(PROB_NO_VIEWPORT);
}

auto Wave::GetRandomParams() const noexcept -> GpuParams
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

  const auto xCycleFreq = m_goomRand->GetRandInRange<CYCLE_FREQUENCY_RANGE>();
  const auto yCycleFreq = m_goomRand->ProbabilityOf<PROB_XY_CYCLE_FREQUENCIES_EQUAL>()
                              ? xCycleFreq
                              : m_goomRand->GetRandInRange<CYCLE_FREQUENCY_RANGE>();

  const auto xFreq = m_goomRand->GetRandInRange<FREQ_FACTOR_RANGE>();
  const auto yFreq = m_goomRand->ProbabilityOf<PROB_XY_FREQUENCIES_EQUAL>()
                         ? xFreq
                         : m_goomRand->GetRandInRange<FREQ_FACTOR_RANGE>();

  const auto reducerCoeff = m_goomRand->GetRandInRange<REDUCER_COEFF_RANGE>();

  const auto sqDistPower = m_goomRand->GetRandInRange<SQ_DIST_POWER_RANGE>();

  return GpuParams{
      viewport,
      {.x = xAmplitude, .y = yAmplitude},
      {     .x = xBase,      .y = yBase},
      {.x = xCycleFreq, .y = yCycleFreq},
      {     .x = xFreq,      .y = yFreq},
      reducerCoeff,
      sqDistPower
  };
}

auto Wave::GetGpuZoomFilterEffectNameValueParams() const noexcept -> NameValuePairs
{
  return {};
}

Wave::GpuParams::GpuParams(const Viewport& viewport,
                           const Amplitude& amplitude,
                           const FilterBase& filterBase,
                           const FrequencyFactor& cycleFrequency,
                           const FrequencyFactor& frequencyFactor,
                           const float reducerCoeff,
                           const float sqDistPower) noexcept
  : m_viewport{viewport},
    m_amplitude{amplitude},
    m_filterBase{filterBase},
    m_cycleFrequency{cycleFrequency},
    m_frequencyFactor{frequencyFactor},
    m_reducerCoeff{reducerCoeff},
    m_sqDistPower{sqDistPower}
{
}

auto Wave::GpuParams::OutputGpuParams(const FilterTimingInfo& filterTimingInfo,
                                      const SetterFuncs& setterFuncs) const noexcept -> void
{
  setterFuncs.setFloat("u_waveStartTime", filterTimingInfo.startTime);
  setterFuncs.setFloat("u_waveMaxTime", filterTimingInfo.maxTime);
  setterFuncs.setFloat("u_waveXAmplitude", m_amplitude.x);
  setterFuncs.setFloat("u_waveYAmplitude", m_amplitude.y);
  setterFuncs.setFloat("u_waveXBase", m_filterBase.x);
  setterFuncs.setFloat("u_waveYBase", m_filterBase.y);
  setterFuncs.setFloat("u_waveXCycleFreq", m_cycleFrequency.x);
  setterFuncs.setFloat("u_waveYCycleFreq", m_cycleFrequency.y);
  setterFuncs.setFloat("u_waveXFreq", m_frequencyFactor.x);
  setterFuncs.setFloat("u_waveYFreq", m_frequencyFactor.y);
  setterFuncs.setFloat("u_waveReducerCoeff", m_reducerCoeff);
  setterFuncs.setFloat("u_waveSqDistPower", m_sqDistPower);
}

} // namespace GOOM::FILTER_FX::GPU_FILTER_EFFECTS
