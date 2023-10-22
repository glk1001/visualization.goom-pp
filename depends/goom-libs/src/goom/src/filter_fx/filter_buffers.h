#pragma once

#include "goom/goom_config.h"
#include "goom/goom_types.h"
#include "goom/point2d.h"
#include "normalized_coords.h"
#include "utils/parallel_utils.h"

#include <algorithm>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <span> // NOLINT: Waiting to use C++20.
#include <vector>

namespace GOOM
{
class PluginInfo;
}

namespace GOOM::FILTER_FX
{

class ZoomFilterBuffers
{
public:
  enum class UpdateStatus : UnderlyingEnumType
  {
    AT_START,
    IN_PROGRESS,
    AT_END,
    HAS_BEEN_COPIED,
  };

  using ZoomPointFunc =
      std::function<NormalizedCoords(const NormalizedCoords& normalizedCoords,
                                     const NormalizedCoords& normalizedFilterViewportCoords)>;

  ZoomFilterBuffers(const PluginInfo& goomInfo,
                    const NormalizedCoordsConverter& normalizedCoordsConverter,
                    const ZoomPointFunc& getZoomPointFunc) noexcept;

  auto SetTransformBufferMidpoint(const Point2dInt& midpoint) noexcept -> void;
  auto SetFilterViewport(const Viewport& viewport) noexcept -> void;

  auto Start() noexcept -> void;
  auto Finish() noexcept -> void;

  auto TransformBufferThread() noexcept -> void;

  [[nodiscard]] auto GetUpdateStatus() const noexcept -> UpdateStatus;
  auto ResetTransformBufferToStart() noexcept -> void;
  auto StartTransformBufferUpdates() noexcept -> void;

  // NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
  auto CopyTransformBuffer(std_spn::span<Point2dFlt> destBuff) noexcept -> void;

protected:
  // For testing only.
  [[nodiscard]] auto GetTransformBufferMidpoint() const noexcept -> Point2dInt;
  auto UpdateTransformBuffer() noexcept -> void;

private:
  Dimensions m_dimensions;
  const NormalizedCoordsConverter* m_normalizedCoordsConverter;
  UpdateStatus m_updateStatus = UpdateStatus::AT_START;

  bool m_shutdown = false;
  std::mutex m_mutex{};
  std::condition_variable m_bufferProducer_cv{};

  UTILS::Parallel m_parallel{UTILS::GetNumAvailablePoolThreads()};
  ZoomPointFunc m_getZoomPoint;
  Point2dInt m_midpoint                 = {0, 0};
  NormalizedCoords m_normalizedMidpoint = {0.0F, 0.0F};
  Viewport m_filterViewport             = Viewport{};

  std::vector<Point2dFlt> m_transformBuffer;

  auto DoNextTransformBuffer() noexcept -> void;
};

inline auto ZoomFilterBuffers::GetUpdateStatus() const noexcept -> UpdateStatus
{
  return m_updateStatus;
}

inline auto ZoomFilterBuffers::GetTransformBufferMidpoint() const noexcept -> Point2dInt
{
  return m_midpoint;
}

inline auto ZoomFilterBuffers::SetTransformBufferMidpoint(const Point2dInt& midpoint) noexcept
    -> void
{
  Expects(UpdateStatus::AT_START == m_updateStatus);

  m_midpoint           = midpoint;
  m_normalizedMidpoint = m_normalizedCoordsConverter->OtherToNormalizedCoords(m_midpoint);
}

inline auto ZoomFilterBuffers::SetFilterViewport(const Viewport& viewport) noexcept -> void
{
  Expects(UpdateStatus::AT_START == m_updateStatus);

  m_filterViewport = viewport;
}

// NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
inline auto ZoomFilterBuffers::CopyTransformBuffer(std_spn::span<Point2dFlt> destBuff) noexcept
    -> void
{
  Expects(UpdateStatus::AT_END == m_updateStatus);

  std::copy(m_transformBuffer.cbegin(), m_transformBuffer.cend(), destBuff.begin());
  m_updateStatus = UpdateStatus::HAS_BEEN_COPIED;
}

} // namespace GOOM::FILTER_FX
