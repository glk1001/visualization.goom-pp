module;

#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <thread>

export module Goom.FilterFx.FilterBuffersService;

import Goom.FilterFx.FilterBuffers;
import Goom.FilterFx.FilterSettings;
import Goom.FilterFx.NormalizedCoords;
import Goom.FilterFx.ZoomVector;
import Goom.Utils.GoomTime;
import Goom.Utils.NameValuePairs;
import Goom.Lib.Point2d;
import Goom.PluginInfo;

export namespace GOOM::FILTER_FX
{

class FilterBuffersService
{
public:
  FilterBuffersService(const PluginInfo& goomInfo,
                       const NormalizedCoordsConverter& normalizedCoordsConverter,
                       std::unique_ptr<IZoomVector> zoomVector) noexcept;

  auto SetFilterEffectsSettings(const FilterEffectsSettings& filterEffectsSettings) noexcept
      -> void;

  auto Start() noexcept -> void;
  auto Finish() noexcept -> void;

  [[nodiscard]] auto IsTransformBufferReadyForNextFilter() const noexcept -> bool;

  [[nodiscard]] auto IsTransformBufferReadyToCopy() const noexcept -> bool;
  auto CopyTransformBuffer(std::span<Point2dFlt> destBuff) noexcept -> void;

  auto UpdateTransformBuffer() noexcept -> void;

  [[nodiscard]] auto GetNameValueParams(const std::string& paramGroup) const noexcept
      -> UTILS::NameValuePairs;

private:
  const UTILS::GoomTime* m_goomTime;
  std::unique_ptr<IZoomVector> m_zoomVector;
  ZoomFilterBuffers m_filterBuffers;

  FilterEffectsSettings m_nextFilterEffectsSettings{};
  bool m_pendingFilterEffectsSettings       = false;
  uint64_t m_numPendingFilterEffectsChanges = 0U;

  std::thread m_bufferProducerThread;
  auto StartTransformBufferThread() noexcept -> void;
  auto UpdateCompletedTransformBufferStats() noexcept -> void;
  auto CompletePendingSettings() noexcept -> void;
  auto UpdateAllPendingSettings() noexcept -> void;

  uint64_t m_goomTimeAtTransformBufferStart   = 0U;
  uint64_t m_goomTimeAtTransformBufferReset   = 0U;
  uint64_t m_totalGoomTimeOfBufferProcessing  = 0U;
  uint64_t m_totalGoomTimeBetweenBufferResets = 0U;
  uint32_t m_numTransformBuffersCompleted     = 0U;
  uint32_t m_numTransformBufferResets         = 0U;
  [[nodiscard]] auto GetAverageGoomTimeOfBufferProcessing() const noexcept -> uint32_t;
  [[nodiscard]] auto GetAverageGoomTimeBetweenBufferResets() const noexcept -> uint32_t;
};

} // namespace GOOM::FILTER_FX

namespace GOOM::FILTER_FX
{

inline auto FilterBuffersService::IsTransformBufferReadyForNextFilter() const noexcept -> bool
{
  return ZoomFilterBuffers::UpdateStatus::IN_PROGRESS != m_filterBuffers.GetUpdateStatus();
}

inline auto FilterBuffersService::IsTransformBufferReadyToCopy() const noexcept -> bool
{
  return ZoomFilterBuffers::UpdateStatus::AT_END == m_filterBuffers.GetUpdateStatus();
}

inline auto FilterBuffersService::CopyTransformBuffer(std::span<Point2dFlt> destBuff) noexcept
    -> void
{
  m_filterBuffers.CopyTransformBuffer(destBuff);
}

} // namespace GOOM::FILTER_FX
