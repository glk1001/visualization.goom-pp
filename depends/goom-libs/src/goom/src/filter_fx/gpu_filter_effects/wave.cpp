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

static constexpr auto AMPLITUDE_RANGE         = NumberRange{0.1F, 2.0F};
static constexpr auto BASE_RANGE              = NumberRange{0.0F, 0.3F};
static constexpr auto CYCLE_FREQUENCY_RANGE   = NumberRange{1.1F, 1.1F};
static constexpr auto FREQ_FACTOR_RANGE       = NumberRange{1.0F, 50.0F};
static constexpr auto REDUCER_COEFF_RANGE     = NumberRange{0.1F, 10.0F};
static constexpr auto SQ_DIST_POWER_RANGE     = NumberRange{0.1F, 1.2F};
static constexpr auto PROB_NEGATIVE_SPIN_SIGN = 0.50F;

static constexpr auto VIEWPORT_BOUNDS = RandomViewport::Bounds{
    .minSideLength       = 0.1F,
    .probUseCentredSides = 1.0F,
    .rect                = {},
    .sides               = {.minMaxWidth  = {.minValue = 2.0F, .maxValue = 10.0F},
                            .minMaxHeight = {.minValue = 2.0F, .maxValue = 10.0F}}
};

static constexpr auto PROB_XY_AMPLITUDES_EQUAL        = 1.0F;
static constexpr auto PROB_XY_BASES_EQUAL             = 1.0F;
static constexpr auto PROB_XY_CYCLE_FREQUENCIES_EQUAL = 1.0F;
static constexpr auto PROB_XY_FREQUENCIES_EQUAL       = 1.0F;
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
  const auto xFreq = m_goomRand->GetRandInRange<FREQ_FACTOR_RANGE>();
  const auto yFreq = m_goomRand->ProbabilityOf<PROB_XY_FREQUENCIES_EQUAL>()
                         ? xFreq
                         : m_goomRand->GetRandInRange<FREQ_FACTOR_RANGE>();

  const auto reducerCoeff = m_goomRand->GetRandInRange<REDUCER_COEFF_RANGE>();

  const auto sqDistPower = m_goomRand->GetRandInRange<SQ_DIST_POWER_RANGE>();

  const auto waveSpinSign = m_goomRand->ProbabilityOf(PROB_NEGATIVE_SPIN_SIGN) ? -1.0F : +1.0F;

  return GpuParams{
      m_randomViewport.GetRandomViewport(),
      GetRandomXYPair(*m_goomRand, AMPLITUDE_RANGE, PROB_XY_AMPLITUDES_EQUAL),
      GetRandomXYPair(*m_goomRand, BASE_RANGE, PROB_XY_BASES_EQUAL),
      GetRandomXYPair(*m_goomRand, CYCLE_FREQUENCY_RANGE, PROB_XY_CYCLE_FREQUENCIES_EQUAL),
      {.x = xFreq, .y = yFreq},
      reducerCoeff,
      sqDistPower,
      waveSpinSign
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
                           const float sqDistPower,
                           const float waveSpinSign) noexcept
  : IGpuParams{"wave", viewport, amplitude, filterBase, cycleFrequency},
    m_frequencyFactor{frequencyFactor},
    m_reducerCoeff{reducerCoeff},
    m_sqDistPower{sqDistPower},
    m_waveSpinSign{waveSpinSign}
{
}

auto Wave::GpuParams::OutputGpuParams(const FilterTimingInfo& filterTimingInfo,
                                      const SetterFuncs& setterFuncs) const noexcept -> void
{
  OutputStandardParams(filterTimingInfo, setterFuncs);

  setterFuncs.setFloat("u_waveXFreq", m_frequencyFactor.x);
  setterFuncs.setFloat("u_waveYFreq", m_frequencyFactor.y);
  setterFuncs.setFloat("u_waveReducerCoeff", m_reducerCoeff);
  setterFuncs.setFloat("u_waveSqDistPower", m_sqDistPower);
  setterFuncs.setFloat("u_waveSpinSign", m_waveSpinSign);
}

} // namespace GOOM::FILTER_FX::GPU_FILTER_EFFECTS
