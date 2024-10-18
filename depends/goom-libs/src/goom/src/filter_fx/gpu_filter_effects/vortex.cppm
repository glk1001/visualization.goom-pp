export module Goom.FilterFx.GpuFilterEffects.Vortex;

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

class Vortex : public IGpuZoomFilterEffect
{
public:
  explicit Vortex(const GoomRand& goomRand) noexcept;

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
              float frequencyFactor,
              float positionFactor,
              float rFactor,
              float vortexSpinSign) noexcept;
    auto OutputGpuParams(const FilterTimingInfo& filterTimingInfo,
                         const SetterFuncs& setterFuncs) const noexcept -> void override;

    [[nodiscard]] auto GetFrequencyFactor() const noexcept -> float;
    [[nodiscard]] auto GetPositionFactor() const noexcept -> float;
    [[nodiscard]] auto GetRFactor() const noexcept -> float;
    [[nodiscard]] auto GetVortexSpinSign() const noexcept -> float;

  private:
    float m_frequencyFactor{};
    float m_positionFactor{};
    float m_rFactor{};
    float m_vortexSpinSign{};
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

inline auto Vortex::SetRandomParams() noexcept -> void
{
  m_gpuParams = GetRandomParams();
}

inline auto Vortex::GetGpuParams() const noexcept -> const IGpuParams&
{
  return m_gpuParams;
}

inline auto Vortex::GpuParams::GetFrequencyFactor() const noexcept -> float
{
  return m_frequencyFactor;
}

inline auto Vortex::GpuParams::GetPositionFactor() const noexcept -> float
{
  return m_positionFactor;
}

inline auto Vortex::GpuParams::GetRFactor() const noexcept -> float
{
  return m_rFactor;
}

inline auto Vortex::GpuParams::GetVortexSpinSign() const noexcept -> float
{
  return m_vortexSpinSign;
}

} // namespace GOOM::FILTER_FX::GPU_FILTER_EFFECTS
