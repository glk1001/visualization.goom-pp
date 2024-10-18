module;

#include <format>

module Goom.FilterFx.GpuFilterEffects.BeautifulField;

import Goom.FilterFx.FilterUtils.Utils;
import Goom.FilterFx.CommonTypes;
import Goom.FilterFx.NormalizedCoords;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRand;
import Goom.Lib.GoomTypes;

namespace GOOM::FILTER_FX::GPU_FILTER_EFFECTS
{

using FILTER_UTILS::RandomViewport;
using UTILS::GetFullParamGroup;
using UTILS::GetPair;
using UTILS::NameValuePairs;
using UTILS::MATH::GoomRand;
using UTILS::MATH::NumberRange;

static constexpr auto AMPLITUDE_RANGE       = NumberRange{0.1F, 40.0F};
static constexpr auto BASE_RANGE            = NumberRange{0.00F, 0.03F};
static constexpr auto CYCLE_FREQUENCY_RANGE = NumberRange{0.5F, 1.0F};

enum class FREQUENCY_TYPES : UnderlyingEnumType
{
  SMALL,
  MEDIUM,
  LARGE,
};
static constexpr auto FREQUENCY_RANGE_SMALL  = NumberRange{1.5F, 15.0F};
static constexpr auto FREQUENCY_RANGE_MEDIUM = NumberRange{15.0F, 100.0F};
static constexpr auto FREQUENCY_RANGE_LARGE  = NumberRange{100.0F, 1000.0F};

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
static constexpr auto PROB_XY_FREQUENCIES_EQUAL       = 0.70F;
static constexpr auto PROB_NO_VIEWPORT                = 0.5F;
static constexpr auto PROB_USE_MULTIPLY               = 0.25F;

BeautifulField::BeautifulField(const GoomRand& goomRand) noexcept
  : m_goomRand{&goomRand},
    m_randomViewport{goomRand, VIEWPORT_BOUNDS},
    m_gpuParams{GetRandomParams()}
{
  m_randomViewport.SetProbNoViewport(PROB_NO_VIEWPORT);
}

auto BeautifulField::GetRandomParams() const noexcept -> GpuParams
{
  const auto frequencyFactor = GetRandomFrequencyFactor();

  static auto s_direction = +1.0F;
  s_direction             = -s_direction;

  const auto useMultiply = m_goomRand->ProbabilityOf<PROB_USE_MULTIPLY>();

  return GpuParams{
      m_randomViewport.GetRandomViewport(),
      GetRandomXYPair(*m_goomRand, AMPLITUDE_RANGE, PROB_XY_AMPLITUDES_EQUAL),
      GetRandomXYPair(*m_goomRand, BASE_RANGE, PROB_XY_BASES_EQUAL),
      GetRandomXYPair(*m_goomRand, CYCLE_FREQUENCY_RANGE, PROB_XY_CYCLE_FREQUENCIES_EQUAL),
      frequencyFactor,
      s_direction,
      useMultiply,
  };
}

auto BeautifulField::GetRandomFrequencyFactor() const noexcept -> FrequencyFactor
{
  using enum FREQUENCY_TYPES;

  switch (GetRandomEqualWeighted<FREQUENCY_TYPES>(*m_goomRand))
  {
    case SMALL:
      return GetRandomFrequencyFactor(FREQUENCY_RANGE_SMALL);
    case MEDIUM:
      return GetRandomFrequencyFactor(FREQUENCY_RANGE_MEDIUM);
    case LARGE:
      return GetRandomFrequencyFactor(FREQUENCY_RANGE_LARGE);
  }
}

auto BeautifulField::GetRandomFrequencyFactor(
    const NumberRange<float>& frequencyRange) const noexcept -> FrequencyFactor
{
  const auto xFreq = m_goomRand->GetRandInRange(frequencyRange);
  const auto yFreq = m_goomRand->ProbabilityOf<PROB_XY_FREQUENCIES_EQUAL>()
                         ? xFreq
                         : m_goomRand->GetRandInRange(frequencyRange);
  return {xFreq, yFreq};
}

BeautifulField::GpuParams::GpuParams(const Viewport& viewport,
                                     const Amplitude& amplitude,
                                     const FilterBase& filterBase,
                                     const FrequencyFactor& cycleFrequency,
                                     const FrequencyFactor& frequencyFactor,
                                     const float direction,
                                     const bool useMultiply) noexcept
  : IGpuParams{"beautifulField", viewport, amplitude, filterBase, cycleFrequency},
    m_frequencyFactor{frequencyFactor},
    m_direction{direction},
    m_useMultiply{useMultiply}
{
}

auto BeautifulField::GpuParams::OutputGpuParams(const FilterTimingInfo& filterTimingInfo,
                                                const SetterFuncs& setterFuncs) const noexcept
    -> void
{
  OutputStandardParams(filterTimingInfo, setterFuncs);

  setterFuncs.setFloat("u_beautifulFieldXFreq", m_frequencyFactor.x);
  setterFuncs.setFloat("u_beautifulFieldYFreq", m_frequencyFactor.y);
  setterFuncs.setFloat("u_beautifulFieldDirection", m_direction);
  setterFuncs.setFloat("u_beautifulFieldUseMultiply", m_useMultiply);
}

auto BeautifulField::GetGpuZoomFilterEffectNameValueParams() const noexcept -> NameValuePairs
{
  const auto fullParamGroup = GetFullParamGroup({PARAM_GROUP, "beau"});
  return {GetPair(fullParamGroup,
                  "params",
                  std::format("{}, ({:.2f},{:.2f}), {:.1f}, {}",
                              m_gpuParams.GetFormattedInOrderParams(),
                              m_gpuParams.GetFrequencyFactor().x,
                              m_gpuParams.GetFrequencyFactor().y,
                              m_gpuParams.GetDirection(),
                              m_gpuParams.GetUseMultiply()))};
}

} // namespace GOOM::FILTER_FX::GPU_FILTER_EFFECTS
