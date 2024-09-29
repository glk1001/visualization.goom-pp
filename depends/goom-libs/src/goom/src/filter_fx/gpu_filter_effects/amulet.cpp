module Goom.FilterFx.GpuFilterEffects.Amulet;

import Goom.FilterFx.FilterUtils.Utils;
import Goom.FilterFx.CommonTypes;
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
  const auto xFreq = m_goomRand->GetRandInRange<FREQUENCY_RANGE>();
  const auto yFreq = m_goomRand->ProbabilityOf<PROB_XY_FREQUENCIES_EQUAL>()
                         ? xFreq
                         : m_goomRand->GetRandInRange<FREQUENCY_RANGE>();

  const auto amuletSpinSign = m_goomRand->ProbabilityOf(PROB_NEGATIVE_SPIN_SIGN) ? -1.0F : +1.0F;

  return GpuParams{
      m_randomViewport.GetRandomViewport(),
      GetRandomXYPair(*m_goomRand, AMPLITUDE_RANGE, PROB_XY_AMPLITUDES_EQUAL),
      GetRandomXYPair(*m_goomRand, BASE_RANGE, PROB_XY_BASES_EQUAL),
      GetRandomXYPair(*m_goomRand, CYCLE_FREQUENCY_RANGE, PROB_XY_CYCLE_FREQUENCIES_EQUAL),
      {.x = xFreq, .y = yFreq},
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
  : IGpuParams{"amulet", viewport, amplitude, filterBase, cycleFrequency},
    m_frequencyFactor{frequencyFactor},
    m_amuletSpinSign{amuletSpinSign}
{
}

auto Amulet::GpuParams::OutputGpuParams(const FilterTimingInfo& filterTimingInfo,
                                        const SetterFuncs& setterFuncs) const noexcept -> void
{
  OutputStandardParams(filterTimingInfo, setterFuncs);

  setterFuncs.setFloat("u_amuletXFreq", m_frequencyFactor.x);
  setterFuncs.setFloat("u_amuletYFreq", m_frequencyFactor.y);
  setterFuncs.setFloat("u_amuletSpinSign", m_amuletSpinSign);
}

} // namespace GOOM::FILTER_FX::GPU_FILTER_EFFECTS
