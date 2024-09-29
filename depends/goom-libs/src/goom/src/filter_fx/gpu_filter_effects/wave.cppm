export module Goom.FilterFx.GpuFilterEffects.Wave;

import Goom.FilterFx.GpuFilterEffects.GpuZoomFilterEffect;
import Goom.FilterFx.FilterUtils.Utils;
import Goom.FilterFx.CommonTypes;
import Goom.FilterFx.NormalizedCoords;
import Goom.Utils.NameValuePairs;
import Goom.Utils.Math.GoomRand;

using GOOM::FILTER_FX::FILTER_UTILS::RandomViewport;
using GOOM::UTILS::NameValuePairs;
using GOOM::UTILS::MATH::GoomRand;

export namespace GOOM::FILTER_FX::GPU_FILTER_EFFECTS
{

class Wave : public IGpuZoomFilterEffect
{
public:
  explicit Wave(const GoomRand& goomRand) noexcept;

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
              float reducerCoeff,
              float sqDistPower) noexcept;
    auto OutputGpuParams(const FilterTimingInfo& filterTimingInfo,
                         const SetterFuncs& setterFuncs) const noexcept -> void override;

  private:
    FrequencyFactor m_frequencyFactor{};
    float m_reducerCoeff{};
    float m_sqDistPower{};
  };

private:
  const GoomRand* m_goomRand;
  RandomViewport m_randomViewport;
  GpuParams m_gpuParams;
  [[nodiscard]] auto GetRandomParams() const noexcept -> GpuParams;
};

} // namespace GOOM::FILTER_FX::GPU_FILTER_EFFECTS

namespace GOOM::FILTER_FX::GPU_FILTER_EFFECTS
{

inline auto Wave::SetRandomParams() noexcept -> void
{
  m_gpuParams = GetRandomParams();
}

inline auto Wave::GetGpuParams() const noexcept -> const IGpuParams&
{
  return m_gpuParams;
}

} // namespace GOOM::FILTER_FX::GPU_FILTER_EFFECTS
