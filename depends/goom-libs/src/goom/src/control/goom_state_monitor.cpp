module;

#include <format>
#include <string>
#include <unordered_set>
#include <vector>

module Goom.Control.GoomStateMonitor;

import Goom.Control.GoomAllVisualFx;
import Goom.Control.GoomMessageDisplayer;
import Goom.Control.GoomMusicSettingsReactor;
import Goom.FilterFx.FilterBuffersService;
import Goom.FilterFx.FilterSettingsService;
import Goom.Lib.Point2d;

namespace GOOM::CONTROL
{

using FILTER_FX::FilterBuffersService;
using FILTER_FX::FilterSettingsService;
using UTILS::GetNameValueGroups;
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

auto GoomStateMonitor::GetCurrentState() const -> std::vector<MessageGroup>
{
  using enum MessageGroupColors;

  auto messageGroup = std::vector<MessageGroup>(NUM_GROUPS + 1);

  // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
  // NOTE: Clunky but first index 0 is reserved for the caller.
  messageGroup.at(1) = {.color    = PURE_RED,
                        .messages = GetNameValueGroups(GetStateAndFilterModeNameValueParams())};
  messageGroup.at(2) = {.color    = PURE_LIME,
                        .messages = GetNameValueGroups(GetShaderVariablesNameValueParams())};
  messageGroup.at(3) = {.color    = PURE_BLUE,
                        .messages = GetNameValueGroups(GetMusicSettingsNameValueParams())};
  messageGroup.at(4) = {.color    = PURE_YELLOW,
                        .messages = GetNameValueGroups(GetFilterBufferValueParams())};
  messageGroup.at(5) = {.color    = ORANGE,
                        .messages = GetNameValueGroups(GetZoomFilterFxNameValueParams())};
  messageGroup.at(6) = {.color    = TIA_MARIA,
                        .messages = GetNameValueGroups(GetFilterEffectsNameValueParams())};
  // NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

  return messageGroup;
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
  static constexpr auto* PARAM_GROUP = "Main";
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
  return {GetPair(PARAM_GROUP,
                  "params",
                  std::format("{:.2f}, {:.2f}, {}",
                              transformBufferLerpData.GetLerpFactor(),
                              transformBufferLerpData.GetIncrement(),
                              transformBufferLerpData.GetUseSFunction()))};
}

inline auto GoomStateMonitor::GetFilterEffectsNameValueParams() const -> UTILS::NameValuePairs
{
  static constexpr auto* PARAM_GROUP = "Filter Settings";
  const auto& filterEffectsSettings =
      m_filterSettingsService->GetFilterSettings().filterEffectsSettings;
  return {
      GetPair(PARAM_GROUP,
              "Midpoint",
              Point2dInt{.x = filterEffectsSettings.zoomMidpoint.x,
                         .y = filterEffectsSettings.zoomMidpoint.y}),
      GetPair(PARAM_GROUP, "After Mult", filterEffectsSettings.afterEffectsVelocityMultiplier),
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
