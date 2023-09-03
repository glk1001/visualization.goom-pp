#pragma once

#include "goom/goom_graphic.h"
#include "goom/goom_types.h"
#include "goom/point2d.h"
#include "normalized_coords.h"

#include <cstdint>
#include <functional>
#include <span> // NOLINT: Waiting to use C++20.
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

class ZoomFilterBufferStriper
{
public:
  using ZoomPointFunc =
      std::function<NormalizedCoords(const NormalizedCoords& normalizedCoords,
                                     const NormalizedCoords& normalizedFilterViewportCoords)>;

  enum class TransformBufferUpdateStatus
  {
    IN_PROGRESS,
    READY_TO_COPY,
    HAS_BEEN_COPIED,
  };

  ZoomFilterBufferStriper(UTILS::Parallel& parallel,
                          const PluginInfo& goomInfo,
                          const NormalizedCoordsConverter& normalizedCoordsConverter,
                          const ZoomPointFunc& getZoomPointFunc) noexcept;

  auto SetTransformBufferMidpoint(const Point2dInt& midpoint) noexcept -> void;
  auto SetFilterViewport(const Viewport& viewport) noexcept -> void;

  auto Start() noexcept -> void;

  [[nodiscard]] auto GetTransformBufferUpdateStatus() const noexcept -> TransformBufferUpdateStatus;
  [[nodiscard]] auto GetPreviousTransformBuffer() const noexcept -> const std::vector<Point2dFlt>&;
  // NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
  auto CopyTransformBuffer(std_spn::span<Point2dFlt> destBuff) noexcept -> void;
  auto ResetTransformBufferToStart() noexcept -> void;

  auto UpdateNextTransformBufferStripe() noexcept -> void;

  // For testing only.
  [[nodiscard]] auto GetTransformBufferMidpoint() const noexcept -> Point2dInt;

private:
  Dimensions m_dimensions;
  const NormalizedCoordsConverter* m_normalizedCoordsConverter;

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
  TransformBufferUpdateStatus m_transformBufferUpdateStatus =
      TransformBufferUpdateStatus::IN_PROGRESS;

  auto DoNextStripe(uint32_t transformBufferStripeHeight) noexcept -> void;
};

inline auto ZoomFilterBufferStriper::GetTransformBufferMidpoint() const noexcept -> Point2dInt
{
  return m_midpoint;
}

inline auto ZoomFilterBufferStriper::SetTransformBufferMidpoint(const Point2dInt& midpoint) noexcept
    -> void
{
  Expects(m_transformBufferYLineStart == 0U);
  m_midpoint           = midpoint;
  m_normalizedMidpoint = m_normalizedCoordsConverter->OtherToNormalizedCoords(m_midpoint);
}

inline auto ZoomFilterBufferStriper::SetFilterViewport(const Viewport& viewport) noexcept -> void
{
  Expects(m_transformBufferYLineStart == 0U);
  m_filterViewport = viewport;
}

inline auto ZoomFilterBufferStriper::GetTransformBufferUpdateStatus() const noexcept
    -> TransformBufferUpdateStatus
{
  return m_transformBufferUpdateStatus;
}

inline auto ZoomFilterBufferStriper::UpdateNextTransformBufferStripe() noexcept -> void
{
  DoNextStripe(m_transformBufferStripeHeight);
}

inline auto ZoomFilterBufferStriper::GetPreviousTransformBuffer() const noexcept
    -> const std::vector<Point2dFlt>&
{
  Expects(TransformBufferUpdateStatus::READY_TO_COPY == m_transformBufferUpdateStatus);
  return m_previousTransformBuffer;
}

} // namespace GOOM::FILTER_FX
