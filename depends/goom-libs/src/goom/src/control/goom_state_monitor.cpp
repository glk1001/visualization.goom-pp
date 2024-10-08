module;

#include <string>
#include <unordered_set>

module Goom.Control.GoomStateMonitor;

import Goom.Control.GoomAllVisualFx;
import Goom.Control.GoomMusicSettingsReactor;
import Goom.FilterFx.FilterBuffersService;
import Goom.FilterFx.FilterSettingsService;
import Goom.Lib.Point2d;

namespace GOOM::CONTROL
{

using FILTER_FX::FilterBuffersService;
using FILTER_FX::FilterSettingsService;
using UTILS::GetNameValuesString;
using UTILS::GetPair;

GoomStateMonitor::GoomStateMonitor(const GoomAllVisualFx& visualFx,
                                   const GoomMusicSettingsReactor& musicSettingsReactor,
                                   const FilterSettingsService& filterSettingsService,
                                   const FilterBuffersService& filterBuffersService) noexcept
  : m_visualFx{&visualFx},
    m_musicSettingsReactor{&musicSettingsReactor},
    m_filterSettingsService{&filterSettingsService},
    m_filterBuffersService{&filterBuffersService}
{
}

auto GoomStateMonitor::GetCurrentState() const -> std::string
{
  auto message = std::string{};

  message += GetNameValuesString(GetStateAndFilterModeNameValueParams()) + "\n";
  message += GetNameValuesString(GetShaderVariablesNameValueParams()) + "\n";
  message += GetNameValuesString(GetMusicSettingsNameValueParams()) + "\n";
  message += GetNameValuesString(GetFilterBufferValueParams()) + "\n";
  message += GetNameValuesString(GetZoomFilterFxNameValueParams()) + "\n";
  message += GetNameValuesString(GetFilterEffectsNameValueParams());

  return message;
}

// TODO(glk) - clean this up.
namespace
{

[[nodiscard]] auto GetString(const std::unordered_set<std::string>& theSet) noexcept -> std::string
{
  auto str = std::string{};

  for (const auto& val : theSet)
  {
    str += "  " + val + "\n";
  }

  str.pop_back();
  return str;
}

} // namespace

auto GoomStateMonitor::GetStateAndFilterModeNameValueParams() const -> UTILS::NameValuePairs
{
  static constexpr auto* PARAM_GROUP = "";
  return {
      GetPair(PARAM_GROUP, "State", m_visualFx->GetCurrentStateName()),
      GetPair(PARAM_GROUP, "Color Maps", GetString(GoomAllVisualFx::GetCurrentColorMapsNames())),
      GetPair(PARAM_GROUP, "Filter Mode", m_filterSettingsService->GetCurrentFilterModeName()),
      GetPair(PARAM_GROUP,
              "Previous Filter Mode",
              m_filterSettingsService->GetPreviousFilterModeName()),
  };
}

auto GoomStateMonitor::GetShaderVariablesNameValueParams() const -> UTILS::NameValuePairs
{
  static constexpr auto* PARAM_GROUP = "Shader";
  const auto& lastFrameMiscData      = m_visualFx->GetFrameMiscData();
  return {
      GetPair(PARAM_GROUP, "Brightness", lastFrameMiscData.brightness),
      GetPair(PARAM_GROUP, "HueShift", lastFrameMiscData.hueShift),
      GetPair(PARAM_GROUP, "ChromaFactor", lastFrameMiscData.chromaFactor),
      GetPair(PARAM_GROUP, "BaseColorMultiplier", lastFrameMiscData.baseColorMultiplier),
  };
}

inline auto GoomStateMonitor::GetFilterBufferValueParams() const -> UTILS::NameValuePairs
{
  static constexpr auto* PARAM_GROUP = "Filter Buffer";
  const auto& transformBufferLerpData =
      m_filterSettingsService->GetFilterSettings().transformBufferLerpData;
  return {
      GetPair(PARAM_GROUP, "LerpFactor", transformBufferLerpData.GetLerpFactor()),
      GetPair(PARAM_GROUP, "LerpIncrement", transformBufferLerpData.GetIncrement()),
      GetPair(PARAM_GROUP, "LerpSFunc", transformBufferLerpData.GetUseSFunction()),
  };
}

inline auto GoomStateMonitor::GetFilterEffectsNameValueParams() const -> UTILS::NameValuePairs
{
  static constexpr auto* PARAM_GROUP = "Filter Settings";
  const auto& filterEffectsSettings =
      m_filterSettingsService->GetFilterSettings().filterEffectsSettings;
  return {
      GetPair(PARAM_GROUP,
              "Middle",
              Point2dInt{.x = filterEffectsSettings.zoomMidpoint.x,
                         .y = filterEffectsSettings.zoomMidpoint.y}),
      GetPair(PARAM_GROUP, "After Effects", filterEffectsSettings.afterEffectsVelocityMultiplier),
  };
}

inline auto GoomStateMonitor::GetMusicSettingsNameValueParams() const -> UTILS::NameValuePairs
{
  return m_musicSettingsReactor->GetNameValueParams();
}

inline auto GoomStateMonitor::GetZoomFilterFxNameValueParams() const -> UTILS::NameValuePairs
{
  static constexpr auto* PARAM_GROUP = "ZoomFilterFx";
  return m_filterBuffersService->GetNameValueParams(PARAM_GROUP);
}

} // namespace GOOM::CONTROL
