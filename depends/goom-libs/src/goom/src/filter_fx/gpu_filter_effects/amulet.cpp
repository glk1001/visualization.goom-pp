module;

#include <format>

module Goom.FilterFx.GpuFilterEffects.Amulet;

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

static constexpr auto AMPLITUDE_RANGE       = NumberRange{0.015F, 1.5F};
static constexpr auto BASE_RANGE            = NumberRange{0.0F, 0.3F};
static constexpr auto CYCLE_FREQUENCY_RANGE = NumberRange{1.0F, 3.0F};

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
  const auto amuletSpinSign = m_goomRand->ProbabilityOf(PROB_NEGATIVE_SPIN_SIGN) ? -1.0F : +1.0F;

  return GpuParams{
      m_randomViewport.GetRandomViewport(),
      GetRandomXYPair(*m_goomRand, AMPLITUDE_RANGE, PROB_XY_AMPLITUDES_EQUAL),
      GetRandomXYPair(*m_goomRand, BASE_RANGE, PROB_XY_BASES_EQUAL),
      GetRandomXYPair(*m_goomRand, CYCLE_FREQUENCY_RANGE, PROB_XY_CYCLE_FREQUENCIES_EQUAL),
      amuletSpinSign,
  };
}

Amulet::GpuParams::GpuParams(const Viewport& viewport,
                             const Amplitude& amplitude,
                             const FilterBase& filterBase,
                             const FrequencyFactor& cycleFrequency,
                             const float amuletSpinSign) noexcept
  : IGpuParams{"amulet", viewport, amplitude, filterBase, cycleFrequency},
    m_amuletSpinSign{amuletSpinSign}
{
}

auto Amulet::GpuParams::OutputGpuParams(const FilterTimingInfo& filterTimingInfo,
                                        const SetterFuncs& setterFuncs) const noexcept -> void
{
  OutputStandardParams(filterTimingInfo, setterFuncs);

  setterFuncs.setFloat("u_amuletSpinSign", m_amuletSpinSign);
}

auto Amulet::GetGpuZoomFilterEffectNameValueParams() const noexcept -> NameValuePairs
{
  const auto fullParamGroup = GetFullParamGroup({PARAM_GROUP, "amulet"});
  return {GetPair(fullParamGroup,
                  "params",
                  std::format("{}, {:.2f}",
                              m_gpuParams.GetFormattedInOrderParams(),
                              m_gpuParams.GetAmuletSpinSign()))};
}

} // namespace GOOM::FILTER_FX::GPU_FILTER_EFFECTS
