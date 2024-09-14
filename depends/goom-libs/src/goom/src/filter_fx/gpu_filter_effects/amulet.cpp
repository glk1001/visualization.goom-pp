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

static constexpr auto AMPLITUDE_RANGE       = NumberRange{1.5F, 4.5F};
static constexpr auto BASE_RANGE            = NumberRange{0.01F, 0.3F};
static constexpr auto CYCLE_FREQUENCY_RANGE = NumberRange{0.50F, 1.0F};
static constexpr auto FREQUENCY_RANGE       = NumberRange{0.001F, 0.03F};

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
static constexpr auto PROB_NEGATIVE_SPIN_SIGN         = 0.50F;
static constexpr auto PROB_NO_VIEWPORT                = 0.5F;

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

  const auto xCycleFreq = m_goomRand->GetRandInRange<CYCLE_FREQUENCY_RANGE>();
  const auto yCycleFreq = m_goomRand->ProbabilityOf<PROB_XY_CYCLE_FREQUENCIES_EQUAL>()
                              ? xCycleFreq
                              : m_goomRand->GetRandInRange<CYCLE_FREQUENCY_RANGE>();

  const auto xFreq = m_goomRand->GetRandInRange<FREQUENCY_RANGE>();
  const auto yFreq = m_goomRand->ProbabilityOf<PROB_XY_FREQUENCIES_EQUAL>()
                         ? xFreq
                         : m_goomRand->GetRandInRange<FREQUENCY_RANGE>();

  const auto amuletSpinSign = m_goomRand->ProbabilityOf(PROB_NEGATIVE_SPIN_SIGN) ? -1.0F : +1.0F;

  return GpuParams{
      viewport,
      {.x = xAmplitude, .y = yAmplitude},
      {     .x = xBase,      .y = yBase},
      {.x = xCycleFreq, .y = yCycleFreq},
      {     .x = xFreq,      .y = yFreq},
      amuletSpinSign,
  };
}

auto Amulet::GetGpuZoomFilterEffectNameValueParams() const noexcept -> NameValuePairs
{
  return {};
}

Amulet::GpuParams::GpuParams(const Viewport& viewport,
                             const Amplitude& amplitude,
                             const FilterBase& filterBase,
                             const FrequencyFactor& cycleFrequency,
                             const FrequencyFactor& frequencyFactor,
                             const float amuletSpinSign) noexcept
  : m_viewport{viewport},
    m_amplitude{amplitude},
    m_filterBase{filterBase},
    m_cycleFrequency{cycleFrequency},
    m_frequencyFactor{frequencyFactor},
    m_amuletSpinSign{amuletSpinSign}
{
}

auto Amulet::GpuParams::OutputGpuParams(const FilterTimingInfo& filterTimingInfo,
                                        const SetterFuncs& setterFuncs) const noexcept -> void
{
  setterFuncs.setFloat("u_amuletStartTime", filterTimingInfo.startTime);
  setterFuncs.setFloat("u_amuletMaxTime", filterTimingInfo.maxTime);
  setterFuncs.setFloat("u_amuletXAmplitude", m_amplitude.x);
  setterFuncs.setFloat("u_amuletYAmplitude", m_amplitude.y);
  setterFuncs.setFloat("u_amuletXBase", m_filterBase.x);
  setterFuncs.setFloat("u_amuletYBase", m_filterBase.y);
  setterFuncs.setFloat("u_amuletXCycleFreq", m_cycleFrequency.x);
  setterFuncs.setFloat("u_amuletYCycleFreq", m_cycleFrequency.y);
  setterFuncs.setFloat("u_amuletXFreq", m_frequencyFactor.x);
  setterFuncs.setFloat("u_amuletYFreq", m_frequencyFactor.y);
  setterFuncs.setFloat("u_amuletSpinSign", m_amuletSpinSign);
}

} // namespace GOOM::FILTER_FX::GPU_FILTER_EFFECTS
