#include "filter_buffers.h"

#include "goom/goom_config.h"
#include "goom/point2d.h"
#include "goom_plugin_info.h"
#include "normalized_coords.h"
#include "utils/parallel_utils.h"

#include <cstddef>
#include <cstdint>
#include <mutex>

namespace GOOM::FILTER_FX
{

ZoomFilterBuffers::ZoomFilterBuffers(const PluginInfo& goomInfo,
                                     const NormalizedCoordsConverter& normalizedCoordsConverter,
                                     const ZoomPointFunc& getZoomPointFunc) noexcept
  : m_dimensions{goomInfo.GetDimensions()},
    m_normalizedCoordsConverter{&normalizedCoordsConverter},
    m_getZoomPoint{getZoomPointFunc},
    m_transformBuffer(m_dimensions.GetSize())
{
}

auto ZoomFilterBuffers::Start() noexcept -> void
{
  Expects(m_transformBuffer.size() == m_dimensions.GetSize());
  Expects(UpdateStatus::AT_START == m_updateStatus);

  // Make sure the current buffer is ready to be updated.
  ResetTransformBufferToStart();
  StartTransformBufferUpdates();
  Ensures(UpdateStatus::IN_PROGRESS == m_updateStatus);

  m_shutdown = false;
}

auto ZoomFilterBuffers::Finish() noexcept -> void
{
  m_shutdown = true;
  m_bufferProducer_cv.notify_all();

  ResetTransformBufferToStart();
}

auto ZoomFilterBuffers::ResetTransformBufferToStart() noexcept -> void
{
  Expects(m_shutdown or (UpdateStatus::IN_PROGRESS != m_updateStatus));

  m_updateStatus = UpdateStatus::AT_START;
}

auto ZoomFilterBuffers::StartTransformBufferUpdates() noexcept -> void
{
  const auto lock = std::scoped_lock<std::mutex>{m_mutex};

  Expects(UpdateStatus::AT_START == m_updateStatus);

  m_updateStatus = UpdateStatus::IN_PROGRESS;

  m_bufferProducer_cv.notify_all();
}

auto ZoomFilterBuffers::TransformBufferThread() noexcept -> void
{
  while (not m_shutdown)
  {
    UpdateTransformBuffer();

    auto lock = std::unique_lock<std::mutex>{m_mutex};
    if (m_shutdown)
    {
      break;
    }
    if (UpdateStatus::IN_PROGRESS != m_updateStatus)
    {
      m_bufferProducer_cv.wait(
          lock, [this] { return m_shutdown or (UpdateStatus::IN_PROGRESS == m_updateStatus); });
    }
  }
}

auto ZoomFilterBuffers::UpdateTransformBuffer() noexcept -> void
{
  auto lock = std::unique_lock<std::mutex>{m_mutex};

  if (UpdateStatus::HAS_BEEN_COPIED == m_updateStatus)
  {
    return;
  }

  Expects(UpdateStatus::IN_PROGRESS == m_updateStatus);

  lock.unlock();

  DoNextTransformBuffer();

  lock.lock();

  m_updateStatus = UpdateStatus::AT_END;
}

/*
 * Makes a transform buffer
 *
 * The transform is (in order) :
 * Translation (-data->middleX, -data->middleY)
 * Homothetie (Center : 0,0   Coeff : 2/data->screenWidth)
 */
auto ZoomFilterBuffers::DoNextTransformBuffer() noexcept -> void
{
  const auto screenWidth                  = m_dimensions.GetWidth();
  const auto screenSpan                   = static_cast<float>(screenWidth - 1);
  const auto sourceCoordsStepSize         = NormalizedCoords::COORD_WIDTH / screenSpan;
  const auto sourceViewportCoordsStepSize = m_filterViewport.GetViewportWidth() / screenSpan;

  const auto doTransformBufferRow =
      [this, &screenWidth, &sourceCoordsStepSize, &sourceViewportCoordsStepSize](const size_t y)
  {
    // Y-position of the first stripe pixel to compute in screen coordinates.
    const auto yScreenCoord = static_cast<uint32_t>(y);
    auto tranBufferPos      = yScreenCoord * screenWidth;

    auto centredSourceCoords =
        m_normalizedCoordsConverter->OtherToNormalizedCoords(GetPoint2dInt(0U, yScreenCoord)) -
        m_normalizedMidpoint;
    auto centredSourceViewportCoords = m_filterViewport.GetViewportCoords(centredSourceCoords);

    for (auto x = 0U; x < screenWidth; ++x)
    {
      const auto zoomPoint = m_getZoomPoint(centredSourceCoords, centredSourceViewportCoords);
      const auto uncenteredZoomPoint = m_normalizedMidpoint + zoomPoint;

      m_transformBuffer[tranBufferPos] = uncenteredZoomPoint.GetFltCoords();

      centredSourceCoords.IncX(sourceCoordsStepSize);
      centredSourceViewportCoords.IncX(sourceViewportCoordsStepSize);
      ++tranBufferPos;
    }
  };

  m_parallel.ForLoop(m_dimensions.GetHeight(), doTransformBufferRow);
}

} // namespace GOOM::FILTER_FX
