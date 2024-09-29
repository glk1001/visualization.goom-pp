module Goom.FilterFx.GpuFilterEffects.UpDown;

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

static constexpr auto AMPLITUDE_RANGE        = NumberRange{0.01F, 5.0F};
static constexpr auto BASE_RANGE             = NumberRange{0.0F, 0.3F};
static constexpr auto CYCLE_FREQUENCY_RANGE  = NumberRange{0.01F, 1.5F};
static constexpr auto FREQUENCY_RANGE        = NumberRange{1.0F, 100.0F};
static constexpr auto ROTATE_FREQUENCY_RANGE = NumberRange{0.1F, 5.0F};
static constexpr auto MIX_FREQUENCY_RANGE    = NumberRange{0.1F, 3.0F};

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

UpDown::UpDown(const GoomRand& goomRand) noexcept
  : m_goomRand{&goomRand},
    m_randomViewport{goomRand, VIEWPORT_BOUNDS},
    m_gpuParams{GetRandomParams()}
{
  m_randomViewport.SetProbNoViewport(PROB_NO_VIEWPORT);
}

auto UpDown::GetRandomParams() const noexcept -> GpuParams
{
  const auto xFreq = m_goomRand->GetRandInRange<FREQUENCY_RANGE>();
  const auto yFreq = m_goomRand->ProbabilityOf<PROB_XY_FREQUENCIES_EQUAL>()
                         ? xFreq
                         : m_goomRand->GetRandInRange<FREQUENCY_RANGE>();

  const auto rotateFrequencyFactor = m_goomRand->GetRandInRange<ROTATE_FREQUENCY_RANGE>();

  const auto mixFrequencyFactor = m_goomRand->GetRandInRange<MIX_FREQUENCY_RANGE>();

  return GpuParams{
      m_randomViewport.GetRandomViewport(),
      GetRandomXYPair(*m_goomRand, AMPLITUDE_RANGE, PROB_XY_AMPLITUDES_EQUAL),
      GetRandomXYPair(*m_goomRand, BASE_RANGE, PROB_XY_BASES_EQUAL),
      GetRandomXYPair(*m_goomRand, CYCLE_FREQUENCY_RANGE, PROB_XY_CYCLE_FREQUENCIES_EQUAL),
      {.x = xFreq, .y = yFreq},
      rotateFrequencyFactor,
      mixFrequencyFactor,
  };
}

auto UpDown::GetGpuZoomFilterEffectNameValueParams() const noexcept -> NameValuePairs
{
  return {};
}

UpDown::GpuParams::GpuParams(const Viewport& viewport,
                             const Amplitude& amplitude,
                             const FilterBase& filterBase,
                             const FrequencyFactor& cycleFrequency,
                             const FrequencyFactor& frequencyFactor,
                             const float rotateFrequencyFactor,
                             const float mixFrequencyFactor) noexcept
  : IGpuParams{"upDown", viewport, amplitude, filterBase, cycleFrequency},
    m_frequencyFactor{frequencyFactor},
    m_rotateFrequencyFactor{rotateFrequencyFactor},
    m_mixFrequencyFactor{mixFrequencyFactor}
{
}

auto UpDown::GpuParams::OutputGpuParams(const FilterTimingInfo& filterTimingInfo,
                                        const SetterFuncs& setterFuncs) const noexcept -> void
{
  OutputStandardParams(filterTimingInfo, setterFuncs);

  setterFuncs.setFloat("u_upDownXFreq", m_frequencyFactor.x);
  setterFuncs.setFloat("u_upDownYFreq", m_frequencyFactor.y);
  setterFuncs.setFloat("u_upDownRotateFreq", m_rotateFrequencyFactor);
  setterFuncs.setFloat("u_upDownMixFreq", m_mixFrequencyFactor);
}

} // namespace GOOM::FILTER_FX::GPU_FILTER_EFFECTS
