export module Goom.FilterFx.GpuFilterEffects.BeautifulField;

import Goom.FilterFx.GpuFilterEffects.GpuZoomFilterEffect;
import Goom.FilterFx.FilterUtils.Utils;
import Goom.FilterFx.CommonTypes;
import Goom.FilterFx.NormalizedCoords;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRand;

using GOOM::FILTER_FX::FILTER_UTILS::RandomViewport;
using GOOM::UTILS::NameValuePairs;
using GOOM::UTILS::MATH::GoomRand;
using GOOM::UTILS::MATH::NumberRange;

export namespace GOOM::FILTER_FX::GPU_FILTER_EFFECTS
{

class BeautifulField : public IGpuZoomFilterEffect
{
public:
  explicit BeautifulField(const GoomRand& goomRand) noexcept;

  auto SetRandomParams() noexcept -> void override;

  [[nodiscard]] auto GetGpuParams() const noexcept -> const IGpuParams& override;

  [[nodiscard]] auto GetGpuZoomFilterEffectNameValueParams() const noexcept
      -> NameValuePairs override;

  class GpuParams : public IGpuParams
  {
  public:
    GpuParams(const Viewport& viewport,
              const Amplitude& amplitude,
              const FilterBase& filterBase,
              const FrequencyFactor& cycleFrequency,
              const FrequencyFactor& frequencyFactor,
              float direction,
              bool useMultiply) noexcept;

    auto OutputGpuParams(const FilterTimingInfo& filterTimingInfo,
                         const SetterFuncs& setterFuncs) const noexcept -> void override;

  private:
    FrequencyFactor m_frequencyFactor{};
    float m_direction{};
    bool m_useMultiply{};
  };

private:
  const GoomRand* m_goomRand;
  RandomViewport m_randomViewport;
  GpuParams m_gpuParams;
  [[nodiscard]] auto GetRandomParams() const noexcept -> GpuParams;
  [[nodiscard]] auto GetRandomFrequencyFactor() const noexcept -> FrequencyFactor;
  [[nodiscard]] auto GetRandomFrequencyFactor(
      const NumberRange<float>& frequencyRange) const noexcept -> FrequencyFactor;
};

} // namespace GOOM::FILTER_FX::GPU_FILTER_EFFECTS

namespace GOOM::FILTER_FX::GPU_FILTER_EFFECTS
{

inline auto BeautifulField::SetRandomParams() noexcept -> void
{
  m_gpuParams = GetRandomParams();
}

inline auto BeautifulField::GetGpuParams() const noexcept -> const IGpuParams&
{
  return m_gpuParams;
}

} // namespace GOOM::FILTER_FX::GPU_FILTER_EFFECTS
