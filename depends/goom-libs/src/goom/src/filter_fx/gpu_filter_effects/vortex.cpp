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
static constexpr auto CYCLE_FREQUENCY_RANGE = NumberRange{0.1F, 0.5F};
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

static constexpr auto PROB_XY_AMPLITUDES_EQUAL        = 0.98F;
static constexpr auto PROB_XY_BASES_EQUAL             = 0.98F;
static constexpr auto PROB_XY_CYCLE_FREQUENCIES_EQUAL = 0.98F;
static constexpr auto PROB_NEGATIVE_SPIN_SIGN         = 0.50F;
static constexpr auto PROB_NO_VIEWPORT                = 0.5F;

Vortex::Vortex(const GoomRand& goomRand) noexcept
  : m_goomRand{&goomRand},
    m_randomViewport{goomRand, VIEWPORT_BOUNDS},
    m_gpuParams{GetRandomParams()}
{
  m_randomViewport.SetProbNoViewport(PROB_NO_VIEWPORT);
}

auto Vortex::GetRandomParams() const noexcept -> GpuParams
{
  const auto frequencyFactor = m_goomRand->GetRandInRange<FREQUENCY_RANGE>();

  const auto positionFactor = m_goomRand->GetRandInRange<POSITION_FACTOR_RANGE>();
  const auto rFactor        = m_goomRand->GetRandInRange<R_FACTOR_RANGE>();

  const auto vortexSpinSign = m_goomRand->ProbabilityOf(PROB_NEGATIVE_SPIN_SIGN) ? -1.0F : +1.0F;

  return GpuParams{
      m_randomViewport.GetRandomViewport(),
      GetRandomXYPair(*m_goomRand, AMPLITUDE_RANGE, PROB_XY_AMPLITUDES_EQUAL),
      GetRandomXYPair(*m_goomRand, BASE_RANGE, PROB_XY_BASES_EQUAL),
      GetRandomXYPair(*m_goomRand, CYCLE_FREQUENCY_RANGE, PROB_XY_CYCLE_FREQUENCIES_EQUAL),
      frequencyFactor,
      positionFactor,
      rFactor,
      vortexSpinSign,
  };
}

auto Vortex::GetGpuZoomFilterEffectNameValueParams() const noexcept -> NameValuePairs
{
  return {};
}

Vortex::GpuParams::GpuParams(const Viewport& viewport,
                             const Amplitude& amplitude,
                             const FilterBase& filterBase,
                             const FrequencyFactor& cycleFrequency,
                             const float frequencyFactor,
                             const float positionFactor,
                             const float rFactor,
                             const float vortexSpinSign) noexcept
  : IGpuParams{"vortex", viewport, amplitude, filterBase, cycleFrequency},
    m_frequencyFactor{frequencyFactor},
    m_positionFactor{positionFactor},
    m_rFactor{rFactor},
    m_vortexSpinSign{vortexSpinSign}
{
}

auto Vortex::GpuParams::OutputGpuParams(const FilterTimingInfo& filterTimingInfo,
                                        const SetterFuncs& setterFuncs) const noexcept -> void
{
  OutputStandardParams(filterTimingInfo, setterFuncs);

  setterFuncs.setFloat("u_vortexFreq", m_frequencyFactor);
  setterFuncs.setFloat("u_vortexPositionFactor", m_positionFactor);
  setterFuncs.setFloat("u_vortexRFactor", m_rFactor);
  setterFuncs.setFloat("u_vortexSpinSign", m_vortexSpinSign);
}

} // namespace GOOM::FILTER_FX::GPU_FILTER_EFFECTS
