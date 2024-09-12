module Goom.FilterFx.GpuFilterEffects.ReflectingPool;

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

static constexpr auto AMPLITUDE_RANGE           = NumberRange{0.05F, 2.5F};
static constexpr auto BASE_RANGE                = NumberRange{0.00F, 0.03F};
static constexpr auto INNER_SIN_FREQUENCY_RANGE = NumberRange{0.005F, 0.05F};
static constexpr auto FREQUENCY_RANGE           = NumberRange{2.0F, 8.0F};
static constexpr auto INNER_POS_FACTOR          = NumberRange{0.1F, 1.5F};

static constexpr auto VIEWPORT_BOUNDS = RandomViewport::Bounds{
    .minSideLength       = 0.1F,
    .probUseCentredSides = 1.0F,
    .rect                = {},
    .sides               = {.minMaxWidth  = {.minValue = 2.0F, .maxValue = 10.0F},
                            .minMaxHeight = {.minValue = 2.0F, .maxValue = 10.0F}}
};

static constexpr auto PROB_XY_AMPLITUDES_EQUAL           = 0.98F;
static constexpr auto PROB_XY_BASES_EQUAL                = 0.98F;
static constexpr auto PROB_XY_INNER_FREQUENCIES_EQUAL    = 0.98F;
static constexpr auto PROB_XY_FREQUENCIES_EQUAL          = 0.98F;
static constexpr auto PROB_XY_INNER_POS_FACTORS_EQUAL    = 0.98F;
static constexpr auto PROB_XY_INNER_POS_FACTORS_NEGATIVE = 0.50F;
static constexpr auto PROB_NO_VIEWPORT                   = 0.5F;

ReflectingPool::ReflectingPool(const GoomRand& goomRand) noexcept
  : m_goomRand{&goomRand},
    m_randomViewport{goomRand, VIEWPORT_BOUNDS},
    m_gpuParams{GetRandomParams()}
{
  m_randomViewport.SetProbNoViewport(PROB_NO_VIEWPORT);
}

auto ReflectingPool::GetRandomParams() const noexcept -> GpuParams
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

  const auto xInnerSinFrequencyFactor = m_goomRand->GetRandInRange<INNER_SIN_FREQUENCY_RANGE>();
  const auto yInnerSinFrequencyFactor =
      m_goomRand->ProbabilityOf<PROB_XY_INNER_FREQUENCIES_EQUAL>()
          ? xInnerSinFrequencyFactor
          : m_goomRand->GetRandInRange<INNER_SIN_FREQUENCY_RANGE>();

  const auto xFreq = m_goomRand->GetRandInRange<FREQUENCY_RANGE>();
  const auto yFreq = m_goomRand->ProbabilityOf<PROB_XY_FREQUENCIES_EQUAL>()
                         ? xFreq
                         : m_goomRand->GetRandInRange<FREQUENCY_RANGE>();

  const auto innerPosFactorSign =
      m_goomRand->ProbabilityOf<PROB_XY_INNER_POS_FACTORS_NEGATIVE>() ? -1.0F : +1.0F;
  const auto xInnerPosFactor = innerPosFactorSign * m_goomRand->GetRandInRange<INNER_POS_FACTOR>();
  const auto yInnerPosFactor =
      m_goomRand->ProbabilityOf<PROB_XY_INNER_POS_FACTORS_EQUAL>()
          ? xInnerPosFactor
          : innerPosFactorSign * m_goomRand->GetRandInRange<INNER_POS_FACTOR>();

  return GpuParams{
      viewport,
      {              xAmplitude,               yAmplitude},
      {                   xBase,                    yBase},
      {xInnerSinFrequencyFactor, yInnerSinFrequencyFactor},
      {                   xFreq,                    yFreq},
      {         xInnerPosFactor,          yInnerPosFactor},
  };
}

auto ReflectingPool::GetGpuZoomFilterEffectNameValueParams() const noexcept -> NameValuePairs
{
  return {};
}

ReflectingPool::GpuParams::GpuParams(const Viewport& viewport,
                                     const Amplitude& amplitude,
                                     const FilterBase& filterBase,
                                     const FrequencyFactor& innerSinFrequencyFactor,
                                     const FrequencyFactor& frequencyFactor,
                                     const FrequencyFactor& innerPosFactor) noexcept
  : m_viewport{viewport},
    m_amplitude{amplitude},
    m_filterBase{filterBase},
    m_innerSinFrequencyFactor{innerSinFrequencyFactor},
    m_frequencyFactor{frequencyFactor},
    m_innerPosFactor{innerPosFactor}
{
}

auto ReflectingPool::GpuParams::OutputGpuParams(const FilterTimingInfo& filterTimingInfo,
                                                const SetterFuncs& setterFuncs) const noexcept
    -> void
{
  setterFuncs.setFloat("u_reflectingPoolStartTime", filterTimingInfo.startTime);
  setterFuncs.setFloat("u_reflectingPoolMaxTime", filterTimingInfo.maxTime);
  setterFuncs.setFloat("u_reflectingPoolXAmplitude", m_amplitude.x);
  setterFuncs.setFloat("u_reflectingPoolYAmplitude", m_amplitude.y);
  setterFuncs.setFloat("u_reflectingPoolXBase", m_filterBase.x);
  setterFuncs.setFloat("u_reflectingPoolYBase", m_filterBase.y);
  setterFuncs.setFloat("u_reflectingPoolXInnerSinFreq", m_innerSinFrequencyFactor.x);
  setterFuncs.setFloat("u_reflectingPoolYInnerSinFreq", m_innerSinFrequencyFactor.y);
  setterFuncs.setFloat("u_reflectingPoolXFreq", m_frequencyFactor.x);
  setterFuncs.setFloat("u_reflectingPoolYFreq", m_frequencyFactor.y);
  setterFuncs.setFloat("u_reflectingPoolInnerPosXFactor", m_innerPosFactor.x);
  setterFuncs.setFloat("u_reflectingPoolInnerPosYFactor", m_innerPosFactor.y);
}

} // namespace GOOM::FILTER_FX::GPU_FILTER_EFFECTS
