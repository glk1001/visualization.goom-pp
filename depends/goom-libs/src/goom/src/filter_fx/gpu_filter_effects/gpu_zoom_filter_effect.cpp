module;

#include <format>
#include <string>

module Goom.FilterFx.GpuFilterEffects.GpuZoomFilterEffect;

import Goom.FilterFx.CommonTypes;
import Goom.FilterFx.NormalizedCoords;

namespace GOOM::FILTER_FX::GPU_FILTER_EFFECTS
{

IGpuParams::IGpuParams(const std::string_view& filterName,
                       const Viewport& viewport,
                       const Amplitude& amplitude,
                       const FilterBase& filterBase,
                       const FrequencyFactor& cycleFrequency) noexcept
  : m_viewport{viewport},
    m_amplitude{amplitude},
    m_xAmplitudeUniformName{std::format("u_{}XAmplitude", filterName)},
    m_yAmplitudeUniformName{std::format("u_{}YAmplitude", filterName)},
    m_filterBase{filterBase},
    m_xFilterBaseUniformName{std::format("u_{}XBase", filterName)},
    m_yFilterBaseUniformName{std::format("u_{}YBase", filterName)},
    m_cycleFrequency{cycleFrequency},
    m_xCycleFrequencyUniformName{std::format("u_{}XCycleFreq", filterName)},
    m_yCycleFrequencyUniformName{std::format("u_{}YCycleFreq", filterName)},
    m_startTimeUniformName{std::format("u_{}StartTime", filterName)},
    m_maxTimeUniformName{std::format("u_{}MaxTime", filterName)}
{
}

auto IGpuParams::OutputStandardParams(const FilterTimingInfo& filterTimingInfo,
                                      const SetterFuncs& setterFuncs) const noexcept -> void
{
  setterFuncs.setFloat(m_startTimeUniformName, filterTimingInfo.startTime);
  setterFuncs.setFloat(m_maxTimeUniformName, filterTimingInfo.maxTime);
  setterFuncs.setFloat(m_xAmplitudeUniformName, m_amplitude.x);
  setterFuncs.setFloat(m_yAmplitudeUniformName, m_amplitude.y);
  setterFuncs.setFloat(m_xFilterBaseUniformName, m_filterBase.x);
  setterFuncs.setFloat(m_yFilterBaseUniformName, m_filterBase.y);
  setterFuncs.setFloat(m_xCycleFrequencyUniformName, m_cycleFrequency.x);
  setterFuncs.setFloat(m_yCycleFrequencyUniformName, m_cycleFrequency.y);
}

auto IGpuParams::GetFormattedInOrderParams() const noexcept -> std::string
{
  return std::format("({:.2f},{:.2f}), ({:.2f},{:.2f}), ({:.2f},{:.2f})",
                     m_amplitude.x,
                     m_amplitude.y,
                     m_filterBase.x,
                     m_filterBase.y,
                     m_cycleFrequency.x,
                     m_cycleFrequency.y);
}

} // namespace GOOM::FILTER_FX::GPU_FILTER_EFFECTS
