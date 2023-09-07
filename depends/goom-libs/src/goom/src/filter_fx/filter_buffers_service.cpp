//#undef NO_LOGGING

#include "filter_buffers_service.h"

#include "filter_buffers.h"
#include "filter_settings.h"
#include "goom/goom_config.h"
#include "goom/goom_time.h"
#include "goom_plugin_info.h"
#include "normalized_coords.h"
#include "utils/name_value_pairs.h"
#include "zoom_vector.h"

#include <cmath>
#include <cstdint>
#include <memory>
#include <string>
#include <thread>
#include <utility>

namespace GOOM::FILTER_FX
{

using UTILS::GetPair;
using UTILS::MoveNameValuePairs;
using UTILS::NameValuePairs;

FilterBuffersService::FilterBuffersService(
    const PluginInfo& goomInfo,
    const NormalizedCoordsConverter& normalizedCoordsConverter,
    std::unique_ptr<IZoomVector> zoomVector) noexcept
  : m_goomTime{&goomInfo.GetTime()},
    m_zoomVector{std::move(zoomVector)},
    m_filterBuffers{
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
  m_totalGoomTimeOfBufferProcessing  = 0U;
  m_totalGoomTimeBetweenBufferResets = 0U;
  m_numTransformBuffersCompleted     = 0U;
  m_numTransformBufferResets         = 0U;

  m_filterBuffers.Start();

  StartTransformBufferThread();
}

auto FilterBuffersService::Finish() noexcept -> void
{
  m_filterBuffers.Finish();
  m_bufferProducerThread.join();
}

auto FilterBuffersService::StartTransformBufferThread() noexcept -> void
{
  m_bufferProducerThread = std::thread{&ZoomFilterBuffers::TransformBufferThread, &m_filterBuffers};
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
    UpdateCompletedTransformBufferStats();
  }

  if (m_pendingFilterEffectsSettings and
      (ZoomFilterBuffers::UpdateStatus::HAS_BEEN_COPIED == m_filterBuffers.GetUpdateStatus()))
  {
    CompletePendingSettings();
  }
}

auto FilterBuffersService::CompletePendingSettings() noexcept -> void
{
  m_filterBuffers.ResetTransformBufferToStart();
  ++m_numTransformBufferResets;
  m_totalGoomTimeBetweenBufferResets +=
      m_goomTime->GetElapsedTimeSince(m_goomTimeAtTransformBufferReset);

  UpdateAllPendingSettings();
  Ensures(not m_pendingFilterEffectsSettings);
  ++m_numPendingFilterEffectsChanges;

  m_filterBuffers.StartTransformBufferUpdates();
  m_goomTimeAtTransformBufferStart = m_goomTime->GetCurrentTime();
  m_goomTimeAtTransformBufferReset = m_goomTime->GetCurrentTime();
}

auto FilterBuffersService::UpdateCompletedTransformBufferStats() noexcept -> void
{
  if (m_goomTimeAtTransformBufferStart == 0U)
  {
    return;
  }

  m_totalGoomTimeOfBufferProcessing +=
      m_goomTime->GetElapsedTimeSince(m_goomTimeAtTransformBufferStart);
  ++m_numTransformBuffersCompleted;

  m_goomTimeAtTransformBufferStart = 0U;
}

auto FilterBuffersService::GetNameValueParams(const std::string& paramGroup) const noexcept
    -> NameValuePairs
{
  static constexpr auto* PARAM_GROUP = "Buffer Service";

  auto nameValuePairs = UTILS::NameValuePairs{
      GetPair(PARAM_GROUP, "num pending changes", m_numPendingFilterEffectsChanges),
      GetPair(PARAM_GROUP, "num completed buffs", m_numTransformBuffersCompleted),
      GetPair(PARAM_GROUP, "av time to do buff", GetAverageGoomTimeOfBufferProcessing()),
      GetPair(PARAM_GROUP, "av time betw resets", GetAverageGoomTimeBetweenBufferResets()),
  };
  MoveNameValuePairs(m_zoomVector->GetNameValueParams(paramGroup), nameValuePairs);

  return nameValuePairs;
}

auto FilterBuffersService::GetAverageGoomTimeOfBufferProcessing() const noexcept -> uint32_t
{
  if (m_numTransformBuffersCompleted == 0)
  {
    return 0U;
  }

  return static_cast<uint32_t>(std::round(static_cast<float>(m_totalGoomTimeOfBufferProcessing) /
                                          static_cast<float>(m_numTransformBuffersCompleted)));
}

auto FilterBuffersService::GetAverageGoomTimeBetweenBufferResets() const noexcept -> uint32_t
{
  if (m_numTransformBuffersCompleted == 0)
  {
    return 0U;
  }

  return static_cast<uint32_t>(std::round(static_cast<float>(m_totalGoomTimeBetweenBufferResets) /
                                          static_cast<float>(m_numTransformBufferResets)));
}

} // namespace GOOM::FILTER_FX
