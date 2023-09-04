#pragma once

#include "filter_buffers.h"
#include "filter_settings.h"
#include "goom/point2d.h"
#include "normalized_coords.h"
#include "utils/name_value_pairs.h"
#include "zoom_vector.h"

#include <cstdint>
#include <memory>
#include <span> // NOLINT: Waiting to use C++20.
#include <string>
#include <vector>

namespace GOOM
{
class PluginInfo;

namespace UTILS
{
class Parallel;
}
} // namespace GOOM

namespace GOOM::FILTER_FX
{

class FilterBuffersService
{
public:
  FilterBuffersService(UTILS::Parallel& parallel,
                       const PluginInfo& goomInfo,
                       const NormalizedCoordsConverter& normalizedCoordsConverter,
                       std::unique_ptr<IZoomVector> zoomVector) noexcept;

  auto SetFilterEffectsSettings(const FilterEffectsSettings& filterEffectsSettings) noexcept
      -> void;

  auto Start() noexcept -> void;
  auto Finish() noexcept -> void;

  [[nodiscard]] auto IsTransformBufferReadyToCopy() const noexcept -> bool;
  [[nodiscard]] auto GetPreviousTransformBuffer() const noexcept -> const std::vector<Point2dFlt>&;
  // NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
  auto CopyTransformBuffer(std_spn::span<Point2dFlt> destBuff) noexcept -> void;

  auto UpdateTransformBuffer() noexcept -> void;

  [[nodiscard]] auto GetNameValueParams(const std::string& paramGroup) const noexcept
      -> UTILS::NameValuePairs;

private:
  std::unique_ptr<IZoomVector> m_zoomVector;
  ZoomFilterBuffers m_filterBuffers;

  FilterEffectsSettings m_nextFilterEffectsSettings{};
  bool m_pendingFilterEffectsSettings       = false;
  uint64_t m_numPendingFilterEffectsChanges = 0U;

  auto UpdateAllPendingSettings() noexcept -> void;
};

inline auto FilterBuffersService::IsTransformBufferReadyToCopy() const noexcept -> bool
{
  return ZoomFilterBuffers::UpdateStatus::AT_END == m_filterBuffers.GetUpdateStatus();
}

inline auto FilterBuffersService::GetPreviousTransformBuffer() const noexcept
    -> const std::vector<Point2dFlt>&
{
  return m_filterBuffers.GetPreviousTransformBuffer();
}

// NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
inline auto FilterBuffersService::CopyTransformBuffer(std_spn::span<Point2dFlt> destBuff) noexcept
    -> void
{
  m_filterBuffers.CopyTransformBuffer(destBuff);
}

} // namespace GOOM::FILTER_FX
