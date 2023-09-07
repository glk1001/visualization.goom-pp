#pragma once

#include "goom/goom_config.h"
#include "goom/goom_time.h"
#include "goom/goom_types.h"
#include "goom/point2d.h"
#include "normalized_coords.h"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <span> // NOLINT: Waiting to use C++20.
#include <vector>

namespace GOOM
{
class PluginInfo;
}
namespace GOOM::UTILS
{
class Parallel;
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

  ZoomFilterBuffers(UTILS::Parallel& parallel,
                    const PluginInfo& goomInfo,
                    const NormalizedCoordsConverter& normalizedCoordsConverter,
                    const ZoomPointFunc& getZoomPointFunc) noexcept;

  auto SetTransformBufferMidpoint(const Point2dInt& midpoint) noexcept -> void;
  auto SetFilterViewport(const Viewport& viewport) noexcept -> void;

  auto Start() noexcept -> void;
  auto Finish() noexcept -> void;

  [[nodiscard]] auto GetUpdateStatus() const noexcept -> UpdateStatus;
  auto ResetTransformBufferToStart() noexcept -> void;
  auto StartTransformBufferUpdates() noexcept -> void;
  auto UpdateTransformBuffer() noexcept -> void;

  [[nodiscard]] auto GetPreviousTransformBuffer() const noexcept -> const std::vector<Point2dFlt>&;
  // NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
  auto CopyTransformBuffer(std_spn::span<Point2dFlt> destBuff) noexcept -> void;

protected:
  // For testing only.
  [[nodiscard]] auto GetTransformBufferMidpoint() const noexcept -> Point2dInt;

private:
  Dimensions m_dimensions;
  const NormalizedCoordsConverter* m_normalizedCoordsConverter;
  UpdateStatus m_updateStatus = UpdateStatus::AT_START;

  UTILS::Parallel* m_parallel;
  ZoomPointFunc m_getZoomPoint;
  Point2dInt m_midpoint                 = {0, 0};
  NormalizedCoords m_normalizedMidpoint = {0.0F, 0.0F};
  Viewport m_filterViewport             = Viewport{};

  // 'NUM_STRIPE_GROUPS' controls how many updates before all stripes, and therefore,
  // all of the tran buffer, is filled. We use stripes to spread the buffer update load
  // over a number of updates. Too few and performance suffers periodically for a
  // number of updates; too many, and performance suffers overall.
  static constexpr auto NUM_STRIPE_GROUPS = 20U;
  uint32_t m_transformBufferYLineStart    = 0;
  uint32_t m_transformBufferStripeHeight  = m_dimensions.GetHeight() / NUM_STRIPE_GROUPS;
  std::vector<Point2dFlt> m_transformBuffer;
  std::vector<Point2dFlt> m_previousTransformBuffer;

  auto DoNextStripe(uint32_t transformBufferStripeHeight) noexcept -> void;
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

inline auto ZoomFilterBuffers::GetPreviousTransformBuffer() const noexcept
    -> const std::vector<Point2dFlt>&
{
  Expects(UpdateStatus::AT_END == m_updateStatus);

  return m_previousTransformBuffer;
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
