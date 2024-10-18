module;

#include <format>

module Goom.FilterFx.GpuFilterEffects.ReflectingPool;

import Goom.FilterFx.FilterUtils.Utils;
import Goom.FilterFx.CommonTypes;
import Goom.FilterFx.NormalizedCoords;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRand;

namespace GOOM::FILTER_FX::GPU_FILTER_EFFECTS
{

using FILTER_UTILS::RandomViewport;
using UTILS::GetFullParamGroup;
using UTILS::GetPair;
using UTILS::NameValuePairs;
using UTILS::MATH::GoomRand;
using UTILS::MATH::NumberRange;

static constexpr auto AMPLITUDE_RANGE1       = NumberRange{0.05F, 1.5F};
static constexpr auto BASE_RANGE1            = NumberRange{0.00F, 0.03F};
static constexpr auto CYCLE_FREQUENCY_RANGE1 = NumberRange{0.5F, 1.0F};
static constexpr auto FREQUENCY_RANGE1       = NumberRange{1.0F, 3.0F};
static constexpr auto INNER_POS_FACTOR1      = NumberRange{0.5F, 2.0F};
static constexpr auto AMPLITUDE_RANGE2       = NumberRange{1.5F, 12.5F};
static constexpr auto BASE_RANGE2            = NumberRange{0.00F, 0.03F};
static constexpr auto CYCLE_FREQUENCY_RANGE2 = NumberRange{0.5F, 1.0F};
static constexpr auto FREQUENCY_RANGE2       = NumberRange{3.0F, 10.0F};
static constexpr auto INNER_POS_FACTOR2      = NumberRange{2.0F, 10.0F};

static constexpr auto VIEWPORT_BOUNDS = RandomViewport::Bounds{
    .minSideLength       = 0.1F,
    .probUseCentredSides = 1.0F,
    .rect                = {},
    .sides               = {.minMaxWidth  = {.minValue = 2.0F, .maxValue = 10.0F},
                            .minMaxHeight = {.minValue = 2.0F, .maxValue = 10.0F}}
};

static constexpr auto PROB_RANGE1                        = 0.5F;
static constexpr auto PROB_XY_AMPLITUDES_EQUAL           = 0.98F;
static constexpr auto PROB_XY_BASES_EQUAL                = 0.98F;
static constexpr auto PROB_XY_CYCLE_FREQUENCIES_EQUAL    = 0.98F;
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
  const auto probRange1 = m_goomRand->ProbabilityOf(PROB_RANGE1);

  const auto xFreq = m_goomRand->GetRandInRange(probRange1 ? FREQUENCY_RANGE1 : FREQUENCY_RANGE2);
  const auto yFreq =
      m_goomRand->ProbabilityOf<PROB_XY_FREQUENCIES_EQUAL>()
          ? xFreq
          : m_goomRand->GetRandInRange(probRange1 ? FREQUENCY_RANGE1 : FREQUENCY_RANGE2);

  const auto innerPosFactorSign =
      m_goomRand->ProbabilityOf<PROB_XY_INNER_POS_FACTORS_NEGATIVE>() ? -1.0F : +1.0F;
  const auto xInnerPosFactor =
      innerPosFactorSign *
      m_goomRand->GetRandInRange(probRange1 ? INNER_POS_FACTOR1 : INNER_POS_FACTOR2);
  const auto yInnerPosFactor =
      m_goomRand->ProbabilityOf<PROB_XY_INNER_POS_FACTORS_EQUAL>()
          ? xInnerPosFactor
          : innerPosFactorSign *
                m_goomRand->GetRandInRange(probRange1 ? INNER_POS_FACTOR1 : INNER_POS_FACTOR2);

  return GpuParams{
      m_randomViewport.GetRandomViewport(),
      GetRandomXYPair(
          *m_goomRand, probRange1 ? AMPLITUDE_RANGE1 : AMPLITUDE_RANGE2, PROB_XY_AMPLITUDES_EQUAL),
      GetRandomXYPair(*m_goomRand, probRange1 ? BASE_RANGE1 : BASE_RANGE2, PROB_XY_BASES_EQUAL),
      GetRandomXYPair(*m_goomRand,
                      probRange1 ? CYCLE_FREQUENCY_RANGE1 : CYCLE_FREQUENCY_RANGE2,
                      PROB_XY_CYCLE_FREQUENCIES_EQUAL),
      {          .x = xFreq,           .y = yFreq},
      {.x = xInnerPosFactor, .y = yInnerPosFactor},
  };
}

ReflectingPool::GpuParams::GpuParams(const Viewport& viewport,
                                     const Amplitude& amplitude,
                                     const FilterBase& filterBase,
                                     const FrequencyFactor& cycleFrequency,
                                     const FrequencyFactor& frequencyFactor,
                                     const FrequencyFactor& innerPosFactor) noexcept
  : IGpuParams{"reflectingPool", viewport, amplitude, filterBase, cycleFrequency},
    m_frequencyFactor{frequencyFactor},
    m_innerPosFactor{innerPosFactor}
{
}

auto ReflectingPool::GpuParams::OutputGpuParams(const FilterTimingInfo& filterTimingInfo,
                                                const SetterFuncs& setterFuncs) const noexcept
    -> void
{
  OutputStandardParams(filterTimingInfo, setterFuncs);

  setterFuncs.setFloat("u_reflectingPoolXFreq", m_frequencyFactor.x);
  setterFuncs.setFloat("u_reflectingPoolYFreq", m_frequencyFactor.y);
  setterFuncs.setFloat("u_reflectingPoolInnerPosXFactor", m_innerPosFactor.x);
  setterFuncs.setFloat("u_reflectingPoolInnerPosYFactor", m_innerPosFactor.y);
}

auto ReflectingPool::GetGpuZoomFilterEffectNameValueParams() const noexcept -> NameValuePairs
{
  const auto fullParamGroup = GetFullParamGroup({PARAM_GROUP, "pool"});
  return {GetPair(fullParamGroup,
                  "params",
                  std::format("{}, ({:.2f},{:.2f}), ({:.2f},{:.2f})",
                              m_gpuParams.GetFormattedInOrderParams(),
                              m_gpuParams.GetFrequencyFactor().x,
                              m_gpuParams.GetFrequencyFactor().y,
                              m_gpuParams.GetInnerPosFactor().x,
                              m_gpuParams.GetInnerPosFactor().y))};
}

} // namespace GOOM::FILTER_FX::GPU_FILTER_EFFECTS
