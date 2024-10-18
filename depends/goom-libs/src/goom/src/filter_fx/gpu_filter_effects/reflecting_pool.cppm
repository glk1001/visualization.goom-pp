export module Goom.FilterFx.GpuFilterEffects.ReflectingPool;

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

class ReflectingPool : public IGpuZoomFilterEffect
{
public:
  explicit ReflectingPool(const GoomRand& goomRand) noexcept;

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
              const FrequencyFactor& innerPosFactor) noexcept;
    auto OutputGpuParams(const FilterTimingInfo& filterTimingInfo,
                         const SetterFuncs& setterFuncs) const noexcept -> void override;

    [[nodiscard]] auto GetFrequencyFactor() const noexcept -> const FrequencyFactor&;
    [[nodiscard]] auto GetInnerPosFactor() const noexcept -> const FrequencyFactor&;

  private:
    FrequencyFactor m_frequencyFactor{};
    FrequencyFactor m_innerPosFactor{};
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

inline auto ReflectingPool::SetRandomParams() noexcept -> void
{
  m_gpuParams = GetRandomParams();
}

inline auto ReflectingPool::GetGpuParams() const noexcept -> const IGpuParams&
{
  return m_gpuParams;
}

inline auto ReflectingPool::GpuParams::GetFrequencyFactor() const noexcept -> const FrequencyFactor&
{
  return m_frequencyFactor;
}

inline auto ReflectingPool::GpuParams::GetInnerPosFactor() const noexcept -> const FrequencyFactor&
{
  return m_innerPosFactor;
}

} // namespace GOOM::FILTER_FX::GPU_FILTER_EFFECTS
