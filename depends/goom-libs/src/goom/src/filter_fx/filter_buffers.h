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
  enum class UpdateStatus : UnderlyingEnumType
  {
    AT_START,
    IN_PROGRESS,
    READY_TO_COPY,
    HAS_BEEN_COPIED,
  };

  ZoomFilterBuffers(UTILS::Parallel& parallel,
                    const PluginInfo& goomInfo,
                    const NormalizedCoordsConverter& normalizedCoordsConverter,
                    const ZoomFilterBufferStriper::ZoomPointFunc& getZoomPointFunc) noexcept;

  auto Start() noexcept -> void;
  auto Finish() noexcept -> void;

  auto SetTransformBufferMidpoint(const Point2dInt& val) noexcept -> void;
  auto SetFilterViewport(const Viewport& val) noexcept -> void;

  [[nodiscard]] auto GetUpdateStatus() const noexcept -> UpdateStatus;
  auto ResetTransformBufferToStart() noexcept -> void;
  auto StartTransformBufferStriping() noexcept -> void;
  auto UpdateTransformBuffer() noexcept -> void;

  [[nodiscard]] auto GetPreviousTransformBuffer() const noexcept -> const std::vector<Point2dFlt>&;
  // NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
  auto CopyTransformBuffer(std_spn::span<Point2dFlt> destBuff) noexcept -> void;

protected:
  // For testing only.
  [[nodiscard]] auto GetTransformBufferBuffMidpoint() const noexcept -> Point2dInt;

private:
  ZoomFilterBufferStriper m_filterStriper;
  UpdateStatus m_updateStatus = UpdateStatus::AT_START;
};

inline ZoomFilterBuffers::ZoomFilterBuffers(
    UTILS::Parallel& parallel,
    const PluginInfo& goomInfo,
    const NormalizedCoordsConverter& normalizedCoordsConverter,
    const ZoomFilterBufferStriper::ZoomPointFunc& getZoomPointFunc) noexcept
  : m_filterStriper{parallel, goomInfo, normalizedCoordsConverter, getZoomPointFunc}
{
}

inline auto ZoomFilterBuffers::GetUpdateStatus() const noexcept -> UpdateStatus
{
  return m_updateStatus;
}

inline auto ZoomFilterBuffers::GetTransformBufferBuffMidpoint() const noexcept -> Point2dInt
{
  return m_filterStriper.GetTransformBufferMidpoint();
}

inline auto ZoomFilterBuffers::SetTransformBufferMidpoint(const Point2dInt& val) noexcept -> void
{
  Expects(UpdateStatus::AT_START == m_updateStatus);

  m_filterStriper.SetTransformBufferMidpoint(val);
}

inline auto ZoomFilterBuffers::SetFilterViewport(const Viewport& val) noexcept -> void
{
  Expects(UpdateStatus::AT_START == m_updateStatus);

  m_filterStriper.SetFilterViewport(val);
}

inline auto ZoomFilterBuffers::Start() noexcept -> void
{
  m_filterStriper.Start();
  m_updateStatus = UpdateStatus::IN_PROGRESS;
}

inline auto ZoomFilterBuffers::Finish() noexcept -> void
{
  m_updateStatus = UpdateStatus::AT_START;
  m_filterStriper.Finish();
}

inline auto ZoomFilterBuffers::ResetTransformBufferToStart() noexcept -> void
{
  Expects(UpdateStatus::HAS_BEEN_COPIED == m_updateStatus);

  m_filterStriper.ResetTransformBufferToStart();
  m_updateStatus = UpdateStatus::AT_START;
}

inline auto ZoomFilterBuffers::StartTransformBufferStriping() noexcept -> void
{
  Expects(UpdateStatus::AT_START == m_updateStatus);

  m_filterStriper.StartTransformBufferStriping();
  m_updateStatus = UpdateStatus::IN_PROGRESS;
}

inline auto ZoomFilterBuffers::UpdateTransformBuffer() noexcept -> void
{
  if (UpdateStatus::HAS_BEEN_COPIED == m_updateStatus)
  {
    return;
  }

  Expects(UpdateStatus::IN_PROGRESS == m_updateStatus);

  m_filterStriper.UpdateNextTransformBufferStripe();

  if (m_filterStriper.HasFinishedTransformBuffer())
  {
    m_updateStatus = UpdateStatus::READY_TO_COPY;
  }
  else
  {
    Ensures(UpdateStatus::IN_PROGRESS == m_updateStatus);
  }
}

inline auto ZoomFilterBuffers::GetPreviousTransformBuffer() const noexcept
    -> const std::vector<Point2dFlt>&
{
  Expects(UpdateStatus::READY_TO_COPY == m_updateStatus);

  return m_filterStriper.GetPreviousTransformBuffer();
}

// NOLINTNEXTLINE(misc-include-cleaner): Waiting for C++20.
inline auto ZoomFilterBuffers::CopyTransformBuffer(std_spn::span<Point2dFlt> destBuff) noexcept
    -> void
{
  Expects(UpdateStatus::READY_TO_COPY == m_updateStatus);

  m_filterStriper.CopyTransformBuffer(destBuff);
  m_updateStatus = UpdateStatus::HAS_BEEN_COPIED;
}

} // namespace GOOM::FILTER_FX
