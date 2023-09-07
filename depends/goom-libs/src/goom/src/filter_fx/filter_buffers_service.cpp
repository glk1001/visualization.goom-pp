//#undef NO_LOGGING

#include "filter_buffers_service.h"

#include "filter_buffers.h"
#include "filter_settings.h"
#include "goom/goom_config.h"
#include "goom/goom_time.h"
#include "goom_plugin_info.h"
#include "normalized_coords.h"
#include "utils/name_value_pairs.h"
#include "utils/parallel_utils.h"
#include "zoom_vector.h"

#include <cmath>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>

namespace GOOM::FILTER_FX
{

using UTILS::GetPair;
using UTILS::MoveNameValuePairs;
using UTILS::NameValuePairs;
using UTILS::Parallel;

FilterBuffersService::FilterBuffersService(
    Parallel& parallel,
    const PluginInfo& goomInfo,
    const NormalizedCoordsConverter& normalizedCoordsConverter,
    std::unique_ptr<IZoomVector> zoomVector) noexcept
  : m_goomTime{&goomInfo.GetTime()},
    m_zoomVector{std::move(zoomVector)},
    m_filterBuffers{
        parallel,
        goomInfo,
        normalizedCoordsConverter,
        [this](const NormalizedCoords& normalizedCoords,
               const NormalizedCoords& normalizedFilterViewportCoords)
        { return m_zoomVector->GetZoomPoint(normalizedCoords, normalizedFilterViewportCoords); }}
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

  m_numPendingFilterEffectsChanges   = 0U;
  m_totalGoomTimeOfTransformBuffers  = 0U;
  m_totalGoomTimeBetweenBufferResets = 0U;
  m_numCompletedTransformBuffers     = 0U;
  m_numTransformBufferResets         = 0U;

  m_filterBuffers.Start();
}

auto FilterBuffersService::Finish() noexcept -> void
{
  m_filterBuffers.Finish();
}

auto FilterBuffersService::UpdateAllPendingSettings() noexcept -> void
{
  m_nextFilterEffectsSettings.afterEffectsSettings.rotationAdjustments.Reset();
  m_zoomVector->SetFilterEffectsSettings(m_nextFilterEffectsSettings);
  m_filterBuffers.SetTransformBufferMidpoint(m_nextFilterEffectsSettings.zoomMidpoint);
  m_filterBuffers.SetFilterViewport(m_nextFilterEffectsSettings.filterViewport);
  m_pendingFilterEffectsSettings = false;
}

auto FilterBuffersService::UpdateTransformBuffer() noexcept -> void
{
  if (ZoomFilterBuffers::UpdateStatus::HAS_BEEN_COPIED == m_filterBuffers.GetUpdateStatus())
  {
    if (m_goomTimeAtStartOfTransformBuffer > 0U)
    {
      ++m_numCompletedTransformBuffers;
      m_totalGoomTimeOfTransformBuffers +=
          m_goomTime->GetElapsedTimeSince(m_goomTimeAtStartOfTransformBuffer);
      m_goomTimeAtStartOfTransformBuffer = 0U;
    }
  }

  if (m_pendingFilterEffectsSettings and
      (ZoomFilterBuffers::UpdateStatus::HAS_BEEN_COPIED == m_filterBuffers.GetUpdateStatus()))
  {
    m_filterBuffers.ResetTransformBufferToStart();
    ++m_numTransformBufferResets;
    m_totalGoomTimeBetweenBufferResets += m_goomTime->GetElapsedTimeSince(m_goomTimeAtBufferReset);

    UpdateAllPendingSettings();
    Ensures(not m_pendingFilterEffectsSettings);
    ++m_numPendingFilterEffectsChanges;

    m_filterBuffers.StartTransformBufferUpdates();
    m_goomTimeAtStartOfTransformBuffer = m_goomTime->GetCurrentTime();
    m_goomTimeAtBufferReset            = m_goomTime->GetCurrentTime();
  }

  m_filterBuffers.UpdateTransformBuffer();
}

auto FilterBuffersService::GetNameValueParams(const std::string& paramGroup) const noexcept
    -> NameValuePairs
{
  static constexpr auto* PARAM_GROUP = "Buffer Service";

  auto nameValuePairs = UTILS::NameValuePairs{
      GetPair(PARAM_GROUP, "pending changes", m_numPendingFilterEffectsChanges),
      GetPair(PARAM_GROUP, "num compl buffs", m_numCompletedTransformBuffers),
      GetPair(PARAM_GROUP, "av buff time", GetAverageGoomTimeOfTransformBuffers()),
      GetPair(PARAM_GROUP, "num resets", m_numCompletedTransformBuffers),
      GetPair(PARAM_GROUP, "av betw resets", GetAverageGoomTimeBetweenBufferResets()),
  };
  MoveNameValuePairs(m_zoomVector->GetNameValueParams(paramGroup), nameValuePairs);

  return nameValuePairs;
}

auto FilterBuffersService::GetAverageGoomTimeOfTransformBuffers() const noexcept -> uint32_t
{
  if (m_numCompletedTransformBuffers == 0)
  {
    return 0U;
  }

  return static_cast<uint32_t>(std::round(static_cast<float>(m_totalGoomTimeOfTransformBuffers) /
                                          static_cast<float>(m_numCompletedTransformBuffers)));
}

auto FilterBuffersService::GetAverageGoomTimeBetweenBufferResets() const noexcept -> uint32_t
{
  if (m_numCompletedTransformBuffers == 0)
  {
    return 0U;
  }

  return static_cast<uint32_t>(std::round(static_cast<float>(m_totalGoomTimeBetweenBufferResets) /
                                          static_cast<float>(m_numTransformBufferResets)));
}

} // namespace GOOM::FILTER_FX
