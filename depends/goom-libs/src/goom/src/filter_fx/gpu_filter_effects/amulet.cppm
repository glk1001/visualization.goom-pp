export module Goom.FilterFx.GpuFilterEffects.Amulet;

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

class Amulet : public IGpuZoomFilterEffect
{
public:
  explicit Amulet(const GoomRand& goomRand) noexcept;

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
              float amuletSpinSign) noexcept;
    auto OutputGpuParams(const FilterTimingInfo& filterTimingInfo,
                         const SetterFuncs& setterFuncs) const noexcept -> void override;

    [[nodiscard]] auto GetAmuletSpinSign() const noexcept -> float;

  private:
    float m_amuletSpinSign = 1.0F;
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

inline auto Amulet::SetRandomParams() noexcept -> void
{
  m_gpuParams = GetRandomParams();
}

inline auto Amulet::GetGpuParams() const noexcept -> const IGpuParams&
{
  return m_gpuParams;
}

inline auto Amulet::GpuParams::GetAmuletSpinSign() const noexcept -> float
{
  return m_amuletSpinSign;
}

} // namespace GOOM::FILTER_FX::GPU_FILTER_EFFECTS
