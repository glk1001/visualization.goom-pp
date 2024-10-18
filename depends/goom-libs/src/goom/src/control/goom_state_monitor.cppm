module;

#include <string>
#include <vector>

export module Goom.Control.GoomStateMonitor;

import Goom.Control.GoomAllVisualFx;
import Goom.Control.GoomMessageDisplayer;
import Goom.Control.GoomMusicSettingsReactor;
import Goom.FilterFx.FilterBuffersService;
import Goom.FilterFx.FilterSettingsService;
import Goom.Utils.NameValuePairs;
import Goom.Utils.EnumUtils;

using GOOM::UTILS::NUM;

export namespace GOOM::CONTROL
{

class GoomStateMonitor
{
public:
  GoomStateMonitor(const GoomAllVisualFx& visualFx,
                   const GoomMusicSettingsReactor& musicSettingsReactor,
                   const FILTER_FX::FilterSettingsService& filterSettingsService,
                   const FILTER_FX::FilterBuffersService& filterBuffersService) noexcept;
  [[nodiscard]] auto GetCurrentState() const -> std::vector<MessageGroup>;

private:
  const GoomAllVisualFx* m_visualFx;
  const GoomMusicSettingsReactor* m_musicSettingsReactor;
  const FILTER_FX::FilterSettingsService* m_filterSettingsService;
  const FILTER_FX::FilterBuffersService* m_filterBuffersService;

  static constexpr auto NUM_GROUPS = 7U;
  static_assert(NUM_GROUPS < NUM<MessageGroupColors>);
  [[nodiscard]] auto GetStateAndFilterModeNameValueParams() const -> UTILS::NameValuePairs;
  [[nodiscard]] auto GetShaderVariablesNameValueParams() const -> UTILS::NameValuePairs;
  [[nodiscard]] auto GetFilterBufferValueParams() const -> UTILS::NameValuePairs;
  [[nodiscard]] auto GetFilterEffectsNameValueParams() const -> UTILS::NameValuePairs;
  [[nodiscard]] auto GetGpuFilterEffectsNameValueParams() const -> UTILS::NameValuePairs;
  [[nodiscard]] auto GetMusicSettingsNameValueParams() const -> UTILS::NameValuePairs;
  [[nodiscard]] auto GetZoomFilterFxNameValueParams() const -> UTILS::NameValuePairs;
};

} // namespace GOOM::CONTROL
