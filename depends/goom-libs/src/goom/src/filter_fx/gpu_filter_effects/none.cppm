export module Goom.FilterFx.GpuFilterEffects.None;

import Goom.FilterFx.GpuFilterEffects.GpuZoomFilterEffect;
import Goom.Utils.NameValuePairs;

using GOOM::UTILS::NameValuePairs;

export namespace GOOM::FILTER_FX::GPU_FILTER_EFFECTS
{

class None : public IGpuZoomFilterEffect
{
public:
  auto SetRandomParams() noexcept -> void override;

  [[nodiscard]] auto GetGpuParams() const noexcept -> const IGpuParams& override;
  [[nodiscard]] static auto GetEmptyGpuParams() noexcept -> const IGpuParams&;

  [[nodiscard]] auto GetGpuZoomFilterEffectNameValueParams() const noexcept
      -> NameValuePairs override;

  class GpuParams : public IGpuParams
  {
  public:
    auto OutputGpuParams(const FilterTimingInfo& filterTimingInfo,
                         const SetterFuncs& setterFuncs) const noexcept -> void override;
  };

private:
  static inline GpuParams s_gpuParams{};
};

} // namespace GOOM::FILTER_FX::GPU_FILTER_EFFECTS

namespace GOOM::FILTER_FX::GPU_FILTER_EFFECTS
{

inline auto None::GetGpuParams() const noexcept -> const IGpuParams&
{
  return GetEmptyGpuParams();
}

inline auto None::GetEmptyGpuParams() noexcept -> const IGpuParams&
{
  return s_gpuParams;
}

inline auto None::SetRandomParams() noexcept -> void
{
  // Does nothing.
}

inline auto None::GpuParams::OutputGpuParams(
    [[maybe_unused]] const FilterTimingInfo& filterTimingInfo,
    [[maybe_unused]] const SetterFuncs& setterFuncs) const noexcept -> void
{
  // Does nothing.
}

inline auto None::GetGpuZoomFilterEffectNameValueParams() const noexcept -> NameValuePairs
{
  return {};
}

} // namespace GOOM::FILTER_FX::GPU_FILTER_EFFECTS
