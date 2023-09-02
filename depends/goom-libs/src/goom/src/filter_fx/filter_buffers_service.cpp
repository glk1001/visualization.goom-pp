//#undef NO_LOGGING

#include "filter_buffers_service.h"

#include "filter_buffer_striper.h"
#include "filter_buffers.h"
#include "filter_settings.h"
#include "goom/goom_config.h"
#include "goom_plugin_info.h"
#include "normalized_coords.h"
#include "utils/name_value_pairs.h"
#include "utils/parallel_utils.h"
#include "zoom_vector.h"

#include <memory>
#include <string>
#include <utility>

namespace GOOM::FILTER_FX
{

using UTILS::MoveNameValuePairs;
using UTILS::NameValuePairs;
using UTILS::Parallel;

FilterBuffersService::FilterBuffersService(
    Parallel& parallel,
    const PluginInfo& goomInfo,
    const NormalizedCoordsConverter& normalizedCoordsConverter,
    std::unique_ptr<IZoomVector> zoomVector) noexcept
  : m_zoomVector{std::move(zoomVector)},
    m_filterBuffers{std::make_unique<ZoomFilterBufferStriper>(
        parallel,
        goomInfo,
        normalizedCoordsConverter,
        [this](const NormalizedCoords& normalizedCoords,
               const NormalizedCoords& normalizedFilterViewportCoords)
        { return m_zoomVector->GetZoomPoint(normalizedCoords, normalizedFilterViewportCoords); })}
{
}

auto FilterBuffersService::SetFilterEffectsSettings(
    const FilterEffectsSettings& filterEffectsSettings) noexcept -> void
{
  m_nextFilterEffectsSettings    = filterEffectsSettings;
  m_pendingFilterEffectsSettings = true;
}

auto FilterBuffersService::Start() noexcept -> void
{
  Expects(m_pendingFilterEffectsSettings);
  Expects(m_nextFilterEffectsSettings.zoomAdjustmentEffect != nullptr);

  UpdateAllPendingSettings();

  m_filterBuffers.Start();
}

auto FilterBuffersService::UpdateAllPendingSettings() noexcept -> void
{
  if (not m_pendingFilterEffectsSettings)
  {
    return;
  }

  m_nextFilterEffectsSettings.afterEffectsSettings.rotationAdjustments.Reset();
  m_zoomVector->SetFilterEffectsSettings(m_nextFilterEffectsSettings);
  m_filterBuffers.SetTransformBufferMidpoint(m_nextFilterEffectsSettings.zoomMidpoint);
  m_filterBuffers.SetFilterViewport(m_nextFilterEffectsSettings.filterViewport);
  m_pendingFilterEffectsSettings = false;

  // Notify buffer maker of settings change - now we'll get a fresh buffer.
  m_filterBuffers.NotifyFilterSettingsHaveChanged();
}

auto FilterBuffersService::UpdateTransformBuffer() noexcept -> void
{
  m_filterBuffers.UpdateTransformBuffer();

  if (m_filterBuffers.HasTransformBufferBeenCopied())
  {
    UpdateAllPendingSettings();
  }
}

auto FilterBuffersService::GetNameValueParams(const std::string& paramGroup) const noexcept
    -> NameValuePairs
{
  auto nameValuePairs = NameValuePairs{};

  MoveNameValuePairs(m_zoomVector->GetNameValueParams(paramGroup), nameValuePairs);

  return nameValuePairs;
}

} // namespace GOOM::FILTER_FX
