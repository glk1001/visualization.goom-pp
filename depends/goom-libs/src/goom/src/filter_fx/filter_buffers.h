#pragma once

#include "filter_buffer_striper.h"
#include "goom/goom_config.h"
#include "goom/point2d.h"
#include "goom_plugin_info.h"
#include "normalized_coords.h"
#include "utils/parallel_utils.h"

#include <span> // NOLINT: Waiting to use C++20.
#include <vector>

namespace GOOM::FILTER_FX
{

class ZoomFilterBuffers
{
public:
  ZoomFilterBuffers(UTILS::Parallel& parallel,
                    const PluginInfo& goomInfo,
                    const NormalizedCoordsConverter& normalizedCoordsConverter,
                    const ZoomFilterBufferStriper::ZoomPointFunc& getZoomPointFunc) noexcept;

  auto Start() noexcept -> void;

  auto SetTransformBufferMidpoint(const Point2dInt& val) noexcept -> void;
  auto SetFilterViewport(const Viewport& val) noexcept -> void;

  [[nodiscard]] auto IsTransformBufferInProgress() const noexcept -> bool;
  [[nodiscard]] auto IsTransformBufferReadyToCopy() const noexcept -> bool;
  [[nodiscard]] auto HasTransformBufferBeenCopied() const noexcept -> bool;
  [[nodiscard]] auto GetPreviousTransformBuffer() const noexcept -> const std::vector<Point2dFlt>&;
  // NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
  auto CopyTransformBuffer(std_spn::span<Point2dFlt> destBuff) noexcept -> void;
  auto StartFreshTranBuffer() noexcept -> void;

  auto UpdateTransformBuffer() noexcept -> void;

protected:
  // For testing only.
  [[nodiscard]] auto GetTransformBufferBuffMidpoint() const noexcept -> Point2dInt;

private:
  ZoomFilterBufferStriper m_filterStriper;

  auto UpdateNextTransformBufferStripe() noexcept -> void;
};

inline ZoomFilterBuffers::ZoomFilterBuffers(
    UTILS::Parallel& parallel,
    const PluginInfo& goomInfo,
    const NormalizedCoordsConverter& normalizedCoordsConverter,
    const ZoomFilterBufferStriper::ZoomPointFunc& getZoomPointFunc) noexcept
  : m_filterStriper{parallel, goomInfo, normalizedCoordsConverter, getZoomPointFunc}
{
}

inline auto ZoomFilterBuffers::IsTransformBufferInProgress() const noexcept -> bool
{
  return m_filterStriper.GetTransformBufferUpdateStatus() ==
         ZoomFilterBufferStriper::TransformBufferUpdateStatus::IN_PROGRESS;
}

inline auto ZoomFilterBuffers::IsTransformBufferReadyToCopy() const noexcept -> bool
{
  return m_filterStriper.GetTransformBufferUpdateStatus() ==
         ZoomFilterBufferStriper::TransformBufferUpdateStatus::READY_TO_COPY;
}

inline auto ZoomFilterBuffers::HasTransformBufferBeenCopied() const noexcept -> bool
{
  return m_filterStriper.GetTransformBufferUpdateStatus() ==
         ZoomFilterBufferStriper::TransformBufferUpdateStatus::HAS_BEEN_COPIED;
}

// NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
inline auto ZoomFilterBuffers::CopyTransformBuffer(std_spn::span<Point2dFlt> destBuff) noexcept
    -> void
{
  m_filterStriper.CopyTransformBuffer(destBuff);
}

inline auto ZoomFilterBuffers::GetTransformBufferBuffMidpoint() const noexcept -> Point2dInt
{
  return m_filterStriper.GetTransformBufferMidpoint();
}

inline auto ZoomFilterBuffers::SetTransformBufferMidpoint(const Point2dInt& val) noexcept -> void
{
  m_filterStriper.SetTransformBufferMidpoint(val);
}

inline auto ZoomFilterBuffers::SetFilterViewport(const Viewport& val) noexcept -> void
{
  m_filterStriper.SetFilterViewport(val);
}

inline auto ZoomFilterBuffers::Start() noexcept -> void
{
  m_filterStriper.Start();
  Ensures(m_filterStriper.GetTransformBufferUpdateStatus() ==
          ZoomFilterBufferStriper::TransformBufferUpdateStatus::IN_PROGRESS);
}

inline auto ZoomFilterBuffers::UpdateTransformBuffer() noexcept -> void
{
  if (m_filterStriper.GetTransformBufferUpdateStatus() ==
      ZoomFilterBufferStriper::TransformBufferUpdateStatus::HAS_BEEN_COPIED)
  {
    return;
  }

  Expects(m_filterStriper.GetTransformBufferUpdateStatus() ==
          ZoomFilterBufferStriper::TransformBufferUpdateStatus::IN_PROGRESS);

  UpdateNextTransformBufferStripe();

  Ensures((m_filterStriper.GetTransformBufferUpdateStatus() ==
           ZoomFilterBufferStriper::TransformBufferUpdateStatus::IN_PROGRESS) or
          (m_filterStriper.GetTransformBufferUpdateStatus() ==
           ZoomFilterBufferStriper::TransformBufferUpdateStatus::READY_TO_COPY));
}

inline auto ZoomFilterBuffers::StartFreshTranBuffer() noexcept -> void
{
  Expects(m_filterStriper.GetTransformBufferUpdateStatus() ==
          ZoomFilterBufferStriper::TransformBufferUpdateStatus::HAS_BEEN_COPIED);

  m_filterStriper.ResetTransformBufferToStart();

  Ensures(m_filterStriper.GetTransformBufferUpdateStatus() ==
          ZoomFilterBufferStriper::TransformBufferUpdateStatus::IN_PROGRESS);
}

inline auto ZoomFilterBuffers::UpdateNextTransformBufferStripe() noexcept -> void
{
  m_filterStriper.UpdateNextTransformBufferStripe();
}

inline auto ZoomFilterBuffers::GetPreviousTransformBuffer() const noexcept
    -> const std::vector<Point2dFlt>&
{
  Expects(IsTransformBufferReadyToCopy());
  return m_filterStriper.GetPreviousTransformBuffer();
}

} // namespace GOOM::FILTER_FX
