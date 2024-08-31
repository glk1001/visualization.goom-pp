module;

#include <cstdint>
#include <functional>
#include <string_view>
#include <vector>

export module Goom.FilterFx.GpuFilterEffects.GpuZoomFilterEffect;

import Goom.Utils.NameValuePairs;

export namespace GOOM::FILTER_FX::GPU_FILTER_EFFECTS
{

class IGpuParams
{
public:
  IGpuParams()                                     = default;
  IGpuParams(const IGpuParams&)                    = default;
  IGpuParams(IGpuParams&&)                         = default;
  virtual ~IGpuParams() noexcept                   = default;
  auto operator=(const IGpuParams&) -> IGpuParams& = default;
  auto operator=(IGpuParams&&) -> IGpuParams&      = default;

  struct SetterFuncs
  {
    std::function<void(const std::string_view& name, float value)> setFloat;
    std::function<void(const std::string_view& name, int32_t value)> setInt;
    std::function<void(const std::string_view& name, bool value)> setBool;
    std::function<void(const std::string_view& name, const std::vector<float>& value)> setFltVector;
    std::function<void(const std::string_view& name, const std::vector<int32_t>& value)>
        setIntVector;
  };

  virtual auto OutputGpuParams(const SetterFuncs& setterFuncs) const noexcept -> void = 0;
};

class IGpuZoomFilterEffect
{
public:
  IGpuZoomFilterEffect()                                               = default;
  IGpuZoomFilterEffect(const IGpuZoomFilterEffect&)                    = default;
  IGpuZoomFilterEffect(IGpuZoomFilterEffect&&)                         = default;
  virtual ~IGpuZoomFilterEffect() noexcept                             = default;
  auto operator=(const IGpuZoomFilterEffect&) -> IGpuZoomFilterEffect& = default;
  auto operator=(IGpuZoomFilterEffect&&) -> IGpuZoomFilterEffect&      = default;

  virtual auto SetRandomParams() noexcept -> void = 0;

  [[nodiscard]] virtual auto GetGpuParams() const noexcept -> const IGpuParams& = 0;

  [[nodiscard]] virtual auto GetGpuZoomFilterEffectNameValueParams() const noexcept
      -> GOOM::UTILS::NameValuePairs = 0;
};

} // namespace GOOM::FILTER_FX::GPU_FILTER_EFFECTS
