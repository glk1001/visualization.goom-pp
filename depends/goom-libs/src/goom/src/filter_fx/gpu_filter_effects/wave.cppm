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
              float sqDistPower,
              float waveSpinSign) noexcept;
    auto OutputGpuParams(const FilterTimingInfo& filterTimingInfo,
                         const SetterFuncs& setterFuncs) const noexcept -> void override;

    [[nodiscard]] auto GetFrequencyFactor() const noexcept -> const FrequencyFactor&;
    [[nodiscard]] auto GetReducerCoeff() const noexcept -> float;
    [[nodiscard]] auto GetSqDistPower() const noexcept -> float;
    [[nodiscard]] auto GetWaveSpinSign() const noexcept -> float;

  private:
    FrequencyFactor m_frequencyFactor{};
    float m_reducerCoeff{};
    float m_sqDistPower{};
    float m_waveSpinSign{};
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

inline auto Wave::GpuParams::GetFrequencyFactor() const noexcept -> const FrequencyFactor&
{
  return m_frequencyFactor;
}

inline auto Wave::GpuParams::GetReducerCoeff() const noexcept -> float
{
  return m_reducerCoeff;
}

inline auto Wave::GpuParams::GetSqDistPower() const noexcept -> float
{
  return m_sqDistPower;
}

inline auto Wave::GpuParams::GetWaveSpinSign() const noexcept -> float
{
  return m_waveSpinSign;
}

} // namespace GOOM::FILTER_FX::GPU_FILTER_EFFECTS
