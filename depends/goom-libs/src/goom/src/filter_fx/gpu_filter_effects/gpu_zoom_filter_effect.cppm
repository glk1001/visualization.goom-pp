module;

#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

export module Goom.FilterFx.GpuFilterEffects.GpuZoomFilterEffect;

import Goom.FilterFx.CommonTypes;
import Goom.FilterFx.NormalizedCoords;
import Goom.Utils.NameValuePairs;

using GOOM::UTILS::NameValuePairs;

export namespace GOOM::FILTER_FX::GPU_FILTER_EFFECTS
{

class IGpuParams
{
public:
  IGpuParams(const std::string_view& filterName,
             const Viewport& viewport,
             const Amplitude& amplitude,
             const FilterBase& filterBase,
             const FrequencyFactor& cycleFrequency) noexcept;
  IGpuParams()                                     = default;
  IGpuParams(const IGpuParams&)                    = default;
  IGpuParams(IGpuParams&&)                         = default;
  virtual ~IGpuParams() noexcept                   = default;
  auto operator=(const IGpuParams&) -> IGpuParams& = default;
  auto operator=(IGpuParams&&) -> IGpuParams&      = default;

  struct FilterTimingInfo
  {
    float startTime;
    float maxTime;
  };
  struct SetterFuncs
  {
    std::function<void(const std::string_view& name, float value)> setFloat;
    std::function<void(const std::string_view& name, int32_t value)> setInt;
    std::function<void(const std::string_view& name, bool value)> setBool;
    std::function<void(const std::string_view& name, const std::vector<float>& value)> setFltVector;
    std::function<void(const std::string_view& name, const std::vector<int32_t>& value)>
        setIntVector;
  };

  virtual auto OutputGpuParams(const FilterTimingInfo& filterTimingInfo,
                               const SetterFuncs& setterFuncs) const noexcept -> void = 0;

protected:
  auto OutputStandardParams(const FilterTimingInfo& filterTimingInfo,
                            const SetterFuncs& setterFuncs) const noexcept -> void;

private:
  Viewport m_viewport{};

  Amplitude m_amplitude{};
  std::string m_xAmplitudeUniformName;
  std::string m_yAmplitudeUniformName;

  FilterBase m_filterBase{};
  std::string m_xFilterBaseUniformName;
  std::string m_yFilterBaseUniformName;

  FrequencyFactor m_cycleFrequency{};
  std::string m_xCycleFrequencyUniformName;
  std::string m_yCycleFrequencyUniformName;

  std::string m_startTimeUniformName;
  std::string m_maxTimeUniformName;
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
      -> NameValuePairs = 0;
};

} // namespace GOOM::FILTER_FX::GPU_FILTER_EFFECTS
